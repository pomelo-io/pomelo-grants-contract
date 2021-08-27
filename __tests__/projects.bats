#!/usr/bin/env bats


@test "donate to missing grant" {
  run cleos transfer user1 app.pomelo "1000.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::donate_project: project not found" ]]
}


@test "create grant1" {

  run cleos push action app.pomelo setproject '["prjman1.eosn", "grant", "grant1", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo grants | jq -r '.rows[0].id')
  [ $result = "grant1" ]

  run cleos transfer user1 app.pomelo "100.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for donation" ]]

  run cleos push action app.pomelo enable '["grant", "grant1", "ok"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo grants | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos transfer user1 app.pomelo "200.0000 USDT" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::get_token: [symcode] not supported" ]]

  run cleos transfer user1 app.pomelo "300.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] is not active" ]]

  run cleos push action app.pomelo setproject '["prjaaa.eosn", "grant", "grant2", "prjgrant2", ["USDT"]]' -p app.pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] does not exist" ]]

}


@test "create bounty1 and fund it" {

  run cleos push action app.pomelo setproject '["prjman1.eosn", "bounty", "bounty1", "prjbounty1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[funding_account] must be empty for bounties" ]]

  run cleos push action app.pomelo setproject '["prjman1.eosn", "bounty", "bounty1", "", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo bounties | jq -r '.rows[0].id')
  [ $result = "bounty1" ]

  run cleos transfer user1 app.pomelo "400.0000 EOS" "bounty:bounty1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for donation" ]]

  run cleos push action app.pomelo enable '["bounty", "bounty1", "ok"]' -p app.pomelo prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos transfer user1 app.pomelo "100.0000 EOS" "bounty:bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "[funding_account] is not set" ]]

  run cleos push action app.pomelo setproject '["prjman1.eosn", "bounty", "bounty1", "prjbounty1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo bounties | jq -r '.rows[0].funding_account')
  [ $result = "prjbounty1" ]

  run cleos transfer user1 app.pomelo "100.0000 EOS" "bounty:bounty1" --contract fake.token
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::calculate_value: invalid pool" ]]

  run cleos transfer user1 app.pomelo "100.0000 USDT" "bounty:bounty1" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::get_token: [symcode] not supported" ]]

  run cleos transfer user1 app.pomelo "500.0000 EOS" "bounty:bounty1"
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[0].user_id')
  [ $result = "user1.eosn" ]
  bounty_balance=$(cleos get currency balance eosio.token prjbounty1 EOS)
  [ "$bounty_balance" = "475.0000 EOS" ]

}


@test "create and test rounds" {

  run cleos push action app.pomelo setround '[1, "2021-08-25T20:00:00", "2021-09-25T20:00:00", "This is round 1 of Pomelo!", [["10000 EOS", "eosio.token"]]]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].round')
  [ $result = "1" ]

  run cleos push action app.pomelo joinround '["grant1", 1]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  run cleos push action app.pomelo joinround '["grant1", 1111]' -p app.pomelo -p prjman1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] does not exist" ]]

  run cleos transfer user1 app.pomelo "600.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] is not active" ]]

  run cleos push action app.pomelo setround '[2, "2021-08-20T10:00:00", "2021-10-28T10:00:00", "This is round 2 of Pomelo!", [["50000 EOS", "eosio.token"]]]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].round')
  [ $result = "2" ]

  run cleos push action app.pomelo joinround '["grant1", 2]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].grant_ids[0]')
  [ $result = "grant1" ]

}

@test "round #1: fund grant1 with 2 donations by 2 users" {

  run cleos push action app.pomelo setconfig '[1, 0]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo globals | jq -r '.rows[0].round_id')
  [ $result = "1" ]

  run cleos transfer user1 app.pomelo "10.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[1].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0] | .user_ids[0] + .donated_tokens[0].quantity')
  [ "$result" = "user1.eosn10.0000 EOS" ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[1] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user1.eosn10.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "10.0000 EOS" ]

  run cleos transfer user2 app.pomelo "20.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0] | .user_ids[1] + .donated_tokens[0].quantity')
  [ "$result" = "user2.eosn30.0000 EOS" ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[2] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user2.eosn20.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "30.0000 EOS" ]

  result=$(cleos get table app.pomelo 1 match | jq -r '.rows[0].square')
  [ $result = "104.46152422706630603" ]

  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].sum_square')
  [ $result = "104.46152422706630603" ]

}

