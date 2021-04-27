# 🍈 Pomelo - EOSIO Smart Contract

## Quickstart

```bash
# donate Pomelo grant
$ cleos transfer myaccount pomelo "1.0000 EOS" "grant:myproject"

# fund Pomelo bounty
$ cleos transfer myaccount pomelo "1.0000 EOS" "bounty:mywork"
```

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
- `{name} eos_account` - (❗️**RESTRICTED**) EOS account name (will be used as default `receiver` if none is provided)
- `{map<name, bool>} social` - social accounts enabled
- `{name} status` - user status (`pending/ok/disabled`)
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} last_updated` - last updated

### example

```json
{
    "user_id": 123,
    "eos_account": "myaccount",
    "social": [{"key": "github", "value": true}],
    "status": "ok",
    "created_at": "2020-12-06T00:00:00",
    "last_updated": "2020-12-06T00:00:00"
}
```

## TABLE `*base*`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byname`       | 2                | i64        |
| `byauthor`     | 3                | i64        |
| `bystatus`     | 4                | i64        |
| `byupdated`    | 5                | i64        |

### params

- `{name} author_user_id` - author (Pomelo User Id)
- `{name} [receiver=""]` - (❗️**RESTRICTED**) receiver of funds (EOS account)
- `{set<name>} [authorized_accounts=[]]` - (❗️**RESTRICTED**) authorized admin (EOS accounts)
- `{name} status` - status (`pending/ok/disabled`)
- `{time_point_sec} created_at` - created at time
- `{time_point_sec} last_updated` - last updated

### example

```json
{
    "author_user_id": 123,
    "receiver": "myreceiver",
    "authorized_accounts": ["myadmin"],
    "status": "ok",
    "created_at": "2020-12-05T00:00:00",
    "last_updated": "2020-12-06T00:00:00"
}
```

## TABLE `grants`

### params

- `{uint64_t} grant_id` - (primary key) grant ID
- `{name} grant_name` - (❗️**RESTRICTED**) grant name (used in memo to receive funds, must be unique)
- `{set<uint64_t>} rounds` - matching rounds participation
- `<...base...>` - extends TABLE `*base*`

### example

```json
{
    "grant_id": 345,
    "grant_name": "mygrant",
    "rounds": [1],
    <...base...>
}
```
## TABLE `bounties`

### params

- `{uint64_t} bounty_id` - (primary key) bounty ID
- `{name} bounty_name` - (❗️**RESTRICTED**) bounty name (used in memo to receive funds, must be unique)
- `<...base...>` - extends TABLE `*base*`

### example

```json
{
    "bounty_id": 345,
    "bounty_name": "mybounty",
    <...base...>
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
