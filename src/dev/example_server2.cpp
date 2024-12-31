#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>


namespace asio = boost::asio;

using namespace std::chrono_literals;


const int MAX_CONNECTIONS=2;


struct ConnectionCounter {
    int n;
};


asio::awaitable<void> handle_connection(asio::ip::tcp::socket socket, ConnectionCounter& cc, int id) {
    std::cout << "Handle " << id << "\n";
    try {
        char data[1024];
        while (true) {
            std::size_t n = co_await socket.async_read_some(asio::buffer(data), asio::use_awaitable);
            std::cout << "Data " << id << ": " << data << "\n";
            std::string s = std::to_string(id);
            co_await asio::async_write(socket, asio::buffer(s.data(), s.size()), asio::use_awaitable);
        }
    }
    catch (...) {}
    std::cout << "Close " << id << "\n";
    --cc.n;
}


asio::awaitable<void> accept_connections(asio::ip::tcp::acceptor& acceptor) {
    ConnectionCounter cc {0};
    int id = 0;
    while(true) {
        auto socket = co_await acceptor.async_accept(asio::use_awaitable);
        if (cc.n >= MAX_CONNECTIONS) {
            std::cout << "MAX CONNECTIONS REACHED\n";
            continue;
        }
        ++cc.n;
        asio::co_spawn(acceptor.get_executor(), handle_connection(std::move(socket), cc, id++), asio::detached);
    }
}


int main() {
    short port {4242};
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor (io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));

    asio::co_spawn(io_context, accept_connections(acceptor), asio::detached);

    io_context.run();

    return 0;
}
