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
- [TABLE `grants`](#table-grants)
- [TABLE `bounties`](#table-bounties)
- [TABLE `rounds`](#table-rounds)
- [ACTION `userstatus`](#action-userstatus)
- [ACTION `deluser`](#action-deluser)

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

## TABLE `grants` & `bounties`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byname`       | 2                | i64        |
| `byauthor`     | 3                | i64        |
| `bystatus`     | 4                | i64        |
| `byupdated`    | 5                | i64        |

### params

- `{uint64_t} id` - (primary key) project ID
- `{name} name` - (❗️**IMMUTABLE**) project name (used in memo to receive funds, must be unique)
- `{name} category` - (❗️**IMMUTABLE**) project category (ex: `grant/bounty`)
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
    "id": 345,
    "name": "mygrant",
    "category": "grant",
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

## TABLE `rounds`

### params

- `{uint64_t} round` - (primary key) matching rounds
- `{set<name>} grant_ids` - grants IDs participating
- `{set<name>} user_ids` - user IDs participating
- `{vector<extended_asset>} accepted_tokens` - accepted tokens
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
$ cleos push action battle.gems setuser '[123, "myaccount", [{"key": "github", "value": true}]]' -p battle.gems
```

## ACTION `userstatus`

- **authority**: `get_self()`

### params

- `{uint64_t} user_id` - user ID
- `{name} status` - status (pending/ok/disabled/delete)

### Example

```bash
$ cleos push action battle.gems userstatus '[123, "ok"]' -p battle.gems
```
