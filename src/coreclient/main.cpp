#include <iostream>
#include <sstream>
#include <univalue.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/asio.hpp>


using namespace std;


string encode64(const string& s) {
    using namespace boost::archive::iterators;

    stringstream os;
    typedef base64_from_binary<transform_width<const char *, 6, 8>> base64_text;

    copy(
        base64_text(s.c_str()),
        base64_text(s.c_str() + s.size()),
        ostream_iterator<char>(os)
    );

    return os.str();
}


class RpcClient {
private:
    string v_host;
    int v_port;
    string v_auth;
    int sock;
    string auth_header;

public:
    RpcClient(const string& host, int port, const string& auth): v_host(host), v_port(port), v_auth(auth){
        string encoded_auth = encode64(v_auth);
        cout << encoded_auth << "\n";
    }

    int connect_client(){
        sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            cerr << "Could not open socket\n";
            return -1;
        }
        cout << sock << "\n";

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(v_port);

        inet_pton(AF_INET, v_host.c_str(), &addr.sin_addr);

        if (inet_pton(AF_INET, v_host.c_str(), &addr.sin_addr) <= 0) {
            cerr << "Invalid address\n";
            return -2;
        }

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            cerr << "Failed to connect to server\n";
            return -3;
        }

        return 0;
    }

    string request(const string& payload){
        string response;
        return response;
    }

    void close_client(){
        close(sock);
    }
};

int main() {
    string r {R"({"hello": "world"})"};
    UniValue data;
    data.read(r);
    cout << data["hello"].get_str() << "\n";

    string host {"127.0.0.1"};
    int port {18443};
    string auth {"some_cookie"};

    RpcClient client {host, port, auth};
    client.connect_client();
    client.close_client();


    boost::asio::io_context io_context;

    return 0;
}
