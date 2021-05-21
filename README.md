# 🍈 Pomelo - EOSIO Smart Contract

## Usage

```bash

# set status ok
$ cleos push action pomelo setstatus '["ok"]' -p pomelo

# set value symbol
cleos push action pomelo setvaluesym '[["4,USDT", "tethertether"]]' -p pomelo

# create Pomelo user for grant manager and link it to EOS account
cleos push action login.eosn create '["prjman.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn link '["prjman.eosn", ["prjman"]]' -p login.eosn

# create funding user, link to EOS account and set socials for funding boost
cleos push action login.eosn create '["user.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn link '["user.eosn", ["user"]]' -p login.eosn
cleos push action login.eosn social '["user.eosn", ["github", "twitter", "facebook", "passport", "sms"]]' -p login.eosn

# create funding round and start it
cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-08-19T20:00:00"]' -p pomelo
cleos push action pomelo startround '[1]' -p pomelo

# create grant, enable it and join round
cleos push action pomelo setgrant '["grant1", "prjman.eosn", ["prjman.eosn"], "prjgrant", [["4,USDT", "tethertether"]]]' -p pomelo
cleos push action pomelo setprjstatus '["grant1", "ok"]' -p pomelo
cleos push action pomelo joinround '["grant1", 1]' -p pomelo

# fund grant1
$ cleos transfer user pomelo "10.0000 USDT" "grant:grant1" --contract tethertether

# query transfer trx id
cleos get table pomelo pomelo transfers | jq -r '.rows[0].trx_id'

# query grant square of sums of all users contribution sqrts
cleos get table pomelo 1 match.grant -L grant1 | jq -r '.rows[0].square'
# => 22.5 ( == (sqrt(10 + 5*0.25))^2 )

# query sum of grant squares for that round
cleos get table pomelo pomelo rounds -L 1 | jq -r '.rows[0].sum_square'
# => 22.5 => grant1 receives 22.5/22.5 = 100% of matching funding

```

## Dependencies

