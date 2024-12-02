#include <iostream>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <univalue.h>


namespace asio = boost::asio;


void shutdown_socket(asio::ip::tcp::socket& socket) {
    boost::system::error_code ec;

    // Check if the socket is open before attempting to shut it down
    if (socket.is_open()) {
        socket.shutdown(asio::ip::tcp::socket::shutdown_both, ec); // Gracefully shutdown the connection
        if (ec && ec != boost::asio::error::not_connected) {
            std::cerr << "Error during shutdown: " << ec.message() << std::endl;
        }
    } else {
        std::cerr << "Socket is already closed, no shutdown needed." << std::endl;
    }

    // Always close the socket to prevent resource leaks
    if (socket.is_open()) {
        socket.close();
    }
}


class JsonRpcServer {
public:
    JsonRpcServer(asio::io_context& io_context, short port, std::size_t max_connections)
        : acceptor_(io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)),
          max_connections_(max_connections),
          current_connections_(0) {
        do_accept();
    }

private:
    asio::ip::tcp::acceptor acceptor_;
    const std::size_t max_connections_;
    std::atomic<std::size_t> current_connections_;


    void do_accept() {
        acceptor_.async_accept(
            [this](boost::system::error_code ec, asio::ip::tcp::socket socket) {
                if (!ec) {
                    // Check if the current connection count is below the limit
                    if (current_connections_ < max_connections_) {
                        // Increment the connection count
                        current_connections_++;

                        // Spawn a coroutine to handle the client
                        asio::co_spawn(socket.get_executor(), handle_client(std::move(socket)), asio::detached);
                    } else {
                        std::cerr << "Connection limit reached. Rejecting new connection." << std::endl;
                        shutdown_socket(socket);
                    }
                }
                do_accept(); // Continue accepting new connections
            }
        );
    }


    boost::asio::awaitable<void> handle_client(asio::ip::tcp::socket socket) {
        try {
            std::string request_buffer;
            const std::size_t max_buffer_size = 1024 * 1024; // 1 MB limit
            const std::chrono::seconds read_timeout_duration(30); // Set timeout duration
            asio::steady_timer timer(socket.get_executor());
            bool timed_out = false; // Flag to indicate if a timeout occurred

            while (true) {
                // Set the timer for the read operation
                timer.expires_after(read_timeout_duration);
                timer.async_wait([&timed_out, &socket](const boost::system::error_code& ec) {
                    if (!ec) {
                        timed_out = true; // Set the timeout flag
                        std::cerr << "Client timeout, shutting down connection." << std::endl;
                        shutdown_socket(socket);
                    }
                });

                std::array<char, 1024> buffer;
                std::size_t n = co_await socket.async_read_some(boost::asio::buffer(buffer), boost::asio::use_awaitable);

                // Cancel the timer on successful read
                timer.cancel();

                // Check if a timeout occurred
                if (timed_out) {
                    std::cerr << "Exiting handle_client due to timeout." << std::endl;
                    co_return; // Exit the coroutine
                }

                // Check if the read operation indicates a disconnection
                if (n == 0) {
                    std::cerr << "Client has closed the connection." << std::endl;
                    shutdown_socket(socket);
                    co_return; // Exit the coroutine
                }

                request_buffer.append(buffer.data(), n);

                // Check if the buffer size exceeds the limit
                if (request_buffer.size() > max_buffer_size) {
                    std::cerr << "Client sent too much data, shutting down connection." << std::endl;
                    shutdown_socket(socket);
                    co_return; // Exit the coroutine
                }

                // Process all complete JSON-RPC requests in the buffer
                while (true) {
                    auto pos = request_buffer.find('\n'); // Assuming requests are newline-terminated
                    if (pos == std::string::npos) {
                        break; // No complete request found
                    }

                    std::string request_str = request_buffer.substr(0, pos);
                    request_buffer.erase(0, pos + 1); // Remove processed request from buffer

                    // Process the JSON-RPC request
                    UniValue request;
                    if (!request.read(request_str)) {
                        // Handle JSON parsing error
                        UniValue error_response(UniValue::VOBJ);
                        error_response.pushKV("error", "Invalid JSON");
                        error_response.pushKV("id", UniValue(UniValue::VNULL));
                        std::string response_str = error_response.write() + "\n";
                        co_await boost::asio::async_write(socket, boost::asio::buffer(response_str), boost::asio::use_awaitable);
                        continue;
                    }

                    UniValue response = process_request(request);

                    // Send the response
                    std::string response_str = response.write() + "\n"; // Add newline for response
                    co_await boost::asio::async_write(socket, boost::asio::buffer(response_str), boost::asio::use_awaitable);
                }
            }
        } catch (const boost::system::system_error& e) {
            if (e.code() == boost::asio::error::eof) {
                std::cerr << "Client has closed the connection." << std::endl;
            } else {
                std::cerr << "Exception in handle_client: " << e.what() << std::endl;
            }
        }

        // Gracefully shutdown the connection before closing
        shutdown_socket(socket);

        // Decrement the connection count when the client disconnects
        current_connections_--;

        std::cout << "Client disconnected. Active connections: " << current_connections_ << std::endl;
    }



    UniValue process_request(const UniValue& request) {
        UniValue response(UniValue::VOBJ);
        try {
            // Basic JSON-RPC 2.0 processing
            if (request.exists("method") && request.exists("id")) {
                std::string method = request["method"].get_str();
                if (method == "add" && request.exists("params")) {
                    const UniValue& params = request["params"];
                    if (params.size() == 2) {
                        int a = params[0].get_real();
                        int b = params[1].get_real();
                        response.pushKV("result", a + b);
                    } else {
                        response.pushKV("error", "Invalid params");
                    }
                } else {
                    response.pushKV("error", "Method not found");
                }
                response.pushKV("id", request["id"]);
            } else {
                response.pushKV("error", "Invalid Request");
            }
        } catch (...) {
            response.pushKV("error", "Internal error");
        }
        return response;
    }
};


int main() {
    try {
        asio::io_context io_context;
        JsonRpcServer server(io_context, 8080, 32);
        io_context.run();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
