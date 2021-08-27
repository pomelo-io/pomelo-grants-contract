#!/usr/bin/env bats

@test "uninitialized contract" {
  run cleos transfer user1 app.pomelo "1000.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::on_transfer: contract is under maintenance" ]]
}


@test "init globals" {

  run cleos push action app.pomelo setconfig '[0, 500]' -p app.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo app.pomelo globals | jq -r '.rows[0].round_id')
  [ $result = "0" ]

}

@test "add login.eosn notifier" {

  run cleos push action login.eosn setnotifiers '[[app.pomelo]]' -p login.eosn
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table login.eosn login.eosn config | jq -r '.rows[0].notifiers[0]')
  [ $result = "app.pomelo" ]

}

@test "set token" {

  run cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 0]' -p app.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  run cleos push action app.pomelo token '["4,USDT", "tethertether", 10000, 12]' -p app.pomelo
  echo "Output: $output"
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo app.pomelo tokens | jq -r '.rows | length')
  [ $result = "2" ]

}
