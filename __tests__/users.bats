#!/usr/bin/env bats

@test "create users" {

  run cleos push action login.eosn create '["prjman1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].user_id')
  [ $result = "prjman1.eosn" ]

  run cleos push action login.eosn create '["prjman2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["prjman3.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["prjman4.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user1.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user2.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user3.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user4.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user5.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user11.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user12.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn create '["user13.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
  [ $status -eq 0 ]

  result=$(cleos get table login.eosn login.eosn users -l 20 | jq -r '.rows | length')
  [ $result = "12" ]

  run cleos push action login.eosn link '["prjman1.eosn", "prjman1", "SIG_K1_KjnbJ2m22HtuRW7u7ZLdoCx76aNMiADHJpATGh32uYeJLdSjhdpHA7tmd4pj1Ni3mSr5DPRHHaydpaggrb5RcBg2HDDn7G"]' -p prjman1
  [ $status -eq 0 ]
  result=$(cleos get table login.eosn login.eosn users | jq -r '.rows[0].accounts[0]')
  [ $result = "prjman1" ]

  run cleos push action login.eosn link '["prjman2.eosn", "prjman2", "SIG_K1_K4LEhA2WrPn9SAHHFTZoMJEqzvk52YViyJYgkvBg6f9VKX46RbdDVy6mhqTv6pUENRbQVPmtKwK7eU4PTqfDX8XH8nUn4i"]' -p prjman2
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["prjman3.eosn", "prjman3", "SIG_K1_KiBpiqFpZN87fAAyC6qEAjKvVYqrvTboA9pPUL3ueihpUjLefnDA88MGTqVT3j54cjvbMwLTkoxp3HqffWYrcapRjGs1vz"]' -p prjman3
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["prjman4.eosn", "prjman4", "SIG_K1_JvbYK4sf2eRCPiB1UMNJV6a2nZwTWQmEa4fay8PntsoniuYTAJBqpi9jze25XopHQ8g8WngieaTLVbWe9n1Hk2eTptE6s7"]' -p prjman4
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user1.eosn", "user1", "SIG_K1_JzJsKRsfVsDJv52xz1DQakvmXQ2NeJr1kwGYrtt4ttN6Cudzm4wG5fmS1ak7JCVRJabM7sYGwk5gpX8TBq2EymQQ1LNUe3"]' -p user1
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user2.eosn", "user2", "SIG_K1_KYuJhk7iCjKkct5MxeMrTuUEmHgd9S27n33sWGGoT45x8J85xMVnpi6k2UWUgx9s5Wc7K76KryoKC8YuRkcXnv51cXqomo"]' -p user2
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user3.eosn", "user3", "SIG_K1_KcJy4yEpUYbLQWfpaNo5EcjMoGMTXcyjyrymWpJyXwrnJSBWtsjibWKcGTWtEnWb4WMfXqtEtFUt3AkauEqLPXt9Q2evBv"]' -p user3
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user4.eosn", "user4", "SIG_K1_Kh2ZLM7fpoWCA2JhpMNzsf8ogh3obYbnVnUoC5Rzt5tsD7c9M1SN4xLPEKDHvhH8YfgiYQxcngrawp2yCH61KScZHcof4J"]' -p user4
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user5.eosn", "user5", "SIG_K1_KcJJJHRTsEnz2BXs85ZoYzsd5fEH1nREfSkYkgfDdLSk51CLC8H7K6VmdmjcScx5E7bj9PKCoYtn6AVsHKKvGCKPtQkfrH"]' -p user5
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user11.eosn", "user11", "SIG_K1_KagdqsCuLitjZ9J7FiN9CHhRXxBWBDnvc5guagT2EaGrHRyGY7Zch5PeT2dacfnBpun44QUXrNBH8Z5xLVzQw4R4BEypdM"]' -p user11
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user12.eosn", "user12", "SIG_K1_KBvNdEPiiokXokFii83oewbPrhEaek6fBB8Uz2Ea3jineZuQW4fqC9pe1R4wsP3jgBHQHfff9ps9ViXtViV2hGfTk3GbJT"]' -p user12
  [ $status -eq 0 ]

  run cleos push action login.eosn link '["user13.eosn", "user13", "SIG_K1_Kc5SeEZoKgWiQcFqTnJRYkiBxvuK2w2MJtySJHvUH2MSX6i7xr9foLkfFDfUCS6c7PeRNkz43iY5kwdvTWw5QhRgivSDGh"]' -p user13
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user1.eosn", "github"]' -p login.eosn -p user1.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user1.eosn", "twitter"]' -p login.eosn -p user1.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user1.eosn", "facebook"]' -p login.eosn -p user1.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user1.eosn", "passport"]' -p login.eosn -p user1.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user1.eosn", "sms"]' -p login.eosn -p user1.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user2.eosn", "github"]' -p login.eosn -p user2.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user2.eosn", "twitter"]' -p login.eosn -p user2.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user3.eosn", "github"]' -p login.eosn -p user3.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user3.eosn", "twitter"]' -p login.eosn -p user3.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user3.eosn", "facebook"]' -p login.eosn -p user3.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user4.eosn", "github"]' -p login.eosn -p user4.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user5.eosn", "github"]' -p login.eosn -p user5.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user5.eosn", "twitter"]' -p login.eosn -p user5.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user5.eosn", "facebook"]' -p login.eosn -p user5.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user5.eosn", "passport"]' -p login.eosn -p user5.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user5.eosn", "sms"]' -p login.eosn -p user5.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user11.eosn", "github"]' -p login.eosn -p user11.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user12.eosn", "github"]' -p login.eosn -p user12.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user12.eosn", "twitter"]' -p login.eosn -p user12.eosn
  [ $status -eq 0 ]

  run cleos push action login.eosn social '["user13.eosn", "github"]' -p login.eosn -p user13.eosn
  [ $status -eq 0 ]
  run cleos push action login.eosn social '["user13.eosn", "twitter"]' -p login.eosn -p user13.eosn
  [ $status -eq 0 ]

}