@test "round #2: fund grant1 with 2 donations by 1 user" {

  run cleos push action app.pomelo setconfig '[2, 0]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo globals | jq -r '.rows[0].round_id')
  [ $result = "2" ]

  run cleos transfer user1 app.pomelo "50.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[3].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1] | .user_ids[0] + .donated_tokens[0].quantity')
  [ "$result" = "user1.eosn50.0000 EOS" ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[3] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user1.eosn50.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "80.0000 EOS" ]

  run cleos transfer user1 app.pomelo "5.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1] | .user_ids[0] + .donated_tokens[0].quantity')
  [ "$result" = "user1.eosn55.0000 EOS" ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[4] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user1.eosn5.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "85.0000 EOS" ]

  result=$(cleos get table app.pomelo 2 match | jq -r '.rows[0].square')
  [ $result = "123.75000000000000000" ]

}


@test "round #2: create grant2 and fund with 8 microdonations" {

  run cleos push action app.pomelo setproject '["prjman2.eosn", "grant", "grant2", "prjgrant2", ["EOS", "USDT"]]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo enable '["grant", "grant2", "ok"]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant2", 2]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos transfer user1 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user2 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user3 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user4 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user5 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user11 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user12 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user13 app.pomelo "1.0000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 2 match | jq -r '.rows[1].square')
  [ $result = "10.47179370391075182" ]

  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].sum_square')
  [ $result = "134.22179370391074826" ]

  result=$(cleos get table app.pomelo 2 users | jq -r '.rows[1].contributions[0] | .id + .value')
  [ $result = "grant20.12500000000000000" ]
}

@test "round #3: 4 projects, 6 users: spreadsheet simulation" {

  run cleos push action app.pomelo setround '[3, "2021-05-20T20:00:00", "2021-09-25T20:00:00", "This is round 3 of Pomelo!", [["100000 EOS", "eosio.token"]]]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos push action app.pomelo setproject '["prjman3.eosn", "grant", "grant3", "prjgrant3", ["EOS"]]' -p app.pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setproject '["prjman4.eosn", "grant", "grant4", "prjgrant4", ["EOS"]]' -p app.pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo enable '["grant", "grant3", "ok"]' -p app.pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo enable '["grant", "grant4", "ok"]' -p app.pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant1", 3]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant2", 3]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant3", 3]' -p app.pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant4", 3]' -p app.pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setconfig '[3, 0]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos transfer user1 app.pomelo "80.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]

  run cleos transfer user1 app.pomelo "100.0000 EOS" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user2 app.pomelo "120.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]

  run cleos transfer user2 app.pomelo "20.0000 EOS" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user3 app.pomelo "300.0000 EOS" "grant:grant1"
  [ $status -eq 0 ]

  run cleos transfer user4 app.pomelo "10.0000 EOS" "grant:grant3"
  [ $status -eq 0 ]

  run cleos transfer user5 app.pomelo "200.0000 EOS" "grant:grant3"
  [ $status -eq 0 ]

  run cleos transfer user11 app.pomelo "1000.0000 EOS" "grant:grant3"
  [ $status -eq 0 ]

  run cleos transfer user11 app.pomelo "1000.0000 EOS" "grant:grant4"
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 3 match -L grant1 | jq -r '.rows[0].square')
  [ $result = "2474.63409191515165730" ]

  result=$(cleos get table app.pomelo 3 match -L grant2 | jq -r '.rows[0].square')
  [ $result = "419.31676725154989072" ]

  result=$(cleos get table app.pomelo 3 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "3612.50000000000045475" ]

  result=$(cleos get table app.pomelo 3 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "1250.00000000000000000" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 | jq -r '.rows[0].sum_square')
  [ $result = "7756.45085916670177539" ]

  result=$(cleos get table app.pomelo 3 users -L user5.eosn | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant3450.00000000000000000" ]
}

