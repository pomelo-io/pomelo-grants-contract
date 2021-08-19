# 🍈 Pomelo - EOSIO Smart Contract

## Usage

```bash
#init config and set status
cleos push action app.pomelo init '[]' -p app.pomelo
cleos push action app.pomelo setconfig '[status, 1]' -p app.pomelo

# create matching round and start it
cleos push action app.pomelo setround '[1, "2021-05-19T20:00:00", "2021-08-19T20:00:00"]' -p app.pomelo
cleos push action app.pomelo setconfig '[roundid, 1]' -p app.pomelo

# create Pomelo user for grant manager and link it to EOS account
cleos push action login.eosn create '["author.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn create '["fund.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn

# create matching user, link to EOS account and set socials for matching boost
cleos push action login.eosn create '["user.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
cleos push action login.eosn social '["user.eosn", ["github", "twitter", "facebook", "passport", "sms"]]' -p login.eosn -p user.eosn
cleos push action login.eosn link '["user.eosn", ["user1"]]' -p login.eosn -p user.eosn

# create grant, enable it and join round
cleos push action app.pomelo setproject '["author.eosn", "grant", "grant1", "fund.eosn", [["4,EOS", "eosio.token"]]]' -p app.pomelo -p author.eosn
cleos push action app.pomelo enable '["grant", "grant1", "ok"]' -p app.pomelo -p author.eosn
cleos push action app.pomelo joinround '["grant1", 1]' -p app.pomelo -p author.eosn

# fund grant1
cleos transfer user1 app.pomelo "10.0000 EOS" "grant:grant1"

# query transfer trx id
cleos get table app.pomelo app.pomelo transfers | jq -r '.rows[0].trx_id'

# query grant square of sums of all users contribution sqrts
cleos get table app.pomelo 1 match -L grant1 | jq -r '.rows[0].square'
# => 22.5 ( == (sqrt(10 + 5*0.25))^2 )

# query sum of grant squares for that round
cleos get table app.pomelo app.pomelo rounds -L 1 | jq -r '.rows[0].sum_square'
# => 22.5 => grant1 receives 22.5/22.5 = 100% of matching funding

# clear 100 rows from transfers table
cleos push action app.pomelo cleartable '["transfers", 100]' -p app.pomelo
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
- [TABLE `tokens`](#table-tokens)
- [ACTION `init`](#action-init)
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `setproject`](#action-setproject)
- [ACTION `enable`](#action-enable)
- [ACTION `setround`](#action-setround)
- [ACTION `joinround`](#action-joinround)
- [ACTION `unjoinround`](#action-unjoinround)
- [ACTION `cleartable`](#action-cleartable)
- [ACTION `removeuser`](#action-removeuser)
- [ACTION `collapse`](#action-collapse)
- [ACTION `token`](#action-token)

## TABLE `globals`

### params

- `{name} key` - (primary key) key
- `{uint64_t} value` - value

### example

```json
[
    { "key": "roundid", "value": 1 },
    { "key": "status", "value": 1 },
    { "key": "systemfee", "value": 500 },
    { "key": "minamount", "value": 1000 }
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
- `{set<symbol_code>} [accepted_tokens=["EOS"]]` - accepted tokens (ex: `["EOS"]`)
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
    "accepted_tokens": ["EOS"],
    "status": "ok",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
    "deleted_at": "1970-01-01T00:00:00"
}
```

## TABLE `transfers`

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
- `{asset} fee` - system fee charged
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
- `{string} description` - grant text description
- `{set<name>} grant_ids` - grants IDs participating
- `{set<name>} user_ids` - user IDs participating
- `{vector<extended_asset>} donated_tokens` - donated tokens
- `{vector<extended_asset>} match_tokens` - matching pool tokens
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
    "description": "Grant Round #1",
    "grant_ids": ["grant1"],
    "user_ids": ["user1.eosn"],
    "donated_tokens": [{"contract": "eosio.token", "quantity": "100.0000 EOS"}],
    "match_tokens": [{"contract": "eosio.token", "quantity": "1000.0000 EOS"}],
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

## TABLE `tokens`

### params

- `{symbol} sym` - (primary key) symbol
- `{name} contract` - token contract
- `{uint64_t} min_amount` - min amount required when donating
- `{string} description` - token description
- `{string} url` - token url

### example

```json
{
    "sym": "4,EOS",
    "contract": "eosio.token",
    "min_amount": 10000,
    "description": "EOS token",
    "url": "https://eos.io"
}
```

## ACTION `init`

- **authority**: `get_self()`

Init contract config with default parameters (status=2, roundid=0, systemfee=0)

### params

### example

```bash
$ cleos push action app.pomelo init '[]' -p app.pomelo
```

## ACTION `setconfig`

- **authority**: `get_self()`

Set contract config key/value
- `status` - contract status (0=testing, 1=ok, 2=maintenance)
- `roundid` - ongoing round (0=not active)
- `systemfee` - donation fee (500=5%)

### params

- `{name} key` - config key
- `{uint64_t} value` - config value

### example

```bash
$ cleos push action app.pomelo init '[status, 1]' -p app.pomelo
$ cleos push action app.pomelo init '[roundid, 3]' -p app.pomelo
```

## ACTION `setproject`

Create/update grant/bounty project without modifying project status

### params

- `{name} author_id` - author user id - cannot be modified
- `{name} project_type` - project type (grant/bounty) - cannot be modified
- `{name} project_id` - project ID - cannot be modified
- `{name} funding_account` - account to forward donations to, when creating bounty should be `""_n`
- `{set<symbol_code>} accepted_tokens` - accepted tokens

### example

```bash
$ cleos push action app.pomelo setproject '["123.eosn", "grant", "mygrant", "project2fund", ["EOS"]]' -p app.pomelo -p 123.eosn
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
$ cleos push action app.pomelo enable '["grant", "grant1", 1]' -p app.pomelo
```

## ACTION `setround`

Create/update round

### params

- `{uint64_t} round_id` - round id
- `{time_point_sec} start_at` - round start time
- `{time_point_sec} end_at` - round end time
- `{string} description` - grant description
- `{vector<extended_asset>} match_tokens` - matching pool tokens

### example

```bash
$ cleos push action app.pomelo setround '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00", "Grant Round #1", [["1000.0000 EOS", "eosio.token"]]]' -p app.pomelo
```

## ACTION `joinround`

- **authority**: `get_self()` + `author_id`

Adds grant to round

### params

- `{name} grant_id` - grant_id
- `{uint64_t} round_id` - round_id

### example

```bash
$ cleos push action app.pomelo joinround '["grant1", 1]' -p app.pomelo -p 123.eosn
```

## ACTION `unjoinround`

- **authority**: `get_self()`

Remove grant from round and update all matchings

### params

- `{name} grant_id` - grant id
- `{uint64_t} round_id` - round id

### example

```bash
$ cleos push action app.pomelo unjoinround '["grant1", 1]' -p app.pomelo
```

## ACTION `cleartable`

- **authority**: `get_self()`

Clear table

### params

- `{name} table_name` - table to clear
- `{uint64_t} max_rows` - max number of rows to clear, if 0 - clear all

### example

```bash
$ cleos push action app.pomelo cleartable '["transfers", 100]' -p app.pomelo
```

## ACTION `removeuser`

- **authority**: `get_self()`

Remove user from all projects at this round and update all matchings

### params

- `{name} user_id` - user id
- `{uint64_t} round_id` - round id

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
- `{uint64_t} round_id` - round ID

### example

```bash
$ cleos push action app.pomelo collapse '[["user2.eosn","user3.eosn","user4.eosn"], "user1.eosn", 1]' -p app.pomelo
```

## ACTION `token`

Set token information

### params

- `{extended_symbol} token` - (primary key) extended symbol
- `{uint64_t} min_amount` - min amount required when donating
- `{string} [description=""]` - (optional) token description
- `{string} [url=""]` - (optional) token url

### example

```bash
$ cleos push action app.pomelo token '[[["eosio.token", "4,EOS"]], 10000, "EOS Token", "https://eosio.io"]' -p app.pomelo
```