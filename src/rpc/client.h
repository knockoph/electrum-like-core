#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <univalue.h>


struct RpcClientSettings {
    const std::string& host;
    const std::string& port;
    const std::string& username;
    const std::string& password;
};


class RpcClient {
private:
    RpcClientSettings settings_;
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::tcp_stream stream_;

public:
    RpcClient(const RpcClientSettings& settings);
    ~RpcClient();
    UniValue request(const std::string& method, const UniValue params);
};

#endif
