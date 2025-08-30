#include "core.h"
#include "Server.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/asio/ssl.hpp>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
using std::cout;

namespace Server {
	_Server* server = 0;
	_SslServer* sslServer = 0;
	std::thread* thr;
	boost::asio::io_context context;

}

namespace Server {
	void initHTTP(std::function<void(std::string, Packet::Type, const char*, int)> readCallback) {
		if (server == nullptr) {
			thr = new std::thread([&]() {
					auto work = boost::asio::make_work_guard(context); // kys 1h wasted
					context.run();
					std::cout << "THE CONTEXT JUST CLOSED! YOU WONT BE ABLE TO SEND MORE MESSAGES !!!!!!!!!!!!!!!!!!!!!!!!!!\n";
				});
			server = new _Server(context, readCallback);

			//Sleep(500);
		}
	}
	void initHTTPS(std::function<void(std::string, Packet::Type, const char*, int)> readCallback) {
		if (sslServer == nullptr) {
			thr = new std::thread([&]() {
				auto work = boost::asio::make_work_guard(context); // kys 1h wasted
				context.run();
				std::cout << "THE CONTEXT JUST CLOSED! YOU WONT BE ABLE TO SEND MORE MESSAGES !!!!!!!!!!!!!!!!!!!!!!!!!!\n";
				});
			sslServer = new _SslServer(context, 57878, readCallback);

			//Sleep(500);
		}
	}
	void close() {
		thr->join();
	}
	_Server* getServerHttp() {
		if (server == nullptr) {
			throw std::runtime_error("You didnt call init()");
		}
		return server; 
	}
	_SslServer* getServerHttps() {
		if (sslServer == nullptr) {
			throw std::runtime_error("You didnt call init()");
		}
		return sslServer;
	}
}