
@test "create grant1" {

  run cleos push action pomelo setproject '["prjman1.eosn", "grant", "grant1", "prjgrant1", [["4,EOS", "eosio.token"]]]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].id')
  [ $result = "grant1" ]

  run cleos push action pomelo enable '["grant", "grant1", "ok"]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo grants | jq -r '.rows[0].status')
  [ $result = "ok" ]

  run cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-08-25T20:00:00"]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].round')
  [ $result = "1" ]

  run cleos push action pomelo joinround '["grant1", 1]' -p pomelo -p prjman1.eosn
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0].grant_ids[0]')
  [ $result = "grant1" ]

  run cleos push action pomelo init '[1, 1]' -p pomelo
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo globals -L round.id | jq -r '.rows[0].value')
  [ $result = "1" ]

  run cleos transfer user1 pomelo "10.0000 EOS" "grant:grant1"
  echo "Output: $output"
  [ $status -eq 0 ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[0].user_id')
  [ $result = "user1.eosn" ]
  result=$(cleos get table pomelo pomelo rounds | jq -r '.rows[0] | .user_ids[0] + .accepted_tokens[0].quantity')
  [ "$result" = "user1.eosn10.0000 EOS" ]
  result=$(cleos get table pomelo pomelo transfers | jq -r '.rows[0] | .user_id + .ext_quantity.quantity')
  [ "$result" = "user1.eosn10.0000 EOS" ]
  grant_balance=$(cleos get currency balance eosio.token prjgrant1 EOS)
  [ "$grant_balance" = "10.0000 EOS" ]
}