#!/usr/bin/env bats

@test "create grant1" {

  run cleos push action pomelo setproject '["prjman1.eosn", "grant", "grant1", "prjgrant1", [["4,EOS", "eosio.token"]]]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].id')
  [ $result = "grant1" ]

  run cleos transfer user1 pomelo "100.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for" ]]

  run cleos push action pomelo enable '["grant", "grant1", "ok"]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos transfer user1 pomelo "200.0000 USDT" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "not acceptable tokens for this project" ]]

  run cleos transfer user1 pomelo "300.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] is not active" ]]

  run cleos push action pomelo setproject '["prjaaa.eosn", "grant", "grant2", "prjgrant2", [["4,USDT", "tethertether"]]]' -p pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] does not exist" ]]

}


@test "create bounty1 and fund it" {

  run cleos push action pomelo setproject '["prjman1.eosn", "bounty", "bounty1", "prjbounty1", [["4,EOS", "eosio.token"]]]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo bounties | jq -r '.rows[0].id')
  [ $result = "bounty1" ]

  run cleos transfer user1 pomelo "400.0000 EOS" "bounty:bounty1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for donation" ]]

  run cleos push action pomelo enable '["bounty", "bounty1", "ok"]' -p pomelo prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo bounties | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos transfer user1 pomelo "100.0000 EOS" "bounty:bounty1" --contract fake.token
  [ $status -eq 1 ]
  [[ "$output" =~ "not accepted tokens for this project" ]]

  run cleos transfer user1 pomelo "500.0000 EOS" "bounty:bounty1"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[0].user_id')
  [ $result = "user1.eosn" ]
  bounty_balance=$(cleos get currency balance eosio.token prjbounty1 EOS)
  [ "$bounty_balance" = "500.0000 EOS" ]

}


@test "create and test rounds" {

  run cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-08-25T20:00:00"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].round')
  [ $result = "1" ]

  run cleos push action pomelo joinround '["grant1", 1]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  run cleos push action pomelo joinround '["grant1", 1111]' -p pomelo -p prjman1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] does not exist" ]]

  run cleos transfer user1 pomelo "600.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] is not active" ]]

  run cleos push action pomelo setround '[2, "2021-05-20T10:00:00", "2021-08-28T10:00:00"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1].round')
  [ $result = "2" ]

  run cleos push action pomelo joinround '["grant1", 2]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1].grant_ids[0]')
  [ $result = "grant1" ]

}

@test "round #1: fund grant1 with 2 donations by 2 users" {

  run cleos push action pomelo init '[1, 1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo globals -L round.id | jq -r '.rows[0].value')
  [ $result = "1" ]

  run cleos transfer user1 pomelo "10.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[1].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn10.0000 EOS" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[1] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user1.eosn10.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "10.0000 EOS" ]

  run cleos transfer user2 pomelo "20.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0] | .user_ids[1] + .accepted_tokens[0].quantity')
  [ "$result" = "user2.eosn30.0000 EOS" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[2] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user2.eosn20.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "30.0000 EOS" ]

  result=$(cleos get table pomelo 1 match | jq -r '.rows[0].square')
  [ $result = "104.46152422706630603" ]

  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].sum_square')
  [ $result = "104.46152422706630603" ]

}

@test "round #2: fund grant1 with 2 donations by 1 user" {

  run cleos push action pomelo init '[2, 1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo globals -L round.id | jq -r '.rows[0].value')
  [ $result = "2" ]

  run cleos transfer user1 pomelo "50.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[3].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn50.0000 EOS" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[3] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user1.eosn50.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "80.0000 EOS" ]

  run cleos transfer user1 pomelo "5.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn55.0000 EOS" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[4] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user1.eosn5.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "85.0000 EOS" ]

  result=$(cleos get table pomelo 2 match | jq -r '.rows[0].square')
  [ $result = "123.75000000000000000" ]

}


@test "round #2: create grant2 and fund with 8 microdonations" {

  run cleos push action pomelo setproject '["prjman2.eosn", "grant", "grant2", "prjgrant2", [["4,EOS", "eosio.token"], ["4,USDT", "tethertether"]]]' -p pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo enable '["grant", "grant2", "ok"]' -p pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant2", 2]' -p pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos transfer user1 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user3 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user4 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user5 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user11 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user12 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user13 pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  result=$(cleos get table pomelo 2 match | jq -r '.rows[1].square')
  [ $result = "10.47179370391075182" ]

  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1].sum_square')
  [ $result = "134.22179370391074826" ]

  result=$(cleos get table pomelo 2 users | jq -r '.rows[1].contributions[0] | .id + .value')
  [ $result = "grant20.12500000000000000" ]
}

