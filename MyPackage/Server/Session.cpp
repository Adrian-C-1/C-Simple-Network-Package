#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>
#include <map>
#include <set>
#include <chrono>

#include "Session.h"
#include "Server.h"
#include "../Shared/packetHandlers.h"

Session::Session(tcp::socket socket, _Server* serv, std::string ip) : ip(ip),
    m_socket(std::move(socket)), sv(serv) {
    m_socket.set_option(tcp::no_delay(1));
    last_frame = 0;
}


Session::~Session() {
    m_socket.close();
    std::cout << "Session destructor called for ip " << ip << '\n';
}
void Session::run(std::function<void(std::string, Packet::Type, const char*, int)>& readCallback) {
    asyncRead(readCallback);
}
void Session::asyncRead(std::function<void(std::string, Packet::Type, const char*, int)> readCallback) {
    auto self(shared_from_this());
    ::asyncRead(ip, m_socket, m_buffer, readCallback, [&]() {
        std::cout << "[HTTP] closed connection with " << ip << '\n';
        sv->close_ip(ip);
        }, self);
}

// ------------------------------------

SslSession::SslSession(tcp::socket socket, _SslServer* server, std::string ip, ssl::context& ctx)
    : stream(std::move(socket), ctx), server(server), ip(ip) {
}

void SslSession::run(std::function<void(std::string, Packet::Type, const char*, int)> readCallback) {
    auto self = shared_from_this();
    stream.async_handshake(ssl::stream_base::server,
        [this, self, readCallback](const boost::system::error_code& ec) {
            if (!ec) {
                //std::cout << "[TLS] Handshake success with " << ip << "\n";
                // now use asyncRead with stream instead of socket
                asyncRead(ip, stream, m_buffer, readCallback, [&]() {
                    std::cout << "[TLS] closed connection with " << ip << '\n';
                    server->close_ip(ip);
                });
            }
            else {
                //std::cerr << "[TLS] Handshake failed: " << ec.message() << "\n";
                server->close_ip(ip);
            }
        });
}

//template<Packet::Type Type, typename... T>
//void SslSession::sendMessage(Packet::Packet<Type, T...> p) {
//    auto data = p.serialize();
//    boost::asio::async_write(stream,
//        boost::asio::buffer(data),
//        [](boost::system::error_code ec, std::size_t length) {
//            if (ec) std::cerr << "[TLS] write failed: " << ec.message() << "\n";
//        });
//}