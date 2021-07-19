#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create matching round and start it
cleos push action app.pomelo setround '[1, "2021-06-01T00:00:00", "2021-09-01T00:00:00"]' -p app.pomelo
cleos push action app.pomelo init '[]' -p app.pomelo
cleos push action app.pomelo setconfig '["status", 1]' -p app.pomelo
cleos push action app.pomelo setconfig '["roundid", 1]' -p app.pomelo

# create grant, enable it and join round
cleos push action app.pomelo setproject '["prjman1.eosn", "grant", "mygrant", "prjgrant1", [["4,EOS", "eosio.token"], ["4,USDT", "tethertether"]]]' -p app.pomelo -p prjman1.eosn
cleos push action app.pomelo enable '["grant", "mygrant", "ok"]' -p app.pomelo -p prjman1.eosn
cleos push action app.pomelo joinround '["mygrant", 1]' -p app.pomelo -p prjman1.eosn
