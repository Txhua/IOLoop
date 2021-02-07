#include "TcpServer.h"
#include "Accept.h"
#include "IOLoop.h"
#include "TcpConnection.h"
#include "IOLoopThreadPool.h"
#include <glog/logging.h>
#include <functional>
#include <boost/asio/basic_stream_socket.hpp>

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

namespace IOEvent
{
TcpServer::TcpServer(IOLoop *loop, const ip::tcp::endpoint & endpoint, const std::string &name)
	:baseLoop_(loop),
	accept_(std::make_unique<Acceptor>(loop, endpoint)),
	threadPool_(std::make_shared<IOLoopThreadPool>(loop)),
	started_(false),
	ipPort_(endpoint.address().to_string()),
	name_(name),
	nextConnId_(1)
{
	accept_->setNewConnectCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1));
}

TcpServer::~TcpServer()
{
	baseLoop_->assertInLoopThread();
	LOG(WARNING) << "server exit :" << this;
	for (auto &item : connections_)
	{
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		conn->getLoop()->dispatch(std::bind(&TcpConnection::connectDestroyed, conn));
	}
}

void TcpServer::start()
{
	if (!started_)
	{
		LOG(INFO) << "server start ";
		started_ = true;
		threadPool_->run();
		accept_->start();
	}
}

void TcpServer::setThreadNum(int numThreads)
{
	threadPool_->setThreadNum(numThreads);
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn)
{
	baseLoop_->dispatch(std::bind(&TcpServer::removeConnectionInThisThread, this, conn));
}

void TcpServer::removeConnectionInThisThread(const TcpConnectionPtr & conn)
{
	baseLoop_->assertInLoopThread();
 	LOG(INFO) << "TcpServer::removeConnectionInLoop [" << name_
           << "] - connection " << conn->name();
	size_t n = connections_.erase(conn->name());
	(void)n;
	assert(n == 1);
	auto *ioLoop = conn->getLoop();
	ioLoop->post(std::bind(&TcpConnection::connectDestroyed, conn));
}

void TcpServer::newConnection(ip::tcp::socket && socket)
{
	baseLoop_->assertInLoopThread();
	char buf[64];
	auto *ioLoop = threadPool_->getNextLoop(socket.native_handle());
	snprintf(buf, sizeof(buf), "-%s#%d", ipPort_.c_str(), nextConnId_);
	std::string conName = name_ + buf;
	++nextConnId_;
	LOG(INFO) << "TcpServer::newConnection [" << name_ << "] - new connection [" << conName << "] from " << socket.remote_endpoint().address().to_string();
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(ioLoop, std::move(ip::tcp::socket(*ioLoop->getContext(), ip::tcp::v4(), socket.release())), conName);
	connections_[conName] = conn;
	conn->setConnectionCallback(connectionCallback_);
	conn->setMessageCallback(messageCallback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	ioLoop->dispatch(std::bind(&TcpConnection::connectEstablished, conn));
}

}
