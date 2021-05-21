#!/usr/bin/env bats

@test "create grant1" {

  run cleos push action pomelo setgrant '["grant1", "prjman1.eosn", ["prjman1.eosn"], "prjgrant1", [["4,B", "tethertether"]]]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].id')
  [ $result = "grant1" ]

  run cleos transfer user1 pomelo "100.0000 A" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for funding" ]]

  run cleos push action pomelo setprjstatus '["grant1", "ok"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos transfer user1 pomelo "200.0000 A" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "not accepted tokens for this project" ]]

  run cleos transfer user1 pomelo "300.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "no funding round ongoing" ]]

  run cleos push action pomelo setgrant '["grant2", "prjaaa.eosn", ["prjman1.eosn"], "prjgrant2", [["4,B", "tethertether"]]]' -p pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "author doesn't exist" ]]

}


@test "create bounty1 and fund it" {

  run cleos push action pomelo setbounty '["bounty1", "prjman1.eosn", ["prjman1.eosn"], "prjbounty1", [["4,A", "eosio.token"]]]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo bounties | jq -r '.rows[0].id')
  [ $result = "bounty1" ]

  run cleos transfer user1 pomelo "400.0000 A" "bounty:bounty1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for funding" ]]

  run cleos push action pomelo setprjstatus '["bounty1", "ok"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo bounties | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos transfer user1 pomelo "500.0000 A" "bounty:bounty1"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[0].user_id')
  [ $result = "user1.eosn" ]

}


@test "create and test rounds" {

  run cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-08-25T20:00:00"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].round')
  [ $result = "1" ]

  run cleos push action pomelo joinround '["grant1", 1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  run cleos push action pomelo joinround '["grant1", 1111]' -p pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "round doesn't exist" ]]

  run cleos transfer user1 pomelo "600.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "no funding round ongoing" ]]

  run cleos push action pomelo setround '[2, "2021-05-20T10:00:00", "2021-08-28T10:00:00"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1].round')
  [ $result = "2" ]

  run cleos push action pomelo joinround '["grant1", 2]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1].grant_ids[0]')
  [ $result = "grant1" ]

}

@test "round #1: fund grant1 with 2 donations by 2 users" {

  run cleos push action pomelo startround '[1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo state | jq -r '.rows[0].round_id')
  [ $result = "1" ]

  run cleos transfer user1 pomelo "10.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[1].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn10.0000 B" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[1] | .user_id + .amount.quantity')
  [ "$result" = "user1.eosn10.0000 B" ]
  grant_balance=$(cleos get currency balance tethertether prjgrant1 B)
  [ "$grant_balance" = "10.0000 B" ]

  run cleos transfer user2 pomelo "20.0000 B" "grant:grant1" --contract tethertether
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0] | .user_ids[1] + .accepted_tokens[0].quantity')
  [ "$result" = "user2.eosn30.0000 B" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[2] | .user_id + .amount.quantity')
  [ "$result" = "user2.eosn20.0000 B" ]
  grant_balance=$(cleos get currency balance tethertether prjgrant1 B)
  [ "$grant_balance" = "30.0000 B" ]

  result=$(cleos get table pomelo 1 match.grant | jq -r '.rows[0].square')
  [ $result = "104.46152422706630603" ]

  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].sum_square')
  [ $result = "104.46152422706630603" ]

}

@test "round #2: fund grant1 with 2 donations by 1 user" {

  run cleos push action pomelo startround '[2]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo state | jq -r '.rows[0].round_id')
  [ $result = "2" ]

  run cleos transfer user1 pomelo "50.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[3].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn50.0000 B" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[3] | .user_id + .amount.quantity')
  [ "$result" = "user1.eosn50.0000 B" ]
  grant_balance=$(cleos get currency balance tethertether prjgrant1 B)
  [ "$grant_balance" = "80.0000 B" ]

  run cleos transfer user1 pomelo "5.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn55.0000 B" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[4] | .user_id + .amount.quantity')
  [ "$result" = "user1.eosn5.0000 B" ]
  grant_balance=$(cleos get currency balance tethertether prjgrant1 B)
  [ "$grant_balance" = "85.0000 B" ]

  result=$(cleos get table pomelo 2 match.grant | jq -r '.rows[0].square')
  [ $result = "123.75000000000000000" ]

}


