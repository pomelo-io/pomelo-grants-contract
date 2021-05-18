#!/usr/bin/env bats

@test "uninitialized contract" {
  run cleos transfer myaccount pomelo "1000.0000 A" ""
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "config does not exist" ]]
}

@test "contract under maintenance" {
  run cleos push action pomelo setstatus '["maintenance"]' -p pomelo
  [ $status -eq 0 ]

  run cleos transfer myaccount pomelo "1000.0000 A" ""
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "contract is under maintenance" ]]
}

@test "set config" {
  run cleos push action pomelo setstatus '["ok"]' -p pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  run cleos push action pomelo setvaluesym '[["4,B", "tethertether"]]' -p pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table pomelo pomelo config | jq -r '.rows[0].status')
  [ $result = "ok" ]

  result=$(cleos get table pomelo pomelo config | jq -r '.rows[0].value_symbol.contract')
  [ $result = "tethertether" ]
}
