#pragma once

#include <iostream>
#include <string>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <functional>
#include "../Shared/Packet2.h"
#include "../Shared/packetHandlers.h"
#include <boost/asio/ssl.hpp>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
using std::cout;

namespace Client {
	void initHTTP(std::function<void(std::string, Packet::Type, const char*, int)> readCallback);
	bool hasInitedHTTP();

	void initHTTPS(std::function<void(std::string, Packet::Type, const char*, int)> readCallback);
	bool hasInitedHTTPS();

	tcp::socket& getHttpSocket();
	ssl::stream<tcp::socket>& getHttpsSocket();

	boost::asio::io_context& getHttpContext();

	template<Packet::Type Type, typename... T>
	void writeHttpPacket(Packet::Packet<Type, T...> p) {
		if (!hasInitedHTTP()) {
			throw std::runtime_error("You didnt call initHTTP()");
		}
		asyncWriteData(getHttpSocket(), p.data.data(), p.data.size());
	}
	template<Packet::Type Type, typename... T>
	void writeHttpsPacket(Packet::Packet<Type, T...> p) {
		if (!hasInitedHTTPS()) {
			throw std::runtime_error("You didnt call initHTTPS()");
		}
		asyncWriteData(getHttpsSocket(), p.data.data(), p.data.size());
	}

}