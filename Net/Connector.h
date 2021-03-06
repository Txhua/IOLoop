//
// Use of this source code is governed by a BSD-style
// license that can be found in the License file.
//
// Author: Txhua
// 
// Copyright 2020, Txhua. All rights reserved.

#ifndef _IOEVENT_CONNECTOR_H
#define _IOEVENT_CONNECTOR_H

#include <boost/noncopyable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <atomic>
#include "TcpConnection.h"

namespace IOEvent
{
using namespace boost::asio;
class IOLoop;
class Connector final : 
	public boost::noncopyable,
	public std::enable_shared_from_this<Connector>
{
	using SocketErrorType = boost::asio::detail::socket_option::integer<SOL_SOCKET, SO_ERROR>;
	using NewConnectionCallback = std::function<void(ip::tcp::socket &&socket)>;
public:
	explicit Connector(IOLoop *loop, const ip::tcp::endpoint &endpoint);
	~Connector();
	void setNewConnectionCallback(NewConnectionCallback cb) { newConnectionCallback_ = std::move(cb); }
	void start();
	void stop();
	void restart();
	std::string serverAddress();
private:
	enum States { kDisconnected, kConnecting, kConnected };
	void setState(States s) { state_ = s; }
	void connect();
	void retry();
	void startInThisThread();
	void stopInThisThread();
	void connecting();
private:
	IOLoop *loop_;
	ip::tcp::socket socket_;
	ip::tcp::endpoint serverAddr_;
	std::atomic<bool> connect_;
	States state_;
	int retryDelayMs_;
	NewConnectionCallback newConnectionCallback_;
	static const int kMaxRetryDelayMs;
	static const int kInitRetryDelayMs;
};

}



#endif // !_IOEVENT_CONNECTOR_H
