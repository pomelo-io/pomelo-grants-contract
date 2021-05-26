#!/bin/bash

# unlock wallet
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

# create account
cleos create account eosio pomelo EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio login.eosn EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio eosio.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio tethertether EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio fake.token EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjbounty1 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant2 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman4 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant4 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjman5 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio prjgrant5 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user3 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user4 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user5 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user11 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user12 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio user13 EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV

# contract
cleos set contract eosio.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract tethertether ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract fake.token ./include/eosio.token eosio.token.wasm eosio.token.abi
cleos set contract login.eosn ./include/eosn.login login.eosn.wasm login.eosn.abi
cleos set contract pomelo . pomelo.wasm pomelo.abi

# @eosio.code permission
cleos set account permission pomelo active --add-code
cleos set account permission login.eosn active --add-code

# create tokens
cleos push action eosio.token create '["eosio", "100000000.0000 A"]' -p eosio.token
cleos push action eosio.token issue '["eosio", "10000000.0000 A", "init"]' -p eosio
cleos push action tethertether create '["eosio", "100000000.0000 B"]' -p tethertether
cleos push action tethertether issue '["eosio", "10000000.0000 B", "init"]' -p eosio

# create fake tokens
cleos push action fake.token create '["eosio", "100000000.0000 A"]' -p fake.token
cleos push action fake.token issue '["eosio", "5000000.0000 A", "init"]' -p eosio

# transfer tokens
cleos transfer eosio user1 "1000000.0000 A" ""
cleos transfer eosio user1 "1000000.0000 B" "" --contract tethertether
cleos transfer eosio user1 "1000000.0000 A" "" --contract fake.token
cleos transfer eosio user2 "1000000.0000 A" ""
cleos transfer eosio user2 "1000000.0000 B" "" --contract tethertether
cleos transfer eosio user3 "1000000.0000 A" ""
cleos transfer eosio user3 "1000000.0000 B" "" --contract tethertether
cleos transfer eosio user4 "1000000.0000 A" ""
cleos transfer eosio user4 "1000000.0000 B" "" --contract tethertether
cleos transfer eosio user5 "1000000.0000 A" ""
cleos transfer eosio user5 "1000000.0000 B" "" --contract tethertether
cleos transfer eosio user11 "1000000.0000 A" ""
cleos transfer eosio user11 "1000000.0000 B" "" --contract tethertether
cleos transfer eosio user12 "1000000.0000 A" ""
cleos transfer eosio user13 "1000000.0000 A" ""