#include "Worker.h"
#include <assert.h> 
#include <IOLoop.h>
#include <boost/asio/local/connect_pair.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <glog/logging.h>

namespace IOEvent
{
Worker::Worker()
    :loop_(std::make_unique<IOLoop>()),
    started_(false),
    id_(0),
    pid_(0)
{

}
Worker::~Worker()
{

}

void Worker::start()
{
    assert(!started_);
    started_ = true;
    //boost::asio::local::connect_pair(workerPipe_, masterPipe_);
    loop_->loop();
}

void Worker::read()
{
    // boost::asio::socket_base::receive_buffer_size size_op(100);
    // masterPipe_.set_option(size_op);
    // boost::asio::async_read(workerPipe_, boost::asio::buffer(inputBuffer_.beginWrite(), len), [this](const boost::system::error_code &error, size_t byte)
	// {
	// 	if (error)
	// 	{
	// 		LOG(WARNING) << "read() error! " << " error message: " << error.message();
	// 	}
	// 	else
	// 	{
	// 		inputBuffer_.hasWritten(byte);
	// 		read();
	// 	}
	// });
}

void Worker::write()
{

}

}