# 🍈 Pomelo - Antelope Smart Contract

## Security Audits

- <a href="https://s3.eu-central-1.wasabisys.com/audit-certificates/Smart%20Contract%20Audit%20Certificate%20-%20Pomelo.app.pdf"><img height=30px src="https://user-images.githubusercontent.com/550895/132641907-6425e632-1b1b-4015-9b84-b7f26a25ec58.png" /> Sentnl Audit</a> (2021-10)

## Usage

### `@user`
```bash
# create grant and join round
cleos push action app.pomelo setproject '["author.eosn", "grant", "grant1", "fundingacc", ["EOS", "USDT"]]' -p author.eosn
cleos push action app.pomelo joinround '["grant1", 101]' -p author.eosn

# fund grant
cleos transfer user1 app.pomelo "10.0000 EOS" "grant:grant1"

# fund bounty
cleos transfer user1 app.pomelo "10.0000 EOS" "bounty:bounty1"
```

### `@admin`

```bash
# configure app
cleos push action app.pomelo setconfig '[1, 500, 500, "login.eosn", "fee.pomelo"]' -p app.pomelo
cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p app.pomelo
cleos push action app.pomelo token '["4,USDT", "tethertether", 10000, 0]' -p app.pomelo

# create matching round and season add it to season and start it
cleos push action app.pomelo setseason '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00", "2021-05-19T20:00:00", "2021-05-25T20:00:00", "Season 1", 100000]' -p app.pomelo
cleos push action app.pomelo setround '[101, 1, "Grant Round #1", 100000]' -p app.pomelo


# approve grant by admin
cleos push action app.pomelo setstate '["grant1", "published"]' -p app.pomelo
```

## Dependencies

