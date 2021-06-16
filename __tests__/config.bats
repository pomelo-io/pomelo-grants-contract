#!/usr/bin/env bats

@test "uninitialized contract" {
  run cleos transfer user1 app.pomelo "1000.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[status] key does not exists" ]]
}

@test "contract under maintenance" {
  run cleos push action app.pomelo init '[]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos transfer user1 app.pomelo "1000.0000 EOS" ""
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "contract is under maintenance" ]]
}

@test "set config" {
  run cleos push action app.pomelo setconfig '[status, 1]' -p app.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo app.pomelo globals -L status | jq -r '.rows[0].value')
  [ $result = "1" ]

  run cleos push action app.pomelo setconfig '[systemfee, 0]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos push action app.pomelo setconfig '[minamount, 1000]' -p app.pomelo
  [ $status -eq 0 ]

}
