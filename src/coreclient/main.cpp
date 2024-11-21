#include<iostream>
#include<univalue.h>
#include <string>

using namespace std;

int main() {
    string r {"{\"hello\": \"world\"}"};

    UniValue data;
    data.read(r);
    cout << data["hello"].get_str() << "\n";
    return 0;
}
