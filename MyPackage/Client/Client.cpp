#include "Client.h"
#include <boost/asio/ssl.hpp>
#include "../Shared/env.h"

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

namespace Client {
    std::thread* thr;
    
    // HTTP
    boost::asio::io_context m_http_context;
    tcp::socket* m_http_socket;
    tcp::resolver* m_http_resolver;
    boost::asio::streambuf m_http_buffer;
   
    // HTTPS
    ssl::context* m_https_context;
    ssl::stream<tcp::socket>* m_https_socket;
    tcp::resolver* m_https_resolver;
    boost::asio::streambuf m_https_buffer;
    // ---

    bool inited_http = 0;
    bool inited_https = 0;
}

namespace Client {
    void initHTTP(std::function<void(std::string, Packet::Type, const char*, int)> readCallback) {
        if (inited_http == 1) return;
        m_http_socket = new tcp::socket(Client::getHttpContext());
        m_http_resolver = new tcp::resolver(Client::getHttpContext());

        boost::asio::connect(*m_http_socket, m_http_resolver->resolve(PUBLIC_IP, HTTP_PORT)); // public ip adress and a port

        if (!m_http_socket->is_open()) {
            throw std::runtime_error("You didnt connect to the server");
        }
        m_http_socket->set_option(tcp::no_delay(1));
        asyncRead("server http", *m_http_socket, m_http_buffer, readCallback, [&]() {
            if(NETWORK_DEBUG)
                std::cout << "Error at read\n";
        });

        thr = new std::thread([&]() {
            // citesc inainte, teoretic are treaba la infinit
            auto work = boost::asio::make_work_guard(m_http_context); // todo stop on close
            m_http_context.run();
            std::cout << "THE IO CONTEXT JUST CLOSED! YOU WONT BE ABLE TO SEND MORE MESSAGES !!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            });

        inited_http = 1;
    }
    bool hasInitedHTTP() { return inited_http; }

    tcp::socket& getHttpSocket() {
        return *m_http_socket;
    }
    boost::asio::io_context& getHttpContext() {
        return m_http_context;
    }

    void initHTTPS(std::function<void(std::string, Packet::Type, const char*, int)> readCallback) {
        if (inited_https) return;
        m_https_context = new ssl::context(ssl::context::tls_client);
        m_https_context->set_verify_mode(ssl::verify_none); // TODO change this!

        m_https_resolver = new tcp::resolver(Client::getHttpContext());
        m_https_socket = new ssl::stream<tcp::socket>(Client::getHttpContext(), *m_https_context);

        boost::asio::connect(m_https_socket->next_layer(), m_https_resolver->resolve(PUBLIC_IP, TLS_PORT));

        if (!m_https_socket->next_layer().is_open()) {
            throw std::runtime_error("SSL: failed to connect");
        }

        m_https_socket->next_layer().set_option(tcp::no_delay(true));

        boost::system::error_code ec;
        m_https_socket->handshake(ssl::stream_base::client, ec);
        if (ec) {
            throw std::runtime_error("SSL handshake failed: " + ec.message());
        }

        asyncRead("server https", *m_https_socket, m_https_buffer, readCallback, [&]() {
            if (NETWORK_DEBUG)
                std::cout << "Error at reading from the socket, whoops\n";
           });

        thr = new std::thread([&]() {
            auto work = boost::asio::make_work_guard(m_http_context);
            m_http_context.run();
            std::cout << "HTTPS context closed!\n";
            });

        inited_https = true;
    }
    bool hasInitedHTTPS() { return inited_https; }
    ssl::stream<tcp::socket>& getHttpsSocket() {
        return *m_https_socket;
    }
}