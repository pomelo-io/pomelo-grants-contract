
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

bats ./__tests__/system.bats
bats ./__tests__/users.bats