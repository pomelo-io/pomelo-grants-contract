#!/usr/bin/env bats

@test "create users" {

  run cleos push action login.eosn create '["prjman1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].user_id')
  [ $result = "prjman1.eosn" ]

  run cleos push action login.eosn create '["user1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[1].user_id')
  [ $result = "user1.eosn" ]

  run cleos push action login.eosn create '["user2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[2].user_id')
  [ $result = "user2.eosn" ]

  run cleos push action login.eosn link '["prjman1.eosn", ["prjman1"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].accounts[0]')
  [ $result = "prjman1" ]

  run cleos push action login.eosn link '["user1.eosn", ["user1"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[1].accounts[0]')
  [ $result = "user1" ]

  run cleos push action login.eosn link '["user2.eosn", ["user2"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[2].accounts[0]')
  [ $result = "user2" ]
}
