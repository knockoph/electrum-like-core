#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>
#include <tuple>
#include <univalue.h>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/http.hpp>


std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

std::tuple<std::string, std::string> read_cookie() {
    std::string rel_path = ".bitcoin/regtest/.cookie";
    std::string home_path = getenv("HOME");
    std::string cookie_path = home_path + "/" + rel_path;

    std::ifstream file {cookie_path};
    if (!file.is_open()) {
        throw std::runtime_error("Could not open cookie file");
    }

    std::string line;
    std::getline(file, line);

    auto tokens = split_string(line, ':');
    return {tokens.at(0), tokens.at(1)};
}


std::string base64_encode(const std::string& input) {
    std::size_t output_length = boost::beast::detail::base64::encoded_size(input.size());
    std::string output;
    output.resize(output_length);
    boost::beast::detail::base64::encode(output.data(), input.data(), input.size());
    return output;
}


struct RpcSettings {
    const std::string& host;
    const std::string& port;
    const std::string& username;
    const std::string& password;
};


class RpcClient {
private:
    RpcSettings settings;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver;
    boost::beast::tcp_stream stream;

public:
    RpcClient(const RpcSettings& rpc_settings): settings(rpc_settings), resolver{io_context}, stream{io_context} {
        try {
            boost::asio::connect(stream.socket(), resolver.resolve(settings.host, settings.port));
        } catch (const boost::system::system_error& e) {
            throw std::runtime_error("Could not connect to rpc server");
        }
    }

    ~RpcClient() {
        boost::beast::error_code ec;
        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

        // if (ec && ec != boost::beast::errc::not_connected) {
        //     throw std::runtime_error("Could not shutdown socket");
        // }
    }

    UniValue request(const std::string& method, const UniValue params) {
        namespace beast = boost::beast;
        namespace http = boost::beast::http;

        UniValue data {UniValue::VOBJ};
        data.pushKV("jsonrpc", "2.0");
        data.pushKV("id", "curltest");
        data.pushKV("method", method);
        data.pushKV("params", params);

        auto payload = data.write();
        std::cout << payload << "\n";

        http::request<http::string_body> req{http::verb::post, "/", 11};
        req.set(http::field::host, settings.host);
        req.set(http::field::content_type, "application/json");
        auto encoded_auth = base64_encode(settings.username + ":" + settings.password);
        req.set(http::field::authorization, "Basic " + encoded_auth);
        req.body() = payload;
        req.prepare_payload();

        http::write(stream, req);
        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        if (res.result() != http::status::ok) {
            std::string error_code {std::to_string(static_cast<unsigned int>(res.result()))};
            throw std::runtime_error("Could not read rpc result due to HTTP error " + error_code);
        }

        auto body = res.body();
        UniValue result;
        result.read(body);

        return result;
    }
};

int main() {
    auto [username, password] = read_cookie();
    RpcSettings settings {"localhost", "18443", username, password};
    RpcClient client {settings};

    std::vector<int> v {};
    UniValue params {UniValue::VARR};
    for (auto e: v) {
        params.push_back(e);
    }

    UniValue r = client.request("getblockchaininfo", params);
    std::cout << "Result: " << r["result"].write() << "\n";

    r = client.request("getblockchaininfo", params);
    std::cout << "Result: " << r["result"].write() << "\n";

    return 0;
}
