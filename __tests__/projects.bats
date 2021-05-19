#!/usr/bin/env bats

@test "create grant" {

  run cleos push action pomelo setgrant '["grant1", "prjman1", ["prjman1"], "prjgrant1", [["4,USDT", "tethertether"]]]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].id')
  [ $result = "grant1" ]

}
