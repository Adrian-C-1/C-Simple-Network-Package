#include "core.h"
#include "Session.h"
#include "../Shared/env.h"
// HTTP --------------------------------------

_Server::_Server(boost::asio::io_context& io_context, std::function<void(std::string, Packet::Type, const char*, int)> readCallback)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), std::stoi(HTTP_PORT))) {
    this->readCallback = readCallback;
    do_accept();
}
void _Server::close_ip(std::string ip) {
    ipToSession.erase(ip);
}
void _Server::do_accept() {
    if (NETWORK_DEBUG)
        std::cout << "Http Listening\n";
    m_acceptor.async_accept([&](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::string ip = socket.remote_endpoint().address().to_string() + ":" + std::to_string(socket.remote_endpoint().port());
            std::cout << "[TTP] New connect with ip: " << ip << '\n';

            std::shared_ptr<Session> b = std::make_shared<Session>(std::move(socket), this, ip);
            ipToSession[ip] = std::shared_ptr<Session>(b);
            b->run(readCallback);
        }
        else {
            if (NETWORK_DEBUG)
                std::cout << "server error: " << ec.message() << std::endl;
        }
        do_accept();
        });
}

// HTTPS --------------------------------------

_SslServer::_SslServer(boost::asio::io_context& io_context,
    unsigned short port,
    std::function<void(std::string, Packet::Type, const char*, int)> readCallback)
    : m_acceptor(io_context, tcp::endpoint(tcp::v4(), port)),
    m_ctx(ssl::context::tls_server)
{
    this->readCallback = readCallback;

    // configure SSL context
    m_ctx.set_options(
        ssl::context::default_workarounds |
        ssl::context::no_sslv2 |
        ssl::context::single_dh_use);

    try {
        m_ctx.use_certificate_chain_file("server.crt");
        m_ctx.use_private_key_file("server.key", ssl::context::pem);
    }
    catch (const std::exception& e) {
        std::cout << "SSL setup failed: " << e.what() << std::endl;
        throw std::runtime_error("you fucked something up with the certificates");
    }

    do_accept();
}

void _SslServer::close_ip(std::string ip) {
    ipToSession.erase(ip);
}

void _SslServer::do_accept() {
    if (NETWORK_DEBUG)
    std::cout << "[TLS] Listening\n";

    m_acceptor.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::string ip = socket.remote_endpoint().address().to_string() + ":" +
                std::to_string(socket.remote_endpoint().port());
            std::cout << "[TLS] New connect on ip: " << ip << '\n';

            // Create session (SslSession instead of Session)
            auto b = std::make_shared<SslSession>(std::move(socket), this, ip, m_ctx);
            ipToSession[ip] = b;
            b->run(readCallback);
        }
        else {
            std::cout << "[TLS server error]: " << ec.message() << std::endl;
        }
        do_accept();
        });
}