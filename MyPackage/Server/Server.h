#pragma once

#include <string>
#include <iostream>
#include <functional>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "core.h"

class _Server;
class _SslServer;

namespace Server {
	void initHTTP(std::function<void(std::string, Packet::Type, const char*, int)> readCallback);
	void initHTTPS(std::function<void(std::string, Packet::Type, const char*, int)> readCallback);
	void close();

	_Server* getServerHttp();
	_SslServer* getServerHttps();
}