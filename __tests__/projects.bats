#!/usr/bin/env bats


@test "donate to missing grant" {
  run cleos transfer user1 app.pomelo "1000.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::donate_project: project not found" ]] || false
}

@test "donate to grant with invalid name" {
  run cleos transfer user1 app.pomelo "1000.0000 EOS" "grant:grant9"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::on_transfer: invalid project id" ]] || false

  run cleos transfer user1 app.pomelo "1000.0000 EOS" "grant:grant132323."
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::on_transfer: invalid project id" ]] || false

  run cleos transfer user1 app.pomelo "1000.0000 EOS" "grant:grant1234212vsddsas"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::on_transfer: invalid project id" ]] || false
}


@test "create grant1" {

  run cleos push action app.pomelo setgrant '["prjman1.eosn", "grant1", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo grants | jq -r '.rows[0].id')
  [ $result = "grant1" ]

  run cleos transfer user1 app.pomelo "100.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for donation" ]] || false

  run cleos push action app.pomelo setstate '["grant1", "published"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo grants | jq -r '.rows[0].status')
  [ $result = "published" ]

  run cleos transfer user1 app.pomelo "200.0000 USDT" "grant:grant1" --contract tethertether
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "not acceptable tokens for this project" ]] || false

  run cleos transfer user1 app.pomelo "300.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "is not active" ]] || false

  run cleos push action app.pomelo setgrant '["prjaaa.eosn", "grant2", "prjgrant2", ["USDT"]]' -p app.pomelo
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[user_id] does not exist" ]] || false

}

@test "create bounty with existing grant name" {

  run cleos push action app.pomelo setproject '["prjman1.eosn", "bounty", "grant1", "", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "Grant with [project_id] already exists" ]] || false

}

@test "attempt to change author id" {

  run cleos push action app.pomelo setgrant '["prjman2.eosn", "grant1", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "project [author_id] cannot be modifed" ]] || false

}

@test "create bounty1 and fund it" {

  run cleos push action app.pomelo setproject '["prjman1.eosn", "bounty", "bounty1", "prjbounty1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[funding_account] must be empty for bounties" ]] || false

  run cleos push action app.pomelo setproject '["prjman1.eosn", "bounty", "bounty1", "", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo bounties | jq -r '.rows[0].id')
  [ $result = "bounty1" ]

  run cleos transfer user1 app.pomelo "400.0000 EOS" "bounty:bounty1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "project not available for donation" ]] || false

  run cleos push action app.pomelo setstate '["bounty1", "published"]' -p app.pomelo prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo bounties | jq -r '.rows[0].status')
  [ $result = "published" ]

  run cleos transfer user1 app.pomelo "100.0000 EOS" "bounty:bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "[funding_account] is not set" ]] || false

  run cleos push action app.pomelo setproject '["prjman1.eosn", "bounty", "bounty1", "prjbounty1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo bounties | jq -r '.rows[0].funding_account')
  [ $result = "prjbounty1" ]

  run cleos transfer user1 app.pomelo "100.0000 EOS" "bounty:bounty1" --contract fake.token
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::get_token: [token.contract] is invalid" ]] || false

  run cleos transfer user1 app.pomelo "100.0000 USDT" "bounty:bounty1" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "not acceptable tokens for this project" ]] || false

  run cleos transfer user1 app.pomelo "500.0000 EOS" "bounty:bounty1"
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[0].user_id')
  [ $result = "user1.eosn" ]
  bounty_balance=$(cleos get currency balance eosio.token prjbounty1 EOS)
  [ "$bounty_balance" = "475.0000 EOS" ]

}

@test "set bad season" {
  run cleos push action app.pomelo setseason '[123, null, null, null, null, "Bad season", 100000]' -p app.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "[end_at] must be after [start_at]" ]] || false

  run cleos push action app.pomelo setseason '[123, "2021-08-25T20:00:00", "2021-08-26T10:00:00", "2021-08-20T20:00:00", "2021-08-25T20:00:00", "Bad round", 100000]' -p app.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "active minimum period must be at least" ]] || false

  run cleos push action app.pomelo setseason '[123, "2021-08-20T20:00:00", "2021-08-29T20:00:00", "2021-08-20T20:00:00", "2021-08-21T10:00:00", "Bad round", 100000]' -p app.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "submission minimum period must be at least" ]] || false

  run cleos push action app.pomelo setseason '[123, "2021-08-20T20:00:00", "2021-08-29T20:00:00", "2021-08-21T20:00:00", "2021-08-29T20:00:00", "Bad round", 100000]' -p app.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "[submission_start_at] must be before [start_at]" ]] || false

  run cleos push action app.pomelo setseason '[123, "2021-08-20T20:00:00", "2021-08-29T20:00:00", "2021-08-20T20:00:00", "2021-08-29T21:00:00", "Bad round", 100000]' -p app.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "[submission_end_at] must be before [end_at]" ]] || false

  run cleos push action app.pomelo setseason '[0, "2022-08-25T20:00:00", "2022-09-25T20:00:00", "2022-08-25T20:00:00", "2022-09-25T20:00:00", "This is season 1 of Pomelo!", 50000]' -p app.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "[season_id] must be positive" ]] || false
}

