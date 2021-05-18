#!/usr/bin/env bats

@test "set user status" {
  run cleos push action pomelo userstatus '[1, "ok"]' -p pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "TODO" ]]
}
