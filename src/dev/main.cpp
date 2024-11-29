#include <iostream>
#include <vector>
#include <tuple>
#include <univalue.h>
#include "../utils.h"
#include "../rpc/client.h"


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