@test "create seasons" {
  run cleos push action app.pomelo setseason '[1, "2021-09-25T20:00:00", "2022-09-25T20:00:00", "2021-08-25T20:00:00", "2021-09-26T20:00:00", "Season 1", 100000]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo seasons | jq -r '.rows[0].season_id')
  [ $result = "1" ]

  run cleos push action app.pomelo setseason '[2, "2021-09-25T20:00:00", "2022-09-25T20:00:00", "2021-09-25T20:00:00", "2022-09-25T20:00:00", "Season 2", 100000]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo seasons | jq -r '.rows[1].season_id')
  [ $result = "2" ]
}

@test "join round outside of submission period" {
  run cleos push action app.pomelo setround '[101, 1, "This is round 1 of Pomelo!", 50000]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].round_id')
  [ $result = "101" ]

  run cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] submission period has ended" ]] || false

  run cleos push action app.pomelo setseason '[1, "2022-08-25T20:00:00", "2022-09-25T20:00:00", "2022-08-25T20:00:00", "2022-09-25T20:00:00", "Season 1", 100000]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] submission period has not started" ]] || false

}

@test "create and test rounds" {

  run cleos push action app.pomelo setseason '[1, "2021-09-25T20:00:00", "2022-09-25T20:00:00", "2021-09-25T20:00:00", "2022-09-25T20:00:00", "Season 1", 100000]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo seasons | jq -r '.rows[0].season_id')
  [ $result = "1" ]

  run cleos push action app.pomelo setround '[101, 1, "This is round 1 of Pomelo!", 100000]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].round_id')
  [ $result = "101" ]

  run cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  run cleos push action app.pomelo joinround '["grant1", 1111]' -p app.pomelo -p prjman1.eosn
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] does not exist" ]] || false

  run cleos transfer user1 app.pomelo "600.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] is not active" ]] || false

  run cleos push action app.pomelo setround '[102, 1, "This is round 2 of Pomelo!", 50000]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].round_id')
  [ $result = "102" ]

}

@test "add grant to another round in this season" {

  run cleos push action app.pomelo joinround '["grant1", 102]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "grant already exists in this season" ]] || false

}

@test "add round to 2 seasons" {
  run cleos push action app.pomelo setround '[101, 2, "This is round 2 of Pomelo!", 50000]' -p app.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "[round_id] already exists in another season" ]] || false
}

@test "update round #2" {

  run cleos push action app.pomelo setround '[102, 1, "This is round 2 of Pomelo (revised)!", 100000]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].round_id')
  [ $result = "102" ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].match_value')
  [ $result = "100000.00000000000000000" ]

}

@test "round #1: fund grant1 with 2 donations by 2 users" {

  run cleos push action app.pomelo setconfig '[1, 0, 0, null, null]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo globals | jq -r '.rows[0].season_id')
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

  result=$(cleos get table app.pomelo 101 match | jq -r '.rows[0].square')
  [ $result = "1044.61524227066342974" ]

  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].sum_square')
  [ $result = "1044.61524227066342974" ]

}

@test "unjoin and join round 101" {

  run cleos push action app.pomelo unjoinround '["grant1", 101]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].grant_ids | length')
  [ $result = "0" ]

  run cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo  -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  sleep 1

  run cleos push action app.pomelo unjoinround '["grant1", 101]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].grant_ids | length')
  [ $result = "0" ]

  run cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo  -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  sleep 1
}

@test "unjoin round 101 and join round 102" {

  run cleos push action app.pomelo unjoinround '["grant1", 101]' -p app.pomelo
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[0].grant_ids | length')
  [ $result = "0" ]

  run cleos push action app.pomelo joinround '["grant1", 102]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].grant_ids[0]')
  [ $result = "grant1" ]

}

