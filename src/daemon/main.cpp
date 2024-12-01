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


struct RpcServerSettings {
    const std::string& host;
    short unsigned int port;
};


class RpcServer {
private:
    RpcServerSettings settings_;
    asio::io_context io_context_;
    asio::ip::address address_;
    asio::ip::port_type port_type_;
    asio::ip::tcp::endpoint endpoint_;
    asio::ip::tcp::acceptor acceptor_;

public:
    RpcServer(const RpcServerSettings& settings):
    settings_{settings},
    address_{asio::ip::make_address(settings_.host)},
    port_type_{settings.port},
    endpoint_{address_, port_type_},
    acceptor_{io_context_, endpoint_} {
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
