#!/usr/bin/env bats

@test "cleos get info" {
  result=$(cleos get info | jq -r .server_version_string)
  echo $result
  [ $result = "v2.0.10" ] || [ $result = "v2.0.11" ]
}
