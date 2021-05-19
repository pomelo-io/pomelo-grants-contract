#!/usr/bin/env bats

@test "create grant" {

  run cleos push action pomelo setgrant '["grant1", "prjman1", ["prjman1"], "prjgrant1", [["4,B", "tethertether"]]]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].id')
  [ $result = "grant1" ]

  run cleos transfer user1 pomelo "1000.0000 A" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for funding" ]]

  run cleos push action pomelo setprjstatus '["grant1", "ok"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos transfer user1 pomelo "1000.0000 A" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "not accepted tokens for this project" ]]

  run cleos transfer user1 pomelo "1000.0000 B" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "no funding round ongoing" ]]

}


@test "create bounty" {

  run cleos push action pomelo setbounty '["bounty1", "prjman1", ["prjman1"], "prjbounty1", [["4,USDT", "tethertether"]]]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo bounties | jq -r '.rows[0].id')
  [ $result = "bounty1" ]

}


@test "create round and add grant to it" {

  run cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].round')
  [ $result = "1" ]

  run cleos push action pomelo addgrant '["grant1", 1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  run cleos push action pomelo addgrant '["grant1", 1]' -p pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "grant already exists in this round" ]]

  run cleos push action pomelo addgrant '["bounty1", 1]' -p pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "grant doesn't exist" ]]

  run cleos push action pomelo addgrant '["grant1", 1111]' -p pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "round doesn't exist" ]]

}