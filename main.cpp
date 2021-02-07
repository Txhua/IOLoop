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
#include <File.h>
#include <HttpServer.hpp>
#include <HttpSession.hpp>
#include <BasicRouter.hpp>
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
		InitGlog();
		LOG(INFO) << "main thread tid: " << std::this_thread::get_id();
		boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 8889);
		using http_server = _Default::http_server_type;
		boost::asio::io_context loop;
		const auto& onAccept = [&](const boost::asio::ip::tcp::socket &s ) {
			auto endpoint = s.remote_endpoint();
		};
		auto const address = boost::asio::ip::address_v4::any();
		auto const port = static_cast<unsigned short>(8080);
		using session = _Default::http_session;
		using http_context = typename session::context_type;
		using beast_http_request = typename session::request_type;
		using asio_socket = typename session::socket_type;
		//Http::BasicRouter<session> route{ std::regex::ECMAScript };


		Http::BasicRouter<session> animals{ std::regex::ECMAScript };

		animals.get("^/cat$", [](auto beast_http_request, auto context) { // '/animals/cat'
			//context.send(make_200<beast::http::string_body>(beast_http_request, "me-ow\n", "text/html"));
			std::cout << "hello " << std::endl;
		});

		animals.get("^/dog$", [](auto beast_http_request, auto context) { // '/animals/dog'
			//context.send(make_200<beast::http::string_body>(beast_http_request, "aw! aw! Rrrrr\n", "text/html"));
		});

		animals.get("^/mouse$", [](auto beast_http_request, auto context) { // '/animals/mouse'
			//context.send(make_200<beast::http::string_body>(beast_http_request, "...\n", "text/html"));
		});

		animals.get("^[/]??$", [](auto beast_http_request, auto context) { // '/animals' or '/animals/'
			//context.send(make_200<beast::http::string_body>(beast_http_request, "animals home page\n", "text/html"));
		});

		//http_server::start(loop, ep, onAccept);
		http_server s(loop, onAccept);

		auto& route = s.basicRouter();

		route.use("^/animals$", animals);
		
		route.get(R"(^/$)", [](beast_http_request r, http_context c) {
			std::cout << "hello " << std::endl;
		});
		
		s.start({ address, port });

		
		uint32_t pool_size = std::thread::hardware_concurrency() * 2;

		// Run the I/O service on the requested number of threads
		std::vector<std::thread> threads;
		threads.reserve(pool_size > 0 ? pool_size : 4);
		for (uint32_t i = 0; i < pool_size; i++)
			threads.emplace_back(std::bind(static_cast<std::size_t(boost::asio::io_context::*)()>
				(&boost::asio::io_context::run), std::ref(loop)));

		//// Block until all the threads exit
		for (auto& t : threads)
			t.join();
		//IOEvent::IOLoop loop_;
		//IOEvent::TcpServer s2(&loop_, ep);
		/*s.setThreadNum(4);
		s.setMessageCallback([&](const TcpConnectionPtr &conn, Buffer *buf) 
		{
			conn->send(buf);
		});
		s.setConnectionCallback([&](const TcpConnectionPtr &conn) {});
		s.start();
		loop.loop();*/
		google::ShutdownGoogleLogging();
	}
	catch (std::exception &e)
	{
		LOG(ERROR) << e.what();
	}
	
	return 0;
}
