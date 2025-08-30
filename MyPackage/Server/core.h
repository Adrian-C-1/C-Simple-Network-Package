#pragma once

#include <iostream>
#include <map>

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <functional>
#include "../Shared/Packet2.h"
#include "Session.h"
#include <boost/asio/ssl.hpp>

class Session;

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
using std::cout;

// todo heartbeat every minute or so, if 1 connection is closed just close both

class _Server {
public:
    _Server(boost::asio::io_context& io_context, std::function<void(std::string, Packet::Type, const char*, int)> readCallback);

    void do_accept();
    void close_ip(std::string);
    
    template<Packet::Type Type, typename... T>
    void sendMessage(std::string ip, Packet::Packet<Type, T...> p) {
        ipToSession[ip]->sendMessage(p);
    }

    template<Packet::Type Type, typename... T>
    void send_message_all_except(std::string ip, Packet::Packet<Type, T...> p) {
        for (auto& i : ipToSession) {
            if (i.first != ip) {
                i.second->sendMessage(p);
                //std::cout << "Sending ack other to " << i.first << '\n';
                //ipToSession[ip]->sendMessage(p);
            }
        }
    }
private:
    tcp::acceptor m_acceptor;
    std::map<std::string, std::shared_ptr<Session>> ipToSession;

    std::function<void(std::string, Packet::Type, const char*, int)> readCallback;
};


class _SslServer {
public:
    _SslServer(boost::asio::io_context& io_context,
        unsigned short port,
        std::function<void(std::string, Packet::Type, const char*, int)> readCallback);

    void do_accept();
    void close_ip(std::string);

    template<Packet::Type Type, typename... T>
    void sendMessage(std::string ip, Packet::Packet<Type, T...> p) {
        ipToSession[ip]->sendMessage(p);
    }

    template<Packet::Type Type, typename... T>
    void send_message_all_except(std::string ip, Packet::Packet<Type, T...> p) {
        for (auto& i : ipToSession) {
            if (i.first != ip) {
                i.second->sendMessage(p);
            }
        }
    }

private:
    tcp::acceptor m_acceptor;
    ssl::context m_ctx;
    std::map<std::string, std::shared_ptr<SslSession>> ipToSession;
    std::function<void(std::string, Packet::Type, const char*, int)> readCallback;
};