@test "round #3: 4 projects, 6 users: spreadsheet simulation" {

  run cleos push action pomelo setround '[3, "2021-05-20T20:00:00", "2021-09-25T20:00:00"]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo setproject '["prjman3.eosn", "grant", "grant3", "prjgrant3", [["4,EOS", "eosio.token"]]]' -p pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo setproject '["prjman4.eosn", "grant", "grant4", "prjgrant4", [["4,EOS", "eosio.token"]]]' -p pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo enable '["grant", "grant3", "ok"]' -p pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo enable '["grant", "grant4", "ok"]' -p pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant1", 3]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant2", 3]' -p pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant3", 3]' -p pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant4", 3]' -p pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo init '[3, 1]' -p pomelo
  [ $status -eq 0 ]

  run cleos transfer user1 pomelo "80.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]

  run cleos transfer user1 pomelo "100.0000 EOS" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "120.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "20.0000 EOS" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user3 pomelo "300.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]

  run cleos transfer user4 pomelo "10.0000 EOS" "grant:grant3"
  [ $status -eq 0 ]

  run cleos transfer user5 pomelo "200.0000 EOS" "grant:grant3"
  [ $status -eq 0 ]

  run cleos transfer user11 pomelo "1000.0000 EOS" "grant:grant3"
  [ $status -eq 0 ]

  run cleos transfer user11 pomelo "1000.0000 EOS" "grant:grant4"
  [ $status -eq 0 ]

  result=$(cleos get table pomelo 3 match -L grant1 | jq -r '.rows[0].square')
  [ $result = "2474.63409191515165730" ]

  result=$(cleos get table pomelo 3 match -L grant2 | jq -r '.rows[0].square')
  [ $result = "419.31676725154989072" ]

  result=$(cleos get table pomelo 3 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "3612.50000000000045475" ]

  result=$(cleos get table pomelo 3 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "1250.00000000000000000" ]

  result=$(cleos get table pomelo pomelo rounds -L 3 | jq -r '.rows[0].sum_square')
  [ $result = "7756.45085916670177539" ]

  result=$(cleos get table pomelo 3 users -L user5.eosn | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant3450.00000000000000000" ]
}

@test "change socials triggering matching update for grants in current round" {

  result=$(cleos get table pomelo 3 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "500.00000000000000000" ]

  run cleos push action login.eosn social '["user11.eosn", ["github","sms"]]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  result=$(cleos get table pomelo 3 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "4029.52895126808152781" ]

  result=$(cleos get table pomelo 3 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "1500.00000000000000000" ]

  result=$(cleos get table pomelo pomelo rounds -L 3 | jq -r '.rows[0].sum_square')
  [ $result = "8423.47981043478284846" ]

  result=$(cleos get table pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant31500.00000000000000000" ]

  result=$(cleos get table pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[1] | .id + .value')
  [ $result = "grant41500.00000000000000000" ]

  result=$(cleos get table pomelo 3 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "1000.00000000000000000" ]

  run cleos push action login.eosn social '["user11.eosn", []]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  result=$(cleos get table pomelo pomelo rounds -L 3 | jq -r '.rows[0].sum_square')
  [ $result = "7071.69844341655516473" ]

  result=$(cleos get table pomelo 3 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "1000.00000000000000000" ]

  result=$(cleos get table pomelo 3 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "3177.74758424985338934" ]

  result=$(cleos get table pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant31000.00000000000000000" ]

  result=$(cleos get table pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[1] | .id + .value')
  [ $result = "grant41000.00000000000000000" ]

  result=$(cleos get table pomelo 3 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "0.00000000000000000" ]
}

@test "disable/enable grant5" {

  run cleos push action pomelo setproject '["prjman1.eosn", "grant", "grant5", "prjgrant1", [["4,EOS", "eosio.token"]]]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant5", 3]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action pomelo enable '["grant", "grant5", "disabled"]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "3000.0000 USDT" "grant:grant5" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for" ]]

  run cleos push action pomelo enable '["grant", "grant5", "ok"]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
}

@test "fund grant by a user without EOSN login" {
  run cleos transfer user.noeosn pomelo "30.0000 EOS" "grant:grant5"
  [ $status -eq 1 ]
  [[ "$output" =~ "account is not linked" ]]

  run cleos transfer user.noeosn pomelo "30.0000 EOS" "bounty:bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "account is not linked" ]]
}