@test "round #2: create grant2 and fund with 8 microdonations" {

  run cleos push action pomelo setgrant '["grant2", "prjman2.eosn", ["prjman2.eosn"], "prjgrant2", [["4,A", "eosio.token"], ["4,B", "tethertether"]]]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo setprjstatus '["grant2", "ok"]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant2", 2]' -p pomelo
  [ $status -eq 0 ]

  run cleos transfer user1 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user3 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user4 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user5 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user11 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user12 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  run cleos transfer user13 pomelo "1.0000 A" "grant:grant2"
  [ $status -eq 0 ]

  result=$(cleos get table pomelo 2 match.grant | jq -r '.rows[1].square')
  [ $result = "1047.17937039107528108" ]

  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[1].sum_square')
  [ $result = "1170.92937039107528108" ]
}

@test "round #3: 4 projects, 6 users: spreadsheet simulation" {

  run cleos push action pomelo setround '[3, "2021-05-20T20:00:00", "2021-09-25T20:00:00"]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo setgrant '["grant3", "prjman3.eosn", ["prjman3.eosn"], "prjgrant3", [["4,B", "tethertether"]]]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo setgrant '["grant4", "prjman4.eosn", ["prjman4.eosn"], "prjgrant4", [["4,B", "tethertether"]]]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo setprjstatus '["grant3", "ok"]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo setprjstatus '["grant4", "ok"]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant1", 3]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant2", 3]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant3", 3]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo joinround '["grant4", 3]' -p pomelo
  [ $status -eq 0 ]

  run cleos push action pomelo startround '[3]' -p pomelo
  [ $status -eq 0 ]

  run cleos transfer user1 pomelo "80.0000 B" "grant:grant1" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user1 pomelo "100.0000 B" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "120.0000 B" "grant:grant1" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "20.0000 B" "grant:grant2" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user3 pomelo "300.0000 B" "grant:grant1" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user4 pomelo "10.0000 B" "grant:grant3" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user5 pomelo "200.0000 B" "grant:grant3" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user11 pomelo "1000.0000 B" "grant:grant3" --contract tethertether
  [ $status -eq 0 ]

  run cleos transfer user11 pomelo "1000.0000 B" "grant:grant4" --contract tethertether
  [ $status -eq 0 ]

  result=$(cleos get table pomelo 3 match.grant -L grant1 | jq -r '.rows[0].square')
  [ $result = "2474.63409191515165730" ]

  result=$(cleos get table pomelo 3 match.grant -L grant2 | jq -r '.rows[0].square')
  [ $result = "419.31676725154989072" ]

  result=$(cleos get table pomelo 3 match.grant -L grant3 | jq -r '.rows[0].square')
  [ $result = "3612.50000000000045475" ]

  result=$(cleos get table pomelo 3 match.grant -L grant4 | jq -r '.rows[0].square')
  [ $result = "1250.00000000000000000" ]

  result=$(cleos get table pomelo pomelo rounds -L 3 | jq -r '.rows[0].sum_square')
  [ $result = "7756.45085916670177539" ]
}

@test "disable/enable grant1" {

  run cleos push action pomelo setprjstatus '["grant1", "disabled"]' -p pomelo
  [ $status -eq 0 ]

  run cleos transfer user2 pomelo "3000.0000 B" "grant:grant1" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for funding" ]]

  run cleos push action pomelo setprjstatus '["grant1", "ok"]' -p pomelo
  [ $status -eq 0 ]
}