- [eosn-login-contract](https://github.com/pomelo-io/eosn-login-contract)
- [sx.defibox](https://github.com/stableex/sx.defibox)
- [sx.utils](https://github.com/stableex/sx.utils)
- [eosio.token](https://github.com/EOSIO/eosio.contracts)

## Testing

```bash
# build contract
$ ./scripts/build.sh

# restart node, create EOSIO users, deploy contracts, issue tokens
$ ./scripts/restart

# run tests
$ ./test.sh
```

## Definitions

### Roles

| `role`        | `description`                 |
|---------------|-------------------------------|
| Backend       | Pomelo Backend                |
| Admin         | Pomelo Admins                 |
| Owners        | Grant/Bounty Owners           |
| SC            | Smart Contract                |


## SPECIFICATION

- [ ] Grants
- [ ] Bounties
- [ ] Donations
- [ ] Restrictions
- [ ] Matching

## Table of Content

- [SINGLETON `config`](#singleton-config)
- [SINGLETON `state`](#singleton-state)
- [TABLES `grants` & `bounties`](#tables-grants-and-bounties)
- [TABLE `transfers`](#table-transfers)
- [TABLE `match.grant`](#table-match.grant)
- [TABLE `rounds`](#table-rounds)
- [ACTION `setstatus`](#action-setstatus)
- [ACTION `setvaluesym`](#action-setvaluesym)
- [ACTION `setgrant`](#action-setgrant)
- [ACTION `setbounty`](#action-setbounty)
- [ACTION `setprjstatus`](#action-setprjstatus)
- [ACTION `setround`](#action-setround)
- [ACTION `startround`](#action-startround)
- [ACTION `joinround`](#action-joinround)

## SINGLETON `config`

### params

- `{name} status = "testing"` - contract status `testing/ok/maintenance`
- `{extended_symbol} value_symbol = [4,USDT@tethertether]` - value symbol
- `{name} login_contract = "login.eosn"` - login contract with user data

### example

```json
{
    "status": "testing",
    "value_symbol": "4,USDT@tethertether",
    "login_contract": "login.eosn"
}
```

## SINGLETON `state`

### params

- `{uint64_t} round = 0` - current funding round ( 0 if no ongoing round)

### example

```json
{
    "status": "testing",
    "value_symbol": "4,USDT@tethertether",
    "login_contract": "login.eosn"
}
```

## TABLES `grants` and `bounties`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byauthor`     | 2                | i64        |
| `bystatus`     | 3                | i64        |
| `byupdated`    | 4                | i64        |

### params

- `{name} id` - (primary key) project name ID (used in memo to receive funds, must be unique)
- `{name} type` - (❗️**IMMUTABLE**) project type (ex: `grant/bounty`)
- `{name} author_user_id` - (❗️**IMMUTABLE**) author (Pomelo User Id)
- `{set<name>} [authorized_user_ids=[]]` - authorized admins (Pomelo User Id)
- `{name} [funding_account=""]` - funding account (EOS account)
- `{set<extended_symbol>} [accepted_tokens=["4,EOS@eosio.token"]]` - accepted tokens (ex: `EOS/USDT`)
- `{name} [status="pending"]` - status (`pending/ok/disabled`)
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time
- `{time_point_sec} deleted_at` - deleted at time

### example

```json
{
    "id": "mygrant",
    "type": "grant",
    "author_user_id": "user1.eosn",
    "authorized_user_ids": ["user1.eosn"],
    "funding_account": "myreceiver",
    "accepted_tokens": [{"contract": "eosio.token", "symbol": "4,EOS"}],
    "status": "ok",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
    "deleted_at": "1970-01-01T00:00:00"
}
```

## TABLE `transfers`
(❗️**IMMUTABLE**)

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byuser`       | 2                | i64        |
| `byround`      | 3                | i64        |
| `bygrant`      | 4                | i64        |
| `byvalue`      | 5                | i64        |
| `bycreated`    | 6                | i64        |

### params

- `{uint64_t} transfer_id` - (primary key) token transfer ID
- `{name} user_id` - Pomelo user account ID
- `{uint64_t} round_id` - participating round ID
- `{name} grant_id` - grant ID
- `{name} eos_account` - EOS account sending transfer
- `{extended_asset} amount` - amount of tokens donated
- `{double} value` - USD valuation at time of received
- `{checksum256} trx_id` - transaction ID
- `{time_point_sec} created_at` - created at time

### example

```json
{
    "transfer_id": 10001,
    "user_id": "user1.eosn",
    "round": 1,
    "grant_id": "grant1",
    "eos_account": "myaccount",
    "amount": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
    "value": 100.0,
    "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
    "created_at": "2020-12-06T00:00:00"
}
```

## TABLE `match.grant`

- scope: `round_id`

### params

- `{name} grant_id` - (primary key) grant ID
- `{uint64_t} round_id` - round ID
- `{map<name, double>} user_value` - user value contributions
- `{map<name, double>} user_multiplier` - user boost multiplier
- `{map<name, double>} user_boost` - user contributions boost
- `{map<name, double>} user_sqrt` - user sqrt contributions (quadratic funding metric)
- `{uint64_t} total_users` - total number of users
- `{double} sum_value` - sum of all user value contributions
- `{double} sum_boost` - sum of all user contributions boosts
- `{double} sum_sqrt` - sum of square root of contributions (quadratic funding metric)
- `{double} square` - total square of the square roots (quadratic funding metric)
- `{time_point_sec} updated_at` - updated at time

### example

```json
{
    "grant_id": "grant1",
    "round_id": 1,
    "user_value": [{ "key": "myaccount", "value": 100.0 }, { "key": "toaccount", "value": 50.0 }],
    "user_multiplier": [{ "key": "myaccount", "value": 2.25 }, { "key": "toaccount", "value": 2.0 }],
    "user_boost": [{ "key": "myaccount", "value": 225.0 }, { "key": "toaccount", "value": 100.0 }],
    "user_sqrt":  [{ "key": "myaccount", "value": 15.0 }, { "key": "toaccount", "value": 10.0 }],
    "total_users": 2,
    "sum_value": 150.0,
    "sum_boost": 325.0,
    "sum_sqrt": 25.0,
    "square": 625.0,
    "updated_at": "2020-12-06T00:00:00"
}
```

## TABLE `rounds`

### params

- `{uint64_t} round` - (primary key) matching rounds
- `{set<name>} grant_ids` - grants IDs participating
- `{set<name>} user_ids` - user IDs participating
- `{vector<extended_asset>} accepted_tokens` - accepted tokens
- `{double} sum_value` - total value donated this round
- `{double} sum_boost` - total boost received this round
- `{double} sum_square` - square of total sqrt sum
- `{time_point_sec} start_at` - start at time
- `{time_point_sec} end_at` - end at time
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time
- `{time_point_sec} deleted_at` - deleted at time

### example

```json
{
    "round": 1,
    "grant_ids": ["grant1"],
    "user_ids": ["user1.eosn"],
    "accepted_tokens": [{"contract": "eosio.token", "quantity": "1.0000 EOS"}],
    "sum_value": 12345,
    "sum_boost": 3231,
    "sum_square": 423451.1233,
    "start_at": "2020-12-06T00:00:00",
    "end_at": "2020-12-12T00:00:00",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
    "deleted_at": "1970-01-01T00:00:00"
}
```

## ACTION `setstatus`

- **authority**: `get_self()`

### params

- `{name} status` - contract status

### Example

```bash
$ cleos push action pomelo setstatus '["maintenance"]' -p pomelo
```

## ACTION `setvaluesym`

- **authority**: `get_self()`

### params

- `{extended_symbol} value_symbol` - value symbol used for matching calculations

### Example

```bash
$ cleos push action pomelo setvaluesym '[["4,USDT", "tethertether"]]' -p pomelo
```

## ACTION `setgrant`

- **authority**: `get_self()`

Creates/updates grant project with specified parameters. Project is created in "pending" state.

### params

- `{name} id` - grant project id
- `{name} author_id` - author user id
- `{set<name>}` - user ids authorized to modify project
- `{name} funding_account` - account to forward donations to
- `{set<extended_symbol>} accepted_tokens` - tokens accepted by the project

### Example

```bash
$ cleos push action pomelo setgrant '["mygrant", "123.eosn", ["123.eosn"], "project2fund", [["4,USDT", "tethertether"]]]' -p pomelo
```

## ACTION `setbounty`

- **authority**: `get_self()`

Creates/updates bounty project with specified parameters. Project is created in "pending" state.

### params

- `{name} id` - bounty project id
- `{name} author_id` - author user id
- `{set<name>}` - user ids authorized to modify project
- `{name} funding_account` - account to forward donations to
- `{set<extended_symbol>} accepted_tokens` - tokens accepted by the project

### Example

```bash
$ cleos push action pomelo setbounty '["mygrant", "123.eosn", ["123.eosn"], "project2fund", [["4,USDT", "tethertether"]]]' -p pomelo
```

## ACTION `setprjstatus`

- **authority**: `get_self()`

Creates/updates bounty project with specified parameters. Project is created in "pending" state.

### params

- `{name} id` - project id
- `{name} status` - new status `pending/ok/disabled`

### Example

```bash
$ cleos push action pomelo setprjstatus '["mygrant", "ok"]' -p pomelo
```

## ACTION `setround`

- **authority**: `get_self()`

Creates/updates funding round with specified parameters.

### params

- `{uint64_t} round_id` - round_id
- `{time_point_sec} start_at` - round start time
- `{time_point_sec} end_at` - round end time

### Example

```bash
$ cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00"]' -p pomelo
```

## ACTION `startround`

- **authority**: `get_self()`

Start round by making sure round is defined and changing the state table

### params

- `{uint64_t} round_id` - round_id

### Example

```bash
$ cleos push action pomelo startround '[1]' -p pomelo
```

## ACTION `joinround`

- **authority**: `get_self()`

Adds grant to round

### params

- `{name} project_id` - project_id
- `{uint64_t} round_id` - round_id

### Example

```bash
$ cleos push action pomelo joinround '["grant1", 1]' -p pomelo
```