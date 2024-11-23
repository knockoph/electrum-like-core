#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <univalue.h>
#include <boost/asio.hpp>


class RpcClient {
private:
    std::string v_host;
    std::string v_port;
    std::string v_auth;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::socket socket;

public:
    RpcClient(const std::string& host, const std::string& port, const std::string& auth):
    v_host(host), v_port(port), v_auth(auth), resolver(io_context), socket(io_context) {}

    bool connect() {
        try {
            boost::asio::connect(socket, resolver.resolve(v_host, v_port));
        } catch (const boost::system::system_error& e) {
            std::cerr << "Could not connect to RPC server.\n";
            return false;
        }
        return true;
    }

    std::string request(const std::string& method, const UniValue params) {
        UniValue data {UniValue::VOBJ};
        data.pushKV("jsonrpc", "2.0");
        data.pushKV("id", 1);
        data.pushKV("method", method);
        data.pushKV("params", params);

        std::string payload = data.write();
        std::cout << payload << "\n";
        std::string response;
        return response;
    }
};

int main() {
    std::string host {"localhost"};
    std::string port {"18443"};
    std::string auth {"some_cookie"};

    RpcClient client {host, port, auth};
    if (!client.connect()) return -1;

    std::vector<int> v {0, 1, 2};

    UniValue params {UniValue::VARR};
    for (auto e: v) {
        params.push_back(e);
    }

    client.request("hello", params);

    return 0;
}