@test "round #2: fund grant1 with 2 donations by 1 user" {


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

  result=$(cleos get table app.pomelo 102 match | jq -r '.rows[0].square')
  [ $result = "1237.49999999999977263" ]

}


@test "round #2: create grant2 and fund with 8 microdonations" {

  run cleos push action app.pomelo setgrant '["prjman2.eosn", "grant2", "prjgrant2", ["EOS", "USDT", "PLAY"]]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setstate '["grant2", "published"]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant2", 102]' -p app.pomelo -p prjman2.eosn
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

  result=$(cleos get table app.pomelo 102 match | jq -r '.rows[1].square')
  [ $result = "104.71793703910751105" ]

  result=$(cleos get table app.pomelo app.pomelo rounds | jq -r '.rows[1].sum_square')
  [ $result = "1342.21793703910725526" ]

  result=$(cleos get table app.pomelo 102 users | jq -r '.rows[1].contributions[0] | .id + "-" + .value')
  [ $result = "grant2-1.25000000000000000" ]
}


@test "collapse 8 users into 1 on round 102" {

  result=$(cleos get table app.pomelo 102 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[0] | .id + "-" + .value')
  [ $result = "grant1-1237.50000000000000000" ]

  result=$(cleos get table app.pomelo 102 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[1] | .id + "-" + .value')
  [ $result = "grant2-2.25000000000000000" ]

  result=$(cleos get table app.pomelo 102 users | jq -r '.rows' |  jq length)
  [ $result = "8" ]

  result=$(cleos get table app.pomelo 102 match -L grant2 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_boost + "-" + .square')
  [ $result = "8.00000000000000000-5.25000000000000000-104.71793703910751105" ]

  result=$(cleos get table app.pomelo 102 match -L grant1 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + "-" + .square')
  [ $result = "550.00000000000000000687.50000000000000000-1237.49999999999977263" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 102 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_boost + "-" + .sum_square')
  [ $result = "558.00000000000000000-692.75000000000000000-1342.21793703910725526" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 102 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "8" ]

  run cleos push action app.pomelo collapse '[["user2.eosn","user3.eosn","user4.eosn","user5.eosn","user11.eosn","user12.eosn","user13.eosn"], "user1.eosn", 102]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 102 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[0] | .id + "-" + .value')
  [ $result = "grant1-1237.50000000000000000" ]

  result=$(cleos get table app.pomelo 102 users | jq -r '.rows' |  jq length)
  [ $result = "1" ]

  result=$(cleos get table app.pomelo 102 users -L user1.eosn -l 1 | jq -r '.rows[0].contributions[1] | .id + "-" + .value')
  [ $result = "grant2-18.00000000000000000" ]

  result=$(cleos get table app.pomelo 102 match -L grant2 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + "-" + .square')
  [ $result = "8.0000000000000000010.00000000000000000-17.99999999999998934" ]

  result=$(cleos get table app.pomelo 102 match -L grant1 -l 1 | jq -r '.rows[0] | .sum_value + .sum_boost + "-" + .square')
  [ $result = "550.00000000000000000687.50000000000000000-1237.49999999999977263" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 102 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_boost + "-" + .sum_square')
  [ $result = "558.00000000000000000-697.50000000000000000-1255.49999999999977263" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 102 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "1" ]

}

@test "round #3: 4 projects, 6 users: spreadsheet simulation" {

  run cleos push action app.pomelo setround '[103, 1, "This is round 3 of Pomelo!", 100000]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos push action app.pomelo setgrant '["prjman3.eosn", "grant3", "prjgrant3", ["EOS"]]' -p app.pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setgrant '["prjman4.eosn", "grant4", "prjgrant4", ["EOS"]]' -p app.pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setstate '["grant3", "published"]' -p app.pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setstate '["grant4", "published"]' -p app.pomelo -p prjman4.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo unjoinround '["grant1", 102]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos push action app.pomelo unjoinround '["grant2", 102]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant1", 103]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant2", 103]' -p app.pomelo -p prjman2.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant3", 103]' -p app.pomelo -p prjman3.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant4", 103]' -p app.pomelo -p prjman4.eosn
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

  result=$(cleos get table app.pomelo 103 match -L grant1 | jq -r '.rows[0].square')
  [ $result = "24746.34091915152021102" ]

  result=$(cleos get table app.pomelo 103 match -L grant2 | jq -r '.rows[0].square')
  [ $result = "4193.16767251549754292" ]

  result=$(cleos get table app.pomelo 103 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "36124.99999999999272404" ]

  result=$(cleos get table app.pomelo 103 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "12500.00000000000000000" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 | jq -r '.rows[0].sum_square')
  [ $result = "77564.50859166700684000" ]

  result=$(cleos get table app.pomelo 103 users -L user5.eosn | jq -r '.rows[0].contributions[0] | .id + "-" + .value')
  [ $result = "grant3-4500.00000000000000000" ]
}

@test "change socials triggering matching update for grants in current round" {

  result=$(cleos get table app.pomelo 103 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "5000.00000000000000000" ]

  result=$(cleos get table login.eosn login.eosn users -L user11.eosn | jq -r '.rows[0].socials | length')
  [ $result = "1" ]

  result=$(cleos get table app.pomelo 103 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "36124.99999999999272404" ]

  run cleos push action login.eosn social '["user11.eosn", "sms"]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  result=$(cleos get table login.eosn login.eosn users -L user11.eosn | jq -r '.rows[0].socials | length')
  [ $result = "2" ]

  result=$(cleos get table app.pomelo 103 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "40295.28951268081436865" ]

  result=$(cleos get table app.pomelo 103 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "15000.00000000000181899" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 | jq -r '.rows[0].sum_square')
  [ $result = "84234.79810434783576056" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn | jq -r '.rows[0].contributions[0] | .id + "-" + .value')
  [ $result = "grant3-15000.00000000000000000" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn | jq -r '.rows[0].contributions[1] | .id + "-" + .value')
  [ $result = "grant4-15000.00000000000000000" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "10000.00000000000000000" ]

  run cleos push action login.eosn unsocial '["user11.eosn", "sms"]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn unsocial '["user11.eosn", "github"]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 | jq -r '.rows[0].sum_square')
  [ $result = "70716.98443416555528529" ]

  result=$(cleos get table app.pomelo 103 match -L grant4 | jq -r '.rows[0].square')
  [ $result = "10000.00000000000000000" ]

  result=$(cleos get table app.pomelo 103 match -L grant3 | jq -r '.rows[0].square')
  [ $result = "31777.47584249851934146" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn | jq -r '.rows[0].contributions[0] | .id + "-" + .value')
  [ $result = "grant3-10000.00000000000000000" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn | jq -r '.rows[0].contributions[1] | .id + "-" + .value')
  [ $result = "grant4-10000.00000000000000000" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn | jq -r '.rows[0].boost')
  [ $result = "0.00000000000000000" ]
}

@test "set bad state" {

  run cleos push action app.pomelo setstate '["grant1", "disabled"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::setstate: invalid [status]" ]] || false

  run cleos push action app.pomelo setstate '["bounty1", "published"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::setstate: status was not modified" ]] || false

  run cleos push action app.pomelo setstate '["grant115", "published"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::setstate: [project_id] does not exist" ]] || false
}

@test "disable/enable grant5" {

  run cleos push action app.pomelo setgrant '["prjman1.eosn", "grant5", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo joinround '["grant5", 103]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setstate '["grant5", "banned"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos transfer user2 app.pomelo "3000.0000 USDT" "grant:grant5" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::donate_project: project not available for donation" ]] || false

}

@test "create 4 pending grants" {

  run cleos push action app.pomelo setgrant '["prjman1.eosn", "grant11", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setgrant '["prjman1.eosn", "grant12", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos push action app.pomelo setgrant '["prjman1.eosn", "grant13", "prjgrant1", ["EOS"]]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::set_project: 3 active grants allowed per author" ]] || false
}


@test "fund grant by a user without EOSN login" {

  run cleos push action app.pomelo setstate '["grant5", "published"]' -p app.pomelo -p prjman1.eosn
  [ $status -eq 0 ]

  run cleos transfer user.noeosn app.pomelo "30.0000 EOS" "grant:grant5"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::get_user_id: account is not linked to EOSN account" ]] || false

  run cleos transfer user.noeosn app.pomelo "30.0000 EOS" "bounty:bounty1"
  [ $status -eq 1 ]
  [[ "$output" =~ "pomelo::get_user_id: account is not linked to EOSN account" ]] || false
}


@test "unjoin grant1 from round 3" {
  result=$(cleos get table app.pomelo 103 match -l 1 | jq -r '.rows[0].grant_id')
  [ $result = "grant1" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0].grant_ids | length')
  [ $result = "5" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_boost + "-" +.sum_square')
  [ $result = "28300.00000000000000000-7725.00000000000000000-70716.98443416555528529" ]

  result=$(cleos get table app.pomelo 103 users -l 1 | jq -r '.rows[0].contributions[0].id')
  [ $result = "grant1" ]

  result=$(cleos get table app.pomelo 103 users -l 1 | jq -r '.rows[0].value')
  [ $result = "1800.00000000000000000" ]

  result=$(cleos get table app.pomelo 103 users -L user3.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user3.eosn" ]

  run cleos push action app.pomelo unjoinround '["grant1", 103]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 103 match -l 1 | jq -r '.rows[0].grant_id')
  [ $result = "grant2" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0].grant_ids' | jq length)
  [ $result = "4" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_boost + "-" + .sum_square')
  [ $result = "23300.00000000000000000-3875.00000000000000000-45970.64351501401688438" ]

  result=$(cleos get table app.pomelo 103 users -l 1 | jq -r '.rows[0].contributions[0].id')
  [ $result = "grant2" ]

  result=$(cleos get table app.pomelo 103 users -l 1 | jq -r '.rows[0].value')
  [ $result = "1000.00000000000000000" ]

  result=$(cleos get table app.pomelo 103 users -L user3.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user4.eosn" ]
}

@test "remove user11.eosn from round 3" {
  result=$(cleos get table app.pomelo 103 match | jq -r '.rows' | jq length)
  [ $result = "3" ]

  result=$(cleos get table app.pomelo 103 match -L grant3 -l 1 | jq -r '.rows[0].sum_value')
  [ $result = "12100.00000000000000000" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "6" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_boost + "-" +  .sum_square')
  [ $result = "23300.00000000000000000-3875.00000000000000000-45970.64351501401688438" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user11.eosn" ]

  run cleos push action app.pomelo removeuser '["user11.eosn", 103]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo 103 match | jq -r '.rows' | jq length)
  [ $result = "2" ]

  result=$(cleos get table app.pomelo 103 match -L grant3 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_sqrt')
  [ $result = "2100.00000000000000000-78.26237921249261831" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0].user_ids' | jq length)
  [ $result = "5" ]

  result=$(cleos get table app.pomelo app.pomelo rounds -L 103 -l 1 | jq -r '.rows[0] | .sum_value + "-" + .sum_boost + "-" + .sum_square')
  [ $result = "3300.00000000000000000-3875.00000000000000000-10318.16767251549390494" ]

  result=$(cleos get table app.pomelo 103 users -L user11.eosn -l 1 | jq -r '.rows[0].user_id')
  [ $result = "user2.eosn" ]
}

@test "donate less than minamount" {
  run cleos transfer user1 app.pomelo "0.9000 USDT" "grant:grant2" --contract tethertether
  [ $status -eq 1 ]
  [[ "$output" =~ "[quantity=0.9000 USDT] is less than [tokens.min_amount=10000]" ]] || false

  run cleos transfer user1 app.pomelo "0.0999 EOS" "grant:grant3"
  [ $status -eq 1 ]
  [[ "$output" =~ "[quantity=0.0999 EOS] is less than [tokens.min_amount=10000]" ]] || false

  run cleos push action app.pomelo token '["4,EOS", "eosio.token", 1001, 1]' -p app.pomelo
  [ $status -eq 0 ]

  run cleos transfer user1 app.pomelo "0.1000 EOS" "grant:grant3"
  [ $status -eq 1 ]
  [[ "$output" =~ "[quantity=0.1000 EOS] is less than [tokens.min_amount=1001]" ]] || false
}

@test "donate with a 5% fee" {
  run cleos push action app.pomelo setconfig '[1, 500, 0, null, null]' -p app.pomelo
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

  run cleos push action app.pomelo cleartable '["transfers", null, 0]' -p app.pomelo
  [ $status -eq 0 ]

  result=$(cleos get table app.pomelo app.pomelo transfers -l 1 | jq -r '.rows | length')
  [ $result = "0" ]
}

@test "donate PLAY token" {

  run cleos transfer user1 app.pomelo "1.0000 PLAY" "grant:grant1" --contract play.pomelo
  [ $status -eq 1 ]
  [[ "$output" =~ "not acceptable tokens for this project" ]] || false

  run cleos transfer user1 app.pomelo "1.0000 PLAY" "grant:grant2" --contract play.pomelo
  [ $status -eq 0 ]
}