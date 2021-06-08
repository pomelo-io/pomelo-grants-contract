#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create matching round and start it
cleos push action pomelo setround '[1, "2021-06-01T00:00:00", "2021-09-01T00:00:00"]' -p pomelo
cleos push action pomelo init '[1, 1]' -p pomelo

# create grant, enable it and join round
cleos push action pomelo setgrant '["grant1", "prjman.eosn", ["prjman.eosn"], "prjgrant", [["4,EOS", "eosio.token"]]]' -p pomelo
cleos push action pomelo enable '["grant", "grant1", "ok"]' -p pomelo
cleos push action pomelo joinround '["grant1", 1]' -p pomelo