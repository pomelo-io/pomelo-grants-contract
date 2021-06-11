# 🍈 Pomelo - EOSIO Smart Contract

## Usage

```bash
# create matching round and start it
cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-08-19T20:00:00"]' -p pomelo
cleos push action pomelo init '[1, 1]' -p pomelo

# create Pomelo user for grant manager and link it to EOS account
cleos push action login.eosn create '["author.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn create '["fund.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn

# create matching user, link to EOS account and set socials for matching boost
cleos push action login.eosn create '["user.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn social '["user.eosn", ["github", "twitter", "facebook", "passport", "sms"]]' -p login.eosn -p user.eosn
cleos push action login.eosn link '["user.eosn", ["user.eosn"]]' -p login.eosn -p user.eosn

# create grant, enable it and join round
cleos push action pomelo setproject '["author.eosn", "grant", "grant1", "fund.eosn", [["4,EOS", "eosio.token"]]]' -p pomelo -p author.eosn
cleos push action pomelo enable '["grant", "grant1", "ok"]' -p pomelo -p author.eosn
cleos push action pomelo joinround '["grant1", 1]' -p pomelo -p author.eosn

# fund grant1
cleos transfer user.eosn pomelo "10.0000 EOS" "grant:grant1"

# query transfer trx id
cleos get table pomelo pomelo transfers | jq -r '.rows[0].trx_id'

# query grant square of sums of all users contribution sqrts
cleos get table pomelo 1 match -L grant1 | jq -r '.rows[0].square'
# => 22.5 ( == (sqrt(10 + 5*0.25))^2 )

# query sum of grant squares for that round
cleos get table pomelo pomelo rounds -L 1 | jq -r '.rows[0].sum_square'
# => 22.5 => grant1 receives 22.5/22.5 = 100% of matching funding
```

## Dependencies

- [eosn.login](https://github.com/pomelo-io/eosn.login)
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

- [TABLE `globals`](#table-globals)
- [TABLES `grants` & `bounties`](#tables-grants-and-bounties)
- [TABLE `transfers`](#table-transfers)
- [TABLE `match`](#table-match)
- [TABLE `rounds`](#table-rounds)
- [ACTION `init`](#action-init)
- [ACTION `setgrant`](#action-setgrant)
- [ACTION `setbounty`](#action-setbounty)
- [ACTION `enable`](#action-enable)
- [ACTION `setround`](#action-setround)
- [ACTION `joinround`](#action-joinround)
- [ACTION `unjoinround`](#action-unjoinround)
- [ACTION `cleartable`](#action-cleartable)

## TABLE `globals`

### params

- `{name} key` - (primary key) key
- `{uint64_t} value` - value

### example

```json
[
    { "key": "round.id", "value": 1 },
    { "key": "status", "value": 1 }
]
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
    "funding_account": "myreceiver",
    "accepted_tokens": [{"contract": "eosio.token", "symbol": "4,EOS"}],
    "status": "ok",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
    "deleted_at": "1970-01-01T00:00:00"
}
```

## TABLE `transfer`

- **scope**: `get_self() {name}`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byfrom`       | 2                | i64        |
| `byuser`       | 3                | i64        |
| `byround`      | 4                | i64        |
| `bygrant`      | 5                | i64        |
| `byvalue`      | 6                | i64        |
| `bycreated`    | 7                | i64        |

### params

- `{uint64_t} transfer_id` - (primary key) token transfer ID
- `{name} from` - EOS account sender
- `{name} to` - EOS account receiver
- `{extended_asset} ext_quantity` - amount of tokens transfered
- `{string} memo` - transfer memo
- `{name} user_id` - Pomelo user account ID
- `{uint64_t} round_id` - participating round ID
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
    "memo": "grant:grant1",
    "user_id": "user1.eosn",
    "round": 1,
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
- `{uint64_t} round_id` - round ID
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
    "total_users": 2,
    "sum_value": 150.0,
    "sum_boost": 325.0,
    "sum_sqrt": 25.0,
    "square": 625.0,
    "updated_at": "2020-12-06T00:00:00"
}
```

## TABLE `users`

- **scope**: `round_id {uint64_t}`

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

## ACTION `init`

- **authority**: `get_self()`

Set contract status and/or start/end round

### params

- `{uint64_t} round_id` - round ID (0=not active)
- `{uint64_t} status` - contract status (0=testing, 1=ok, 2=maintenance)

### example

```bash
$ cleos push action pomelo init '[1, 1]' -p pomelo
$ cleos push action pomelo init '[0, 2]' -p pomelo
```

## ACTION `setproject`

Create/update grant/bounty project without modifying project status

### params

- `{name} author_id` - author user id
- `{name} project_type` - project type (grant/bounty)
- `{name} project_id` - project ID
- `{name} funding_account` - account to forward donations to
- `{set<extended_symbol>} accepted_tokens` - accepted tokens

### example

```bash
$ cleos push action pomelo setproject '["123.eosn", "grant", "mygrant", "project2fund", [["4,USDT", "tethertether"]]]' -p pomelo -p 123.eosn
```

## ACTION `enable`

- **authority**: `get_self()` + `author_id`

Enable/disable grant or bounty

### params

- `{name} project_type` - project type `grant/bounty`
- `{name} project_id` - project ID
- `{name} status` - project status (0=pending, 1=ok, 2=disabled)

### example

```bash
$ cleos push action pomelo enable '["grant", "grant1", 1]' -p pomelo
```

## ACTION `setround`

- **authority**: `get_self()` + `author_id`

Creates/updates match round with specified parameters.

### params

- `{uint64_t} round_id` - round_id
- `{time_point_sec} start_at` - round start time
- `{time_point_sec} end_at` - round end time

### example

```bash
$ cleos push action pomelo setround '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00"]' -p pomelo
```

## ACTION `joinround`

- **authority**: `get_self()` + `author_id`

Adds grant to round

### params

- `{name} grant_id` - grant_id
- `{uint64_t} round_id` - round_id

### example

```bash
$ cleos push action pomelo joinround '["grant1", 1]' -p pomelo -p 123.eosn
```

## ACTION `unjoinround`

- **authority**: `get_self()`

Remove grant from round and update all matchings

### params

- `{name} grant_id` - grant id
- `{uint64_t} round_id` - round id

### example

```bash
$ cleos push action pomelo unjoinround '["grant1", 1]' -p pomelo
```

## ACTION `cleartable`

- **authority**: `get_self()`

Clear table

### params

- `{name} table_name` - table to clear

### example

```bash
$ cleos push action pomelo cleartable '["transfers"]' -p pomelo
```

## ACTION `removeuser`

- **authority**: `get_self()`

Remove user from all projects at this round and update all matchings

### params

- `{name} user_id` - user id
- `{uint64_t} round_id` - round id

### example

```bash
$ cleos push action pomelo removeuser '["user1.eosn", 1]' -p pomelo
```