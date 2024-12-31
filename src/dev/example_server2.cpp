#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>


namespace asio = boost::asio;


asio::awaitable<void> handle_connection(asio::ip::tcp::socket socket, int connections) {
    std::cout << "Handle " << connections << "\n";
    char data[1024];
    while (true) {
        std::size_t n = co_await socket.async_read_some(asio::buffer(data), asio::use_awaitable);
        std::cout << "Data " << connections << ": " << data << "\n";
        std::string s = std::to_string(connections);
        co_await asio::async_write(socket, asio::buffer(s.data(), s.size()), asio::use_awaitable);
    }
}


asio::awaitable<void> accept_connections(asio::ip::tcp::acceptor& acceptor) {
    int connections = 0;
    while(true) {
        auto socket = co_await acceptor.async_accept(asio::use_awaitable);
        asio::co_spawn(acceptor.get_executor(), handle_connection(std::move(socket), connections++), asio::detached);
    }
}


int main() {
    short port {4242};
    asio::io_context io_context;
    asio::ip::tcp::acceptor acceptor (io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port));
    //asio::ip::tcp::socket socket(io_context);
    int max_connections {2};

    //acceptor.listen(max_connections);

    asio::co_spawn(io_context, accept_connections(acceptor), asio::detached);

    io_context.run();

    return 0;
}