- [eosn.login](https://github.com/pomelo-io/eosn.login)
- [sx.utils](https://github.com/stableex/sx.utils)
- [eosio.token](https://github.com/eosnetworkfoundation/eos-system-contracts)

## Testing

```bash
# build contract
$ ./scripts/build.sh

# restart node, create Antelope users, deploy contracts, issue tokens
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

## Table of Content

- [🍈 Pomelo - Antelope Smart Contract](#-pomelo---antelope-smart-contract)
  - [Security Audits](#security-audits)
  - [Usage](#usage)
    - [`@user`](#user)
    - [`@admin`](#admin)
  - [Dependencies](#dependencies)
  - [Testing](#testing)
  - [Definitions](#definitions)
    - [Roles](#roles)
  - [Table of Content](#table-of-content)
  - [TABLE `globals`](#table-globals)
    - [example](#example)
  - [TABLES `grants`](#tables-grants)
    - [multi-indexes](#multi-indexes)
    - [params](#params)
    - [example](#example-1)
  - [TABLE `transfers`](#table-transfers)
    - [params](#params-1)
    - [example](#example-2)
  - [TABLE `match`](#table-match)
    - [params](#params-2)
    - [example](#example-3)
  - [TABLE `users`](#table-users)
    - [multi-indexes](#multi-indexes-1)
    - [params](#params-3)
    - [example](#example-4)
  - [TABLE `seasons`](#table-seasons)
    - [params](#params-4)
    - [example](#example-5)
  - [TABLE `rounds`](#table-rounds)
    - [params](#params-5)
    - [example](#example-6)
  - [TABLE `tokens`](#table-tokens)
    - [params](#params-6)
    - [example](#example-7)
  - [ACTION `setconfig`](#action-setconfig)
    - [params](#params-7)
    - [example](#example-8)
  - [ACTION `setseason`](#action-setseason)
    - [params](#params-8)
    - [example](#example-9)
  - [ACTION `setgrant`](#action-setgrant)
    - [params](#params-9)
    - [example](#example-10)
  - [ACTION `setstate`](#action-setstate)
    - [params](#params-10)
    - [example](#example-11)
  - [ACTION `setround`](#action-setround)
    - [params](#params-11)
    - [example](#example-12)
  - [ACTION `joinround`](#action-joinround)
    - [params](#params-12)
    - [example](#example-13)
  - [ACTION `unjoinround`](#action-unjoinround)
    - [params](#params-13)
    - [example](#example-14)
  - [ACTION `cleartable`](#action-cleartable)
    - [params](#params-14)
    - [example](#example-15)
  - [ACTION `removeuser`](#action-removeuser)
    - [params](#params-15)
    - [example](#example-16)
  - [ACTION `collapse`](#action-collapse)
    - [params](#params-16)
    - [example](#example-17)
  - [ACTION `token`](#action-token)
    - [params](#params-17)
    - [example](#example-18)

## TABLE `globals`

- `{uint16_t} season_id` - season ID (0 = not active)
- `{uint64_t} grant_fee` - grant fee (bips - 1/100 1%)
- `{uint64_t} bounty_fee` - bounty fee (bips - 1/100 1%)
- `{name} login_contract` - EOSN Login contract
- `{name} fee_account` - fee

### example

```json
{
    "season_id": 1,
    "grant_fee": 500,
    "bounty_fee": 500,
    "login_contractt": "login.eosn",
    "fee_account": "fee.pomelo",
}
```

## TABLES `grants`

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
- `{name} [funding_account=""]` - funding account (EOS account)
- `{set<symbol_code>} [accepted_tokens=["EOS"]]` - accepted tokens (ex: `["EOS"]`)
- `{name} [status="pending"]` - status (`pending/published/retired/banned/denied`)
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time

### example

```json
{
    "id": "mygrant",
    "type": "grant",
    "author_user_id": "user1.eosn",
    "funding_account": "myreceiver",
    "accepted_tokens": ["EOS"],
    "status": "published",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
}
```

## TABLE `transfers`

- **scope**: `get_self()`

### params

- `{uint64_t} transfer_id` - (primary key) token transfer ID
- `{name} from` - EOS account sender
- `{name} to` - EOS account receiver
- `{extended_asset} ext_quantity` - amount of tokens transfered
- `{asset} fee` - fee charged and sent to `global.fee_account`
- `{string} memo` - transfer memo
- `{name} user_id` - Pomelo user account ID
- `{uint16_t} season_id` - participating season ID
- `{uint16_t} round_id` - participating round ID
- `{name} project_type` - project type ("grant" / "bounty")
- `{name} project_id` - project ID
- `{double} value` - valuation at time of received
- `{checksum256} trx_id` - transaction ID
- `{time_point_sec} created_at` - created at time

### example

```json
{
    "transfer_id": 10001,
    "from": "myaccount",
    "to": "pomelo",
    "ext_quantity": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
    "fee": "1.0000 EOS",
    "memo": "grant:grant1",
    "user_id": "user1.eosn",
    "season_id": 1,
    "round_id": 101,
    "project_type": "grant",
    "project_id": "grant1",
    "value": 100.0,
    "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
    "created_at": "2020-12-06T00:00:00"
}
```

## TABLE `match`

- **scope**: `round_id {uint64_t}`

### params

- `{name} grant_id` - (primary key) grant ID
- `{uint16_t} round_id` - round ID
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
    "round_id": 101,
    "total_users": 2,
    "sum_value": 150.0,
    "sum_boost": 325.0,
    "sum_sqrt": 25.0,
    "square": 625.0,
    "updated_at": "2020-12-06T00:00:00"
}
```

## TABLE `users`

- **scope**: `{uint16_t} round_id`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `bydonated`    | 2                | i64        |
| `byboosted`    | 3                | i64        |

### params

- `{name} user_id` - (primary key) grant ID
- `{double} multiplier` - current user multiplier
- `{double} value` - total amount contributed by user in this round
- `{double} boost` - total boost amount for user in this round
- `{vector<contribution_t>} contributions` - vector of user's contributions (donated + boost) to each project
- `{time_point_sec} updated_at` - updated at time

### example

```json
{
    "user_id": "user1",
    "multiplier": 1.25,
    "contributions": [{"id": "grant1", "value": 24.0}, {"id": "grant2", "value": 10.0}],
    "updated_at": "2020-12-06T00:00:00"
}

```

## TABLE `seasons`

### params

- `{uint16_t} season_id` - (primary key) season_id
- `{string} description` - season description
- `{vector<uint16_t>} round_ids` - round ids participating in this season
- `{double} match_value` - total matching pool value for this season
- `{time_point_sec} start_at` - start at time
- `{time_point_sec} end_at` - end at time
- `{time_point_sec} submission_start_at` - submission start time
- `{time_point_sec} submission_end_at` - submission end time
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time

### example

```json
{
    "season_id": 1,
    "description": "Season #1",
    "round_ids": [101, 102, 103],
    "match_value": 100000,
    "start_at": "2020-12-06T00:00:00",
    "end_at": "2020-12-12T00:00:00",
    "submission_start_at": "2020-11-06T00:00:00",
    "submission_end_at": "2020-12-06T00:00:00",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
}
```

## TABLE `rounds`

### params

- `{uint64_t} round_id` - (primary key) matching rounds
- `{string} description` - grant text description
- `{uint16_t} season_id` - season ID
- `{set<name>} grant_ids` - grants IDs participating
- `{set<name>} user_ids` - user IDs participating
- `{vector<extended_asset>} donated_tokens` - donated tokens
- `{double} match_value` - estimated value of the matching pool
- `{double} sum_value` - total value donated this round
- `{double} sum_boost` - total boost received this round
- `{double} sum_square` - square of total sqrt sum
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time

### example

```json
{
    "round_id": 101,
    "description": "Grant Round #1",
    "season_id": 1,
    "grant_ids": ["grant1"],
    "user_ids": ["user1.eosn"],
    "donated_tokens": [{"contract": "eosio.token", "quantity": "100.0000 EOS"}],
    "match_value": 100000,
    "sum_value": 12345,
    "sum_boost": 3231,
    "sum_square": 423451.1233,
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
}
```

## TABLE `tokens`

### params

- `{symbol} sym` - (primary key) symbol
- `{name} contract` - token contract
- `{uint64_t} min_amount` - min amount required when donating
- `{name} oracle_contract` - Oracle contract (swap.defi, oracle.defi)
- `{uint64_t} oracle_id` - Defibox Oracle ID for oracle.defi or pair ID for swap.defi

### example

```json
{
    "sym": "4,EOS",
    "contract": "eosio.token",
    "min_amount": 10000,
    "oracle_contract": "oracle.defi",
    "oracle_id": 1
}
```

## ACTION `setconfig`

### params

- `{uint16_t} season_id` - season ID (0 = not active)
- `{optional<uint64_t>} grant_fee` - grant fee (bips - 1/100 1%)
- `{optional<uint64_t>} bounty_fee` - bounty fee (bips - 1/100 1%)
- `{optional<name>} login_contract` - EOSN Login contract
- `{optional<name>} fee_account` - fee account

### example

```bash
$ cleos push action app.pomelo setconfig '[1, 500, 500, "login.eosn", "fee.pomelo"]' -p app.pomelo
```

## ACTION `setseason`

Set season parameters. If optional parameter undefined - don't change it. If all parameters undefined - delete

### params

- `{uint16_t} season_id` - season ID (0 = not active)
- `{optional<time_point_sec>} start_at` - round start time
- `{optional<time_point_sec>} end_at` - round end time
- `{optional<time_point_sec>} submission_start_at` - round submission start time
- `{optional<time_point_sec>} submission_end_at` - round submission end time
- `{optional<string>} description` - season description
- `{optional<double>} match_value` - match value (for information purposes)

### example

```bash
$ cleos push action app.pomelo setseason '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00", "2021-05-19T20:00:00", "2021-05-25T20:00:00", "Season 1", 100000]' -p app.pomelo
```

## ACTION `setgrant`

Create/update grant - wrapper for setproject for grants

### params

- `{name} author_id` - author user id - cannot be modified
- `{name} project_id` - project ID - cannot be modified
- `{name} funding_account` - account to forward donations to
- `{set<symbol_code>} accepted_tokens` - accepted tokens

### example

```bash
$ cleos push action app.pomelo setgrant '["123.eosn", "mygrant", "project2fund", ["EOS"]]' -p app.pomelo -p 123.eosn
```

## ACTION `setstate`

- **authority**: `get_self()`

Set grant or bounty state

### params

- `{name} project_id` - project ID
- `{name} status` - project status ("pending", "published", "retired", "banned", "denied")

### example

```bash
$ cleos push action app.pomelo setstate '["grant1", "published"]' -p app.pomelo
```

## ACTION `setround`

Create/update round. If a parameter is null - don't change it

### params

- `{uint16_t} round_id` - round id
- `{uint16_t} season_id` - season id
- `{optional<string>} description` - grant description
- `{optional<double>} match_value` - total value of the matching pool

### example

```bash
$ cleos push action app.pomelo setround '[101, 1, "Grant Round #1", 100000]' -p app.pomelo
```

## ACTION `joinround`

- **authority**: `get_self()` + `author_id`

Adds grant to round

### params

- `{name} grant_id` - grant_id
- `{uint16_t} round_id` - round_id

### example

```bash
$ cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p 123.eosn
```

## ACTION `unjoinround`

- **authority**: `get_self()`

Remove grant from round and update all matchings

### params

- `{name} grant_id` - grant id
- `{uint16_t} round_id` - round id

### example

```bash
$ cleos push action app.pomelo unjoinround '["grant1", 101]' -p app.pomelo
```

## ACTION `cleartable`

Clear table

### params

- `{name} table_name` - table name, i.e. "transfers"
- `{uint16_t} [round_id]` - (optional) round ID
- `{uint64_t} [max_rows]` - (optional) max number of rows to clear, if 0 - clear all

### example

```bash
$ cleos push action app.pomelo cleartable '["transfers", 101, 500]' -p app.pomelo
```

## ACTION `removeuser`

- **authority**: `get_self()`

Remove user from all projects at this round and update all matchings

### params

- `{name} user_id` - user id
- `{uint16_t} round_id` - round id

### example

```bash
$ cleos push action app.pomelo removeuser '["user1.eosn", 1]' -p app.pomelo
```

## ACTION `collapse`

- **authority**: `get_self()`

Collapse donations from {user_ids} users into {user_id} in {round_id} and recalculate all matchings

### params

- `{set<name>} user_ids` - user IDs to collapse
- `{name} user_id` - user ID to collapse into
- `{uint16_t} round_id` - round ID

### example

```bash
$ cleos push action app.pomelo collapse '[["user2.eosn","user3.eosn","user4.eosn"], "user1.eosn", 1]' -p app.pomelo
```

## ACTION `token`

Set token information

### params

- `{symbol} sym` - (primary key) symbol
- `{name} contract` - token contract
- `{uint64_t} min_amount` - min amount required when donating
- `{name} oracle_contract` - Oracle contract, i.e. swap.defi, oracle.defi
- `{uint64_t} oracle_id` - Defibox oracle ID (for oracle.defi) or pair ID (for swap.defi)

### example

```bash
$ cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, "oracle.defi", 1]' -p app.pomelo
```
