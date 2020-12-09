#include <TcpServer.h>
#include <iostream>
#include <thread>
#include <boost/asio.hpp>
#include <glog/logging.h>
#include <Dispatcher.h>
#include <ProtobufCodec.h>
#include <TcpConnection.h>
#include <query.pb.h>
#include <TimerQueue.h>
#include <IOLoop.h>
#include <thread>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio.hpp>
#include <File.h>
using namespace IOEvent;

using QueryPtr = std::shared_ptr<QueryDef::Query> ;
using AnswerPtr = std::shared_ptr<QueryDef::Answer>;

void InitGlog()
{
	// 设置输出路径
	FLAGS_log_dir = "./log";
	// 应用程序的名称
	google::InitGoogleLogging("TcpServer");
	//设置级别高于 google::INFO 的日志同时输出到屏幕
	google::SetStderrLogging(google::GLOG_INFO);
	//设置输出到屏幕的日志显示相应颜色
	FLAGS_colorlogtostderr = true;
	//设置 google::INFO 级别的日志存储路径和文件名前缀
	google::SetLogDestination(google::GLOG_INFO, "./log/INFO_");
	//设置 google::WARNING 级别的日志存储路径和文件名前缀
	google::SetLogDestination(google::GLOG_WARNING, "./log/WARNING_");
	//设置 google::ERROR 级别的日志存储路径和文件名前缀
	google::SetLogDestination(google::GLOG_ERROR, "./log/ERROR_");
	//缓冲日志输出，默认为30秒，此处改为立即输出
	FLAGS_logbufsecs = 0;
	//最大日志大小为 100MB
	FLAGS_max_log_size = 100;
	//当磁盘被写满时，停止日志输出
	FLAGS_stop_logging_if_full_disk = true;
	//设置文件名扩展，如平台？或其它需要区分的信息
	google::SetLogFilenameExtension("server_");
	//捕捉 core dumped
	google::InstallFailureSignalHandler();

	//默认捕捉 SIGSEGV 信号信息输出会输出到 stderr，可以通过下面的方法自定义输出>方式：
	//google::InstallFailureWriter(&SignalHandle); 
}

class QueryServer
{
public:
	QueryServer(IOLoop *ios,
		const boost::asio::ip::tcp::endpoint &endpoint)
		: server_(ios, endpoint),
		dispatcher_(std::bind(&QueryServer::onUnknownMessage, this, std::placeholders::_1, std::placeholders::_2)),
		codec_(std::bind(&Dispatcher::onMessage, &dispatcher_, std::placeholders::_1, std::placeholders::_2))
	{
		dispatcher_.registerMessageCallback<QueryDef::Query>(
			std::bind(&QueryServer::onQuery, this, std::placeholders::_1, std::placeholders::_2));
		dispatcher_.registerMessageCallback<QueryDef::Answer>(
			std::bind(&QueryServer::onAnswer, this, std::placeholders::_1, std::placeholders::_2));
		server_.setConnectionCallback(
			std::bind(&QueryServer::onConnection, this, std::placeholders::_1));
		server_.setMessageCallback(
			std::bind(&ProtobufCodec::onMessage, &codec_, std::placeholders::_1, std::placeholders::_2));
	}

	void start()
	{
		server_.setThreadNum(5);
		server_.start();
	}

private:
	void onConnection(const TcpConnectionPtr& conn)
	{
		LOG(INFO) << conn->localAddr().address().to_string() << " -> "
			<< conn->remoteAddr().address().to_string();
	}

	void onUnknownMessage(const TcpConnectionPtr& conn,const MessagePtr& message)
	{
		LOG(INFO) << "onUnknownMessage: " << message->GetTypeName();
		conn->shutdown();
	}

	void onQuery(const TcpConnectionPtr& conn,const QueryPtr& message)
	{
		LOG(INFO) << "onQuery: " << message->GetTypeName() << message->DebugString();
		QueryDef::Answer answer;
		answer.set_id(1);
		answer.set_questioner("Txhua");
		answer.set_answerer("boost.org");
		codec_.send(conn, &answer);
		conn->shutdown();
	}

	void onAnswer(const TcpConnectionPtr& conn,const AnswerPtr& message)
	{
		LOG(INFO) << "onAnswer: " << message->GetTypeName();
		conn->shutdown();
	}

	IOEvent::TcpServer server_;
	Dispatcher dispatcher_;
	ProtobufCodec codec_;
};

void timeOut()
{
	LOG(INFO) << "time out";
}

void timeOut1()
{
	LOG(INFO) << "time out1";
}

using namespace std;



int main(int argc, char* argv[])
{
	try
	{
		// InitGlog();
		// LOG(INFO) << "main thread tid: " << std::this_thread::get_id();
		// boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8888);
		// IOEvent::IOLoop loop;
		// IOEvent::TcpServer s(&loop, ep);
		// s.setThreadNum(5);
		// s.setMessageCallback([&](const TcpConnectionPtr &conn, Buffer *buf) {});
		// s.setConnectionCallback([&](const TcpConnectionPtr &conn) {});
		// s.start();
		// google::ShutdownGoogleLogging();
		// boost::asio::io_context io;
		// boost::asio::ip::tcp::socket s(io);
		// auto b = s.native_handle();
		// std::thread t1(std::bind([]{while(1);}));
		// std::cout << std::this_thread::get_id() << std::endl;
		int socks[2];
		socketpair(AF_UNIX, SOCK_STREAM, 0, socks);
		boost::asio::local::stream_protocol protocel;
		boost::asio::io_context io;
		boost::asio::local::stream_protocol::socket parentSocket(io);
		boost::asio::local::stream_protocol::socket childSocket(io);
		parentSocket.assign(protocel, socks[1]);
		childSocket.assign(protocel, socks[0]);

		 std::string request("Dad I am your child, hello!");
		std::string dadRequest("Hello son!");

		//Create child process
		pid_t pid = fork();
		if( pid < 0 ){
			std::cerr << "fork() erred\n";
		} else if (pid == 0 ) { //child process
			parentSocket.close(); // no need of parents socket handle, childSocket is bidirectional stream socket unlike pipe that has different handles for read and write
			boost::asio::write(childSocket, boost::asio::buffer(request)); //Send data to the parent

			std::vector<char> dadResponse(dadRequest.size(),0);
			boost::asio::read(childSocket, boost::asio::buffer(dadResponse)); //Wait for parents response

			std::cout << "Dads response: ";
			std::cout.write(&dadResponse[0], dadResponse.size());
			std::cout << std::endl;


		} 
		else { //parent
			childSocket.close(); //Close childSocket here use one bidirectional socket
			std::vector<char> reply(request.size());
			boost::asio::read(parentSocket, boost::asio::buffer(reply)); //Wait for child process to send message

			std::cout << "Child message: ";
			std::cout.write(&reply[0], request.size());
			std::cout << std::endl;

			sleep(5); //Add 5 seconds delay before sending response to parent
			boost::asio::write(parentSocket, boost::asio::buffer(dadRequest)); //Send child process response

           }
		
		File f(1, "/dev/null");

	}
	catch (std::exception &e)
	{
		LOG(ERROR) << e.what();
	}
	
	return 0;
}