@test "change socials triggering matching update for grants in current round" {

  result=$(cleos get table app.pomelo 3 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "500.00000000000000000" ]

  result=$(cleos get table login.eosn login.eosn users -L user11.eosn | jq -r '.rows[0].socials | length')
  [ $result = "1" ]

  run cleos push action login.eosn social '["user11.eosn", "sms"]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  result=$(cleos get table login.eosn login.eosn users -L user11.eosn | jq -r '.rows[0].socials | length')
  [ $result = "2" ]

  result=$(cleos get table app.pomelo 3 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "4029.52895126808152781" ]

  result=$(cleos get table app.pomelo 3 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "1500.00000000000000000" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 | jq -r '.rows[0].sum_square')
  [ $result = "8423.47981043478284846" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant31500.00000000000000000" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[1] | .id + .value')
  [ $result = "grant41500.00000000000000000" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "1000.00000000000000000" ]

  run cleos push action login.eosn unsocial '["user11.eosn", "sms"]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn unsocial '["user11.eosn", "github"]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 | jq -r '.rows[0].sum_square')
  [ $result = "7071.69844341655516473" ]

  result=$(cleos get table app.pomelo 3 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "1000.00000000000000000" ]

  result=$(cleos get table app.pomelo 3 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "3177.74758424985338934" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant31000.00000000000000000" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn | jq -r '.rows[0].contributions[1] | .id + .value')
  [ $result = "grant41000.00000000000000000" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "0.00000000000000000" ]
}

@test "disable/enable grant5" {

  run cleos push action app.pomelo setproject '["prjman1.eosn", "grant", "grant5", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant5", 3]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo enable '["grant", "grant5", "disabled"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos transfer user2 app.pomelo "3000.0000 USDT" "grant:grant5" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::donate_project: project not available for donation" ]]

  run cleos push action app.pomelo enable '["grant", "grant5", "ok"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
}

@test "fund grant by a user without EOSN login" {
  run cleos transfer user.noeosn app.pomelo "30.0000 EOS" "grant:grant5"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::get_user_id: account is not linked" ]]

  run cleos transfer user.noeosn app.pomelo "30.0000 EOS" "bounty:bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::get_user_id: account is not linked" ]]
}


