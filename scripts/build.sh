#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# build
eosio-cpp pomelo.cpp -I include
# blanc++ pomelo.cpp -I include
cleos set contract pomelo . pomelo.wasm pomelo.abi

# additional builds
if [ ! -f "./include/eosio.token/eosio.token.wasm" ]; then
    eosio-cpp ./include/eosio.token/eosio.token.cpp -I include -o include/eosio.token/eosio.token.wasm
fi

if [ ! -f "./include/eosn.login/login.eosn.wasm" ]; then
    eosio-cpp ./include/eosn.login/login.eosn.cpp -I include -o include/eosn.login/login.eosn.wasm
fi
