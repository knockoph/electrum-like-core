# Electrum Like Core

Electrum server with a tech stack like Bitcoin Core.

## Install Dependencies

Install dependencies on Ubuntu 24.04:

```bash
sudo apt update
sudo apt install build-essential cmake libboost-dev
```

## Build

Build the C++ project:

```bash
# in repository
cmake -B build
cmake --build build
```

## Install Test Dependencies

Install Python3 and [Poetry](https://python-poetry.org/docs/):

```bash
sudo apt update
sudo apt install python3-dev python3-venv curl
curl -sSL https://install.python-poetry.org | python3 -
```

Install Python packages:

```bash
# in repository
poetry install
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

Run `elcd`:

```bash
# in repository
./build/src/elcd
```

Run Python tests:

```bash
# in repository
poetry run pytest -s tests
```
