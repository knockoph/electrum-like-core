#include <string>
#include <iostream>
#include <boost/asio.hpp>


namespace asio = boost::asio;


void accept_handler(const boost::system::error_code& error, boost::asio::ip::tcp::socket socket) {
    std::cout << "in accept handler\n";
    // while(true) {
    //     std::array<char, 1024> buffer_;
    //     socket.async_read_some(asio::buffer(buffer_));
    // }
}


asio::ip::tcp::endpoint make_endpoint(const std::string& host, short unsigned int port) {
    auto address = asio::ip::make_address(host);
    asio::ip::port_type port_type {port};
    return {address, port_type};
}


struct RpcServerSettings {
    const std::string& host;
    short unsigned int port;
};


class RpcServer {
private:
    RpcServerSettings settings_;
    asio::io_context io_context_;
    asio::ip::tcp::acceptor acceptor_;

public:
    RpcServer(const RpcServerSettings& settings): settings_{settings}, acceptor_{io_context_, make_endpoint(settings.host, settings.port)} {
        acceptor_.listen();
    };

    void run() {
        while (true) {
            auto socket = std::make_shared<asio::ip::tcp::socket>(io_context_);
            acceptor_.async_accept(*socket, [socket](const boost::system::error_code& error) {
                std::cout << "call accept handler\n";
                accept_handler(error, std::move(*socket));
            });
            io_context_.run();
            io_context_.restart();
        }
    }
};


int main() {
    RpcServerSettings settings {"127.0.0.1", 4242};
    RpcServer server {settings};
    server.run();
    return 0;
}
