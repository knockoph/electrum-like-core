#include <iostream>
#include <boost/beast/core/detail/base64.hpp>
#include <boost/beast/http.hpp>
#include "client.h"


namespace beast = boost::beast;
namespace http = beast::http;


std::string base64_encode(const std::string& input) {
    std::size_t output_length = beast::detail::base64::encoded_size(input.size());
    std::string output;
    output.resize(output_length);
    beast::detail::base64::encode(output.data(), input.data(), input.size());
    return output;
}


RpcClient::RpcClient(const RpcClientSettings& settings): settings_{settings}, resolver_{io_context_}, stream_{io_context_} {
    try {
        boost::asio::connect(stream_.socket(), resolver_.resolve(settings_.host, settings_.port));
    } catch (const boost::system::system_error& e) {
        throw std::runtime_error("Could not connect to rpc server");
    }
}


RpcClient::~RpcClient() {
    beast::error_code ec;
    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);

    // if (ec && ec != beast::errc::not_connected) {
    //     throw std::runtime_error("Could not shutdown socket");
    // }
}


UniValue RpcClient::request(const std::string& method, const UniValue params) {
    UniValue data {UniValue::VOBJ};
    data.pushKV("jsonrpc", "2.0");
    data.pushKV("id", "curltest");
    data.pushKV("method", method);
    data.pushKV("params", params);

    auto payload = data.write();
    std::cout << payload << "\n";

    http::request<http::string_body> req{http::verb::post, "/", 11};
    req.set(http::field::host, settings_.host);
    req.set(http::field::content_type, "application/json");
    auto encoded_auth = base64_encode(settings_.username + ":" + settings_.password);
    req.set(http::field::authorization, "Basic " + encoded_auth);
    req.body() = payload;
    req.prepare_payload();

    http::write(stream_, req);
    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream_, buffer, res);

    if (res.result() != http::status::ok) {
        std::string error_code {std::to_string(static_cast<unsigned int>(res.result()))};
        throw std::runtime_error("Could not read rpc result due to HTTP error " + error_code);
    }

    auto body = res.body();
    UniValue result;
    result.read(body);

    return result;
}
