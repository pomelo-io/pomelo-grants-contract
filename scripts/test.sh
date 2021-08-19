#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# enable tokens
cleos push action pomeloappapp token '["4,EOS", "eosio.token", 10000, "EOS Token", "https://eos.io", true]' -p pomeloappapp

# create matching round and start it
cleos push action pomeloappapp setround '[1, "2021-06-01T00:00:00", "2021-09-01T00:00:00", "Grant Round #1", [["1000.0000 EOS", "eosio.token"]]]' -p pomeloappapp
cleos push action pomeloappapp init '[]' -p pomeloappapp
cleos push action pomeloappapp setconfig '["status", 1]' -p pomeloappapp
cleos push action pomeloappapp setconfig '["roundid", 1]' -p pomeloappapp

# create grant, enable it and join round
cleos push action pomeloappapp setproject '["prjman1.eosn", "grant", "mygrant", "prjgrant1", ["EOS", "USDT"]]' -p pomeloappapp -p prjman1.eosn
cleos push action pomeloappapp enable '["grant", "mygrant", "ok"]' -p pomeloappapp -p prjman1.eosn
cleos push action pomeloappapp joinround '["mygrant", 1]' -p pomeloappapp -p prjman1.eosn
