#ifndef RPCCLIENT_H
#define RPCCLIENT_H

#include <string>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <univalue.h>


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
    RpcClient(const RpcSettings& rpc_settings);
    ~RpcClient();
    UniValue request(const std::string& method, const UniValue params);
};

#endif
