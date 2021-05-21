#!/usr/bin/env bats

@test "create users" {

  run cleos push action login.eosn create '["prjman1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].user_id')
  [ $result = "prjman1.eosn" ]

  run cleos push action login.eosn create '["prjman2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["prjman3.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["prjman4.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user3.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user4.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user5.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user11.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user12.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user13.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["prjman1.eosn", ["prjman1"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].accounts[0]')
  [ $result = "prjman1" ]

  run cleos push action login.eosn link '["prjman2.eosn", ["prjman2"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["prjman3.eosn", ["prjman3"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["prjman4.eosn", ["prjman4"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user1.eosn", ["user1"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user2.eosn", ["user2"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user3.eosn", ["user3"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user4.eosn", ["user4"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user5.eosn", ["user5"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user11.eosn", ["user11"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user12.eosn", ["user12"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user13.eosn", ["user13"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user1.eosn", ["github", "twitter", "facebook", "passport", "sms"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user2.eosn", ["github", "twitter"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user3.eosn", ["github", "twitter", "facebook"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user4.eosn", ["github"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user5.eosn", ["github", "twitter", "facebook", "passport", "sms"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user11.eosn", ["github"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user12.eosn", ["github", "twitter"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user13.eosn", ["github", "twitter"]]' -p login.eosn
  [ $status -eq 0 ]

}
