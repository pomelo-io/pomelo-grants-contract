#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# enable tokens
cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, "EOS Token", "https://eos.io", true]' -p app.pomelo

# create matching round and start it
cleos push action app.pomelo setconfig '[1, 500]' -p app.pomelo
cleos push action app.pomelo setround '[1, "2021-06-01T00:00:00", "2021-09-01T00:00:00", "Grant Round #1", [["1000.0000 EOS", "eosio.token"]]]' -p app.pomelo

# create grant, enable it and join round
cleos push action app.pomelo setproject '["myaccount", "grant", "mygrant", "myaccount", ["EOS", "USDT"]]' -p myaccount
cleos push action app.pomelo enable '["grant", "mygrant", "ok"]' -p app.pomelo
cleos push action app.pomelo joinround '["mygrant", 1]' -p myaccount
