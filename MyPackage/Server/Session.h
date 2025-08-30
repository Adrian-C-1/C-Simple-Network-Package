#pragma once

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "../Shared/Packet2.h"
#include "../Shared/packetHandlers.h"
#include <boost/asio/ssl.hpp>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
using std::cout;

extern class _Server;

class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, _Server* serv, std::string ip);
    ~Session();
    void run(std::function<void(std::string, Packet::Type, const char*, int)>& readCallback);

    template<Packet::Type Type, typename... T>
    void sendMessage(Packet::Packet<Type, T...> p) {
        asyncWriteData(m_socket, p.data.data(), p.data.size());
    }

    void asyncRead(std::function<void(std::string, Packet::Type, const char*, int)> readCallback);

private:
    _Server* sv;
    boost::asio::streambuf m_buffer;
    int last_frame;
    std::string ip;
public:
    tcp::socket m_socket;
};

// ---------------------------------

class _SslServer;

class SslSession : public std::enable_shared_from_this<SslSession> {
public:
    SslSession(tcp::socket socket, _SslServer* server, std::string ip, ssl::context& ctx);

    void run(std::function<void(std::string, Packet::Type, const char*, int)> readCallback);

    template<Packet::Type Type, typename... T>
    void sendMessage(Packet::Packet<Type, T...> p) {
        asyncWriteData(stream, p.data.data(), p.data.size());
    }

   /* template<Packet::Type Type, typename... T>
    void sendMessage(Packet::Packet<Type, T...> p);*/

private:
    boost::asio::streambuf m_buffer;
    ssl::stream<tcp::socket> stream;
    _SslServer* server;
    std::string ip;
};