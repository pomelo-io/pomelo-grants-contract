#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create matching round and start it
cleos push action app.pomelo setround '[1, "2021-06-01T00:00:00", "2021-09-01T00:00:00"]' -p app.pomelo
cleos push action app.pomelo init '[1, 1]' -p app.pomelo

# create grant, enable it and join round
cleos push action app.pomelo setgrant '["grant1", "prjman.eosn", ["prjman.eosn"], "prjgrant", [["4,EOS", "eosio.token"]]]' -p app.pomelo
cleos push action app.pomelo enable '["grant", "grant1", "ok"]' -p app.pomelo
cleos push action app.pomelo joinround '["grant1", 1]' -p app.pomelo
