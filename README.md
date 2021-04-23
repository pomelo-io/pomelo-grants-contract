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
- [ACTION `setuser`](#action-setuser)
- [ACTION `deluser`](#action-deluser)

## TABLE `users`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byaccount`    | 2                | i64        |
| `byregion`     | 3                | i64        |
| `byupdated`    | 4                | i64        |

### params

- `{uint64_t} user_id` - (primary key) user ID
- `{name} eos_account` - (secondary key) EOS account name
- `{name} region` - member region
- `{map<name, bool>} social` - social enabled
- `{time_point_sec} last_updated` - last updated

### example

```json
{
    "user_id": 123,
    "eos_account": "myaccount",
    "region": "ca",
    "social": [{"key": "github", "value": true}],
    "last_updated": "2020-12-06T00:00:00"
}
```

## TABLE `grants`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byname`       | 2                | i64        |
| `byreceiver`   | 3                | i64        |
| `byupdated`    | 4                | i64        |

### params

- `{uint64_t} grant_id` - (primary key) grant ID
- `{name} grant_name` - (secondary key) grant name (used in memo to receive funds, must be unique)
- `{uint64_t} receiver_user_id` - receiver of funds (user ID)
- `{set<name>} requirements` - specific funding requirements
- `{time_point_sec} last_updated` - last updated

### example

```json
{
    "grant_id": 345,
    "grant_name": "mygrant",
    "receiver_user_id": 123,
    "requirements": ["user"],
    "last_updated": "2020-12-06T00:00:00"
}
```

## TABLE `bounties`

### multi-indexes

| `param`        | `index_position` | `key_type` |
|--------------- |------------------|------------|
| `byname`       | 2                | i64        |
| `byreceiver`   | 3                | i64        |
| `byupdated`    | 4                | i64        |

### params

- `{uint64_t} bounty_id` - (primary key) bounty ID
- `{name} bounty_name` - (secondary key) bounty name (used in memo to receive funds, must be unique)
- `{uint64_t} receiver_user_id` - receiver of funds (user ID)
- `{set<name>} requirements` - specific funding requirements
- `{time_point_sec} last_updated` - last updated

### example

```json
{
    "bounty_id": 345,
    "bounty_name": "mybounty",
    "receiver_user_id": 123,
    "requirements": ["user"],
    "last_updated": "2020-12-06T00:00:00"
}
```


## ACTION `setuser`

- **authority**: `get_self()`

### params

- `{uint64_t} user_id` - user ID
- `{name} eos_account` - EOS account name
- `{name} region` - member region
- `{map<name, bool>} social` - social enabled

### Example

```bash
$ cleos push action battle.gems setuser '[123, "myaccount", "ca", [{"key": "github", "value": true}]]' -p battle.gems
```

## ACTION `deluser`

- **authority**: `get_self()`

### params

- `{uint64_t} user_id` - user ID

### Example

```bash
$ cleos push action battle.gems deluser '[123]' -p battle.gems
```
