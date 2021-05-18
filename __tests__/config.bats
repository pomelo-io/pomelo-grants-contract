#!/usr/bin/env bats

@test "set config" {
  run cleos push action pomelo setstatus '["ok"]' -p pomelo
  echo "Output: $output"
  [ $status -eq 0 ]
}

@test "config.status = ok" {
  result=$(cleos get table pomelo pomelo config | jq -r '.rows[0].status')
  echo $result
  [ $result = "ok" ]
}