@test "unjoin grant1 from round 3" {
  result=$(cleos get table app.pomelo 3 match -l 1 | jq -r '.rows[0].grant_id')
  [ $result = "grant1" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0].grant_ids | length')
  [ $result = "5" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .sum_square')
  [ $result = "2830.00000000000000000772.500000000000000007071.69844341655516473" ]

  result=$(cleos get table app.pomelo 3 users -l 1 | jq -r '.rows[0].contributions[0].id')
  [ $result = "grant1" ]

  result=$(cleos get table app.pomelo 3 users -l 1 | jq -r '.rows[0].value')
  [ $result = "180.00000000000000000" ]

  result=$(cleos get table app.pomelo 3 users -L user3.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user3.eosn" ]

  run cleos push action app.pomelo unjoinround '["grant1", 3]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 3 match -l 1 | jq -r '.rows[0].grant_id')
  [ $result = "grant2" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0].grant_ids' | jq length)
  [ $result = "4" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .sum_square')
  [ $result = "2330.00000000000000000387.500000000000000004597.06435150140350743" ]

  result=$(cleos get table app.pomelo 3 users -l 1 | jq -r '.rows[0].contributions[0].id')
  [ $result = "grant2" ]

  result=$(cleos get table app.pomelo 3 users -l 1 | jq -r '.rows[0].value')
  [ $result = "100.00000000000000000" ]

  result=$(cleos get table app.pomelo 3 users -L user3.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user4.eosn" ]
}

@test "remove user11.eosn from round 3" {
  result=$(cleos get table app.pomelo 3 match | jq -r '.rows' | jq length)
  [ $result = "3" ]

  result=$(cleos get table app.pomelo 3 match -L grant3 -l 1 | jq -r '.rows[0].sum_value')
  [ $result = "1210.00000000000000000" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "6" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .sum_square')
  [ $result = "2330.00000000000000000387.500000000000000004597.06435150140350743" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user11.eosn" ]

  run cleos push action app.pomelo removeuser '["user11.eosn", 3]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 3 match | jq -r '.rows' | jq length)
  [ $result = "2" ]

  result=$(cleos get table app.pomelo 3 match -L grant3 -l 1 | jq -r '.rows[0] | .sum_value + .sum_sqrt')
  [ $result = "210.0000000000000000024.74873734152916782" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "5" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 3 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .sum_square')
  [ $result = "330.00000000000000000387.500000000000000001031.81676725155011809" ]

  result=$(cleos get table app.pomelo 3 users -L user11.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user2.eosn" ]
}

@test "collapse 8 users into 1 on round 2" {

  result=$(cleos get table app.pomelo 2 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant1123.75000000000000000" ]

  result=$(cleos get table app.pomelo 2 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[1] | .id + .value')
  [ $result = "grant20.22500000000000001" ]

  result=$(cleos get table app.pomelo 2 users | jq -r '.rows' |  jq length)
  [ $result = "8" ]

  result=$(cleos get table app.pomelo 2 match -L grant2 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .square')
  [ $result = "0.799999999999999930.5250000000000000210.47179370391075182" ]

  result=$(cleos get table app.pomelo 2 match -L grant1 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .square')
  [ $result = "55.0000000000000000068.75000000000000000123.75000000000000000" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 2 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .sum_square')
  [ $result = "55.8000000000000113769.27500000000000568134.22179370391074826" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 2 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "8" ]

  run cleos push action app.pomelo collapse '[["user2.eosn","user3.eosn","user4.eosn","user5.eosn","user11.eosn","user12.eosn","user13.eosn"], "user1.eosn", 2]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 2 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[0] | .id + .value')
  [ $result = "grant1123.75000000000000000" ]

  result=$(cleos get table app.pomelo 2 users | jq -r '.rows' |  jq length)
  [ $result = "1" ]

  result=$(cleos get table app.pomelo 2 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[1] | .id + .value')
  [ $result = "grant21.80000000000000049" ]

  result=$(cleos get table app.pomelo 2 match -L grant2 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .square')
  [ $result = "0.799999999999999930.999999999999999891.80000000000000071" ]

  result=$(cleos get table app.pomelo 2 match -L grant1 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .square')
  [ $result = "55.0000000000000000068.75000000000000000123.75000000000000000" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 2 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + .sum_square')
  [ $result = "55.8000000000000113769.75000000000000000125.54999999999999716" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 2 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "1" ]

}

@test "donate less than minamount" {
  run cleos transfer user1 app.pomelo "0.9000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "is less than [tokens.min_amount" ]]

  run cleos transfer user1 app.pomelo "0.0999 EOS" "grant:grant3"
  [ $status -eq 1 ]
  [[ "$output" =~ "is less than [tokens.min_amount" ]]

  run cleos push action app.pomelo token '["4,EOS", "eosio.token", 1001, null, null]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos transfer user1 app.pomelo "0.1000 EOS" "grant:grant3"
  [ $status -eq 1 ]
  [[ "$output" =~ "is less than [tokens.min_amount" ]]
}

@test "donate with a 5% fee" {
  run cleos push action app.pomelo setconfig '[3, 500]' -p app.pomelo
  [ $status -eq 0 ]

  balance=$(cleos get currency balance eosio.token fee.pomelo EOS)
  [ "$balance" = "25.0000 EOS" ]

  run cleos transfer user1 app.pomelo "10.0000 EOS" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user1 app.pomelo "10.0000 EOS" "bounty:bounty1"
  [ $status -eq 0 ]

  balance=$(cleos get currency balance eosio.token fee.pomelo EOS)
  [ "$balance" = "26.0000 EOS" ]

}

@test "clear transfers table" {
  result=$(cleos get table app.pomelo app.pomelo transfers -l 100 | jq -r '.rows | length')
  [ $result = "24" ]

  run cleos push action app.pomelo cleartable '["transfers", 0]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo app.pomelo transfers -l 1 | jq -r '.rows | length')
  [ $result = "0" ]
}