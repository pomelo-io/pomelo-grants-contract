#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# build
eosio-cpp pomelo.cpp -I include
# blanc++ curve.sx.cpp -I include
cleos set contract pomelo . pomelo.wasm pomelo.abi
