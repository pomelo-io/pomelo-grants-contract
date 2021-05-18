# 🍈 Pomelo - EOSIO Smart Contract

## Quickstart

```bash
# donate Pomelo grant
$ cleos transfer myaccount pomelo "1.0000 EOS" "grant:myproject"

# fund Pomelo bounty
$ cleos transfer myaccount pomelo "1.0000 EOS" "bounty:mywork"
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

- [ ] Users
- [ ] Social
- [ ] Grants
- [ ] Bounties
- [ ] Donations
- [ ] Restrictions
- [ ] Matching

## Table of Content

- [TABLE `users`](#table-users)
- [TABLES `grants` & `bounties`](#tables-grants-and-bounties)
- [TABLE `transfers`](#table-transfers)
- [TABLE `match.grant`](#table-match.grant)
- [TABLE `rounds`](#table-rounds)
- [ACTION `setuser`](#action-setuser)
- [ACTION `userstatus`](#action-userstatus)

## TABLE `users`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byaccount`    | 2                | i64        |
| `bystatus`     | 3                | i64        |
| `byupdated`    | 4                | i64        |

### params

- `{uint64_t} user_id` - (primary key) user ID
- `{name} [eos_account=""]` - EOS account name
- `{name} [region=""]` - user region (ex: `ca`)
- `{map<name, bool>} [social=[]]` - social accounts enabled
- `{name} [status="pending"]` - user status (ex: `pending/ok/disabled`)
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time
- `{time_point_sec} deleted_at` - deleted at time

### example

```json
{
    "user_id": 123,
    "eos_account": "myaccount",
    "region": "ca",
    "social": [{"key": "github", "value": true}],
    "status": "ok",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
    "deleted_at": "1970-01-01T00:00:00"
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
    "author_user_id": 123,
    "authorized_user_ids": [123],
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
- `{uint64_t} user_id` - Pomelo user account ID
- `{uint64_t} round_id` - participating round ID
- `{uint64_t} grant_id` - grant ID
- `{name} eos_account` - EOS account sending transfer
- `{extended_asset} amount` - amount of tokens donated
- `{double} value` - USD valuation at time of received
- `{checksum256} trx_id` - transaction ID
- `{time_point_sec} created_at` - created at time

### example

```json
{
    "transfer_id": 10001,
    "user_id": 5,
    "round": 1,
    "grant_id": 1001,
    "eos_account": "myaccount",
    "amount": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
    "value": 100.0,
    "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
    "created_at": "2020-12-06T00:00:00"
}
```

## TABLE `match.grant`

- scope: `grant_id`

### params

- `{uint64_t} round_id` - (primary key) round ID
- `{uint64_t} grant_id` - grant ID
- `{map<name, double>} user_value` - user value contributions
- `{map<name, double>} user_multiplier` - user match multiplier
- `{map<name, double>} user_match` - user match contributions
- `{map<name, double>} user_sqrt` - user sqrt contributions (quadratic funding metric)
- `{uint64_t} total_users` - total number of users
- `{double} sum_value` - sum of all user value contributions
- `{double} sum_match` - sum of all user match contributions
- `{double} sum_sqrt` - sum of square root of contributions (quadratic funding metric)
- `{double} square` - total square of the square roots (quadratic funding metric)
- `{time_point_sec} updated_at` - updated at time

### example

```json
{
    "round_id": 1,
    "grant_id": 1001,
    "user_value": [{ "key": "myaccount", "value": 100.0 }, { "key": "toaccount", "value": 50.0 }],
    "user_multiplier": [{ "key": "myaccount", "value": 2.25 }, { "key": "toaccount", "value": 2.0 }],
    "user_match": [{ "key": "myaccount", "value": 225.0 }, { "key": "toaccount", "value": 100.0 }],
    "user_sqrt":  [{ "key": "myaccount", "value": 15.0 }, { "key": "toaccount", "value": 10.0 }],
    "total_users": 2,
    "sum_value": 150.0,
    "sum_match": 325.0,
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

** TO-DO ADD sums of `donate.grants`

- `{time_point_sec} start_at` - start at time
- `{time_point_sec} end_at` - end at time
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} updated_at` - updated at time
- `{time_point_sec} deleted_at` - deleted at time

### example

```json
{
    "round": 1,
    "grant_ids": [345],
    "user_ids": [123],
    "accepted_tokens": [{"contract": "eosio.token", "quantity": "1.0000 EOS"}],
    "start_at": "2020-12-06T00:00:00",
    "end_at": "2020-12-12T00:00:00",
    "created_at": "2020-12-06T00:00:00",
    "updated_at": "2020-12-06T00:00:00",
    "deleted_at": "1970-01-01T00:00:00"
}
```

## ACTION `setuser`

- **authority**: `get_self()`

### params

- `{uint64_t} user_id` - user ID
- `{name} eos_account` - EOS account name
- `{map<name, bool>} social` - social enabled

### Example

```bash
$ cleos push action pomelo setuser '[123, "myaccount", [{"key": "github", "value": true}]]' -p pomelo
```

## ACTION `userstatus`

- **authority**: `get_self()`

### params

- `{uint64_t} user_id` - user ID
- `{name} status` - status (pending/ok/disabled/delete)

### Example

```bash
$ cleos push action pomelo userstatus '[123, "ok"]' -p pomelo
```
