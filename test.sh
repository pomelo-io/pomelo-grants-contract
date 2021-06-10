
cleos wallet unlock --password $(cat ~/eosio-wallet/.pass)

bats ./__tests__/system.bats
bats ./__tests__/config.bats
bats ./__tests__/users.bats
# bats ./__tests__/projects.bats
bats ./__tests__/projects2.bats