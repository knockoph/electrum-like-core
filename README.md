# Electrum Like Core

Electrum server with a tech stack like Bitcoin Core.

## Dependencies

Install on Ubuntu 24.04:

```bash
sudo apt update
sudo apt install build-essential cmake
```

## Build and Run

```bash
cmake -B build
cmake --build build
```

## Test

Run Bitcoin Core 28.0 in regtest mode:

```bash
bitcoind -regtest
```

Create a wallet and generate some blocks:

```bash
bitcoin-cli -regtest createwallet "testwallet"
bitcoin-cli -regtest -generate 101
```


