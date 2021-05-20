#!/usr/bin/env bats

@test "create grant" {

  run cleos push action pomelo setgrant '["grant1", "prjman1", ["prjman1"], "prjgrant1", [["4,B", "tethertether"]]]' -p pomelo
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

}


@test "create bounty and fund it" {

  run cleos push action pomelo setbounty '["bounty1", "prjman1", ["prjman1"], "prjbounty1", [["4,A", "eosio.token"]]]' -p pomelo
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


@test "create round, add grant to it, start round and fund it" {

  run cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].round')
  [ $result = "1" ]

  run cleos push action pomelo addgrant '["grant1", 1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  run cleos push action pomelo addgrant '["grant1", 1111]' -p pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "round doesn't exist" ]]

  run cleos transfer user1 pomelo "600.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "no funding round ongoing" ]]

  run cleos push action pomelo startround '[1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo state | jq -r '.rows[0].round_id')
  [ $result = "1" ]

  run cleos transfer user1 pomelo "1000.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[1].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn1000.0000 B" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[1] | .user_id + .amount.quantity')
  [ "$result" = "user1.eosn1000.0000 B" ]
  grant_balance=$(cleos get currency balance tethertether prjgrant1 B)
  [ "$grant_balance" = "1000.0000 B" ]

  run cleos transfer user2 pomelo "2000.0000 B" "grant:grant1" --contract tethertether
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0] | .user_ids[1] + .accepted_tokens[0].quantity')
  [ "$result" = "user2.eosn3000.0000 B" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[2] | .user_id + .amount.quantity')
  [ "$result" = "user2.eosn2000.0000 B" ]
  grant_balance=$(cleos get currency balance tethertether prjgrant1 B)
  [ "$grant_balance" = "3000.0000 B" ]

}