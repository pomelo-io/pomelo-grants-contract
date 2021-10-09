# 🍈 Pomelo - EOSIO Smart Contract

## Usage

### `@user`
```bash
# create grant and join round
cleos push action app.pomelo setproject '["author.eosn", "grant", "grant1", "fund.eosn", ["EOS", "USDT"]]' -p author.eosn
cleos push action app.pomelo joinround '["grant1", 1]' -p author.eosn

# fund grant
cleos transfer user1 app.pomelo "10.0000 EOS" "grant:grant1"

# fund bounty
cleos transfer user1 app.pomelo "10.0000 EOS" "bounty:bounty1"
```

### `@admin`

```bash
# configure app
cleos push action app.pomelo setconfig '[1, 500]' -p app.pomelo
cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 0]' -p app.pomelo
cleos push action app.pomelo token '["4,USDT", "tethertether", 10000, 12]' -p app.pomelo

# create matching round and start it
cleos push action app.pomelo setround '[1, "2021-05-19T20:00:00", "2021-08-19T20:00:00", "Round 1", [["1000.0000 EOS", "eosio.token"]]]' -p app.pomelo

# approve grant by admin
cleos push action app.pomelo enable '["grant", "grant1", "ok"]' -p app.pomelo
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
- [TABLE `seasons`](#table-seasons)
- [TABLE `rounds`](#table-rounds)
- [TABLE `tokens`](#table-tokens)
- [ACTION `setconfig`](#action-setconfig)
- [ACTION `setseason`](#action-setseason)
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

- `{uint16_t} round_id` - round ID (0 = not active)
- `{uint64_t} grant_fee` - grant fee (bips - 1/100 1%)
- `{uint64_t} bounty_fee` - bounty fee (bips - 1/100 1%)
- `{name} login_contract` - EOSN Login contract
- `{name} fee_account` - fee

### example

```json
{
    "round_id": 1,
    "grant_fee": 500,
    "bounty_fee": 500,
    "login_contractt": "login.eosn",
    "fee_account": "fee.pomelo",
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
- `{asset} fee` - fee charged and sent to `global.fee_account`
- `{string} memo` - transfer memo
- `{name} user_id` - Pomelo user account ID
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

- `{uint16_t} season_id` - (primary key)
- `{vector<uint16_t>} round_ids` - round ids in this season
- `{string} description` - grant text description
- `{double} match_value` - estimated total matching pool value for this season

### example

```json
{
    "season_id": 1,
    "round_ids": [101, 102],
    "description": "Season #1",
    "match_value": 100000,
}
```

## TABLE `rounds`

### params

- `{uint64_t} round_id` - (primary key) matching rounds
- `{string} description` - grant text description
- `{set<name>} grant_ids` - grants IDs participating
- `{set<name>} user_ids` - user IDs participating
- `{vector<extended_asset>} donated_tokens` - donated tokens
- `{double} match_value` - total value of the matching pool
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
    "round_id": 1,
    "description": "Grant Round #1",
    "grant_ids": ["grant1"],
    "user_ids": ["user1.eosn"],
    "donated_tokens": [{"contract": "eosio.token", "quantity": "100.0000 EOS"}],
    "match_value": 100000,
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
- `{uint64_t} oracle_id` - Defibox oracle ID (0 for base token)

### example

```json
{
    "sym": "4,EOS",
    "contract": "eosio.token",
    "min_amount": 10000,
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

### params

- `{uint16_t} season_id` - season ID
- `{vector<uint16_t>} round_ids` - round ids in this season
- `{optional<string>} description` - season description
- `{optional<value>} match_value` - estimated total matching pool value for this season

### example

```bash
$ cleos push action app.pomelo setseason '[1, [101,102], "Season #1", 100000]' -p app.pomelo
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

- `{uint16_t} round_id` - round id
- `{time_point_sec} start_at` - round start time
- `{time_point_sec} end_at` - round end time
- `{string} description` - grant description
- `{double} match_value` - matching pool value

### example

```bash
$ cleos push action app.pomelo setround '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00", "Grant Round #1", 100000]' -p app.pomelo
```

## ACTION `joinround`

- **authority**: `get_self()` + `author_id`

Adds grant to round

### params

- `{name} grant_id` - grant_id
- `{uint16_t} round_id` - round_id

### example

```bash
$ cleos push action app.pomelo joinround '["grant1", 1]' -p app.pomelo -p 123.eosn
```

## ACTION `unjoinround`

- **authority**: `get_self()`

Remove grant from round and update all matchings

### params

- `{name} grant_id` - grant id
- `{uint16_t} round_id` - round id

### example

```bash
$ cleos push action app.pomelo unjoinround '["grant1", 1]' -p app.pomelo
```

## ACTION `cleartable`

Clear table

### params

- `{name} table_name` - table name, i.e. "transfers"
- `{uint16_t} [round_id]` - (optional) round ID
- `{uint64_t} [max_rows]` - (optional) max number of rows to clear, if 0 - clear all

### example

```bash
$ cleos push action app.pomelo cleartable '["transfers", 1, 500]' -p app.pomelo
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
- `{uint64_t} oracle_id` - Defibox oracle id for price discovery (0 for base token)

### example

```bash
$ cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p app.pomelo
```
