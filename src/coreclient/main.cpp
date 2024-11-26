#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <univalue.h>
#include <boost/asio.hpp>
#include <filesystem>
#include <wordexp.h>


std::vector<std::string> split_string(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}


std::string read_cookie() {
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
    return tokens.at(1);
}


struct RpcSettings {
    const std::string& host;
    const std::string& port;
    const std::string& cookie;
};


class RpcClient {
private:
    RpcSettings settings;
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::socket socket;

public:
    RpcClient(const RpcSettings& rpc_settings): settings(rpc_settings), resolver(io_context), socket(io_context) {}

    void connect() {
        try {
            boost::asio::connect(socket, resolver.resolve(settings.host, settings.port));
        } catch (const boost::system::system_error& e) {
            throw std::runtime_error("Could not connect to rpc server");
        }
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
    auto cookie = read_cookie();
    std::cout << cookie << "\n";
    RpcSettings settings {"localhost", "18443", cookie};
    RpcClient client {settings};
    client.connect();

    std::vector<int> v {0, 1, 2};
    UniValue params {UniValue::VARR};
    for (auto e: v) {
        params.push_back(e);
    }

    client.request("hello", params);

    return 0;
}
