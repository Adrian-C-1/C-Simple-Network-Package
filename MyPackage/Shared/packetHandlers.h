#pragma once

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <boost/asio.hpp>
#include "Packet2.h"
#include "env.h"

using tcp = boost::asio::ip::tcp;

//void asyncWritePacket(boost::asio::ip::tcp::socket &m_socket, PKT_TYPE type, const void* data, int count);
//void asyncWritePacket2(boost::asio::ip::tcp::socket& m_socket, void* data, int count);

// TODO timeout on close 

template<typename AsyncReadWriteStream>
inline void asyncWriteData(AsyncReadWriteStream& m_socket, void* data, size_t count) {
	auto buffer = std::make_shared<std::vector<char>>(count);
	if (count != 0)
		memcpy((char*)buffer->data(), data, count);

	boost::asio::async_write(m_socket, boost::asio::buffer(*buffer), [buffer](boost::system::error_code ec, std::size_t len) {
		if (ec) {
			std::cout << "Error at write\n";
			; // do stuff
		}
		else {
			//std::cout << "Successfully wrote packet, " << len << " bytes\n";
		}
		});
}

class Session;

// tcp::socket | ssl::socket
template <typename AsyncReadWriteStream>
inline void asyncRead(std::string ip, AsyncReadWriteStream& m_socket, boost::asio::streambuf& m_buffer, const std::function<void(std::string, Packet::Type, const char*, int)>& readCallback, const std::function<void()>& errorCallback, std::shared_ptr<Session> self = nullptr) {
	boost::asio::async_read(m_socket, m_buffer,
		boost::asio::transfer_exactly(8),
		[&m_socket, &m_buffer, self, readCallback, errorCallback, ip](boost::system::error_code ec, std::size_t len)
		{
			if (ec) {
				if (NETWORK_DEBUG) {
					std::cout << "asyncRead error\n";
					std::cout << ec.message() << '\n';
				}
				errorCallback();
				return;
			}

			auto data = m_buffer.data();
			auto begin = boost::asio::buffers_begin(data);
			const char* ptr = reinterpret_cast<const char*>(&(*begin));

			// todo use the packet read function here to assure 100% it will read what's written 
			Packet::Type type;
			uint32_t type_n;
			int count;
			memcpy(&type_n, ptr, 4);
			type = static_cast<Packet::Type>(type_n);
			memcpy(&count, ptr + 4, 4);

			//std::cout << "Got a packet of type " << Packet::TypeString(type) << '\n';
			m_buffer.consume(8);

			if (count > 0) {
				boost::asio::read(m_socket, m_buffer, boost::asio::transfer_exactly(count), ec);
				if (ec) {
					if (NETWORK_DEBUG) {
						std::cout << "Read error: " << ec.message() << std::endl;
					}
					errorCallback();
					return;
				}

				auto data = m_buffer.data();
				auto begin = boost::asio::buffers_begin(data);
				void* ptr = static_cast<void*>(const_cast<char*>(&(*begin)));

#ifdef __SERVER
				// if self != nullptr, it's server-side
				// game->onReceivePacket(self->ip, type, ptr, count);
#elif defined(__CLIENT)
				// game->onReceivePacket(this->ip, type, ptr, count);
#endif

				readCallback(ip, type, (const char*)ptr, count);

				m_buffer.consume(count);
			}
			else if (count == 0) {
				readCallback("ip here", type, 0, 0);
				// game->onReceivePacket(self ? self->ip : "", type, 0, 0);
			}
			else {
				if (NETWORK_DEBUG) {
					std::cout << "Negative count in packet?????\n";
				}
				errorCallback();
				return;
			}

			asyncRead(ip, m_socket, m_buffer, readCallback, errorCallback, self);
		});
}