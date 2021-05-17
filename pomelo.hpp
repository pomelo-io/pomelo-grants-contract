#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("pomelo")]] pomelo : public eosio::contract {
public:

    /**
     * ## TABLE `users`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{uint64_t} user_id` - (primary key) user ID
     * - `{name} eos_account = ""` - EOS account name
     * - `{name} region` - user region (ex: `ca`)
     * - `{map<name, bool>} social = {}` - user status (ex: pending/ok/disabled)
     * - `{name} status = "pending"_n` - user status (ex: pending/ok/disabled)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} deleted_at` - deleted at time
     *
     * *Multi-indexes*
     * - `{uint64_t} byaccount` - by `eos_account`
     * - `{uint64_t} bystatus` - by `status`
     * - `{uint64_t} byupdated` - by `updated_at`
     *
     * ### example
     *
     * ```json
     * {
     *   "user_id": 123,
     *   "eos_account": "myaccount",
     *   "region": "ca",
     *   "social": [{"key": "github", "value": true}],
     *   "status": "ok",
     *   "created_at": "2020-12-06T00:00:00",
     *   "updated_at": "2020-12-06T00:00:00",
     *   "deleted_at": "1970-01-01T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("users")]] users_row {
        uint64_t                user_id;
        name                    eos_account;
        name                    region;
        map<name, bool>         social;
        name                    status = "pending"_n;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          deleted_at;

        uint64_t primary_key() const { return user_id; };
        uint64_t byaccount() const { return eos_account.value; };
        uint64_t bystatus() const { return status.value; };
        uint64_t byupdated() const { return updated_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "users"_n, users_row,
        indexed_by< "byaccount"_n, const_mem_fun<users_row, uint64_t, &users_row::byaccount> >,
        indexed_by< "bystatus"_n, const_mem_fun<users_row, uint64_t, &users_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<users_row, uint64_t, &users_row::byupdated> >
    > users_table;


    /**
     * ## TABLE `grants` & `bounties`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{uint64_t} id` - (primary key) project ID
     * - `{name} name` - (❗️IMMUTABLE) project name (used in memo to receive funds, must be unique)
     * - `{name} category` - (❗️IMMUTABLE) project category (ex: grant/bounty)
     * - `{name} author_user_id - (❗️IMMUTABLE) author (Pomelo User Id)
     * - `{set<name>} authorized_user_ids = {}` - authorized admins (Pomelo User Id)
     * - `{name} funding_account - ""` - funding account (EOS account)
     * - `{set<extended_symbol>} accepted_tokens = ["4,EOS@eosio.token"]]` - accepted tokens (ex: EOS/USDT)
     * - `{name} status = "pending" - status (pending/ok/disabled)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} deleted_at` - deleted at time
     *
     * *Multi-indexes*
     * - `{uint64_t} byname` - by `name`
     * - `{uint64_t} byauthor` - by `author_user_id`
     * - `{uint64_t} bystatus` - by `status`
     * - `{uint64_t} byupdated` - by `updated_at`
     *
     * ### example
     *
     * ```json
        * {
        *   "id": 345,
        *   "name": "mygrant",
        *   "category": "grant",
        *   "author_user_id": 123,
        *   "authorized_user_ids": [123],
        *   "funding_account": "myreceiver",
        *   "accepted_tokens": [{"contract": "eosio.token", "symbol": "4,EOS"}],
        *   "status": "ok",
        *   "created_at": "2020-12-06T00:00:00",
        *   "updated_at": "2020-12-06T00:00:00",
        *   "deleted_at": "1970-01-01T00:00:00"
        * }
     * ```
     */
    struct [[eosio::table]] projects_row {
        uint64_t                id;
        name                    project_name;
        name                    category;
        name                    author_user_id;
        set<name>               authorized_user_ids;
        name                    funding_account;
        set<extended_symbol>    accepted_tokens = { extended_symbol {symbol{"EOS", 4}, "eosio.token"_n}};
        name                    status = "pending"_n;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          deleted_at;

        uint64_t primary_key() const { return id; }
        uint64_t byname() const { return project_name.value; };
        uint64_t byauthor() const { return author_user_id.value; };
        uint64_t bystatus() const { return status.value; };
        uint64_t byupdated() const { return updated_at.sec_since_epoch(); };
    };

    typedef eosio::multi_index< "grants"_n, projects_row,
        indexed_by< "byname"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byname> >,
        indexed_by< "byauthor"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byauthor> >,
        indexed_by< "bystatus"_n, const_mem_fun<projects_row, uint64_t, &projects_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byupdated> >
    > grants_table;

    typedef eosio::multi_index< "bounties"_n, projects_row,
        indexed_by< "byname"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byname> >,
        indexed_by< "byauthor"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byauthor> >,
        indexed_by< "bystatus"_n, const_mem_fun<projects_row, uint64_t, &projects_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byupdated> >
    > bounties_table;


    /**
     * ## TABLE `rounds`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{uint64_t} round` - (primary key) matching rounds
     * - `{set<name>} grant_ids` - grants IDs participating
     * - `{set<name>} user_ids` - user IDs participating
     * - `{vector<extended_asset>} accepted_tokens` - minimum accepted tokens
     * - `{time_point_sec} start_at` - start at time
     * - `{time_point_sec} end_at` - end at time
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} deleted_at` - deleted at time
     *
     * ### example
     *
     * ```json
     * {
     *   "round": 1,
     *   "grant_ids": [345],
     *   "user_ids": [123],
     *   "accepted_tokens": [{"contract": "eosio.token", "quantity": "1.0000 EOS"}],
     *   "start_at": "2020-12-06T00:00:00",
     *   "end_at": "2020-12-12T00:00:00",
     *   "created_at": "2020-12-06T00:00:00",
     *   "updated_at": "2020-12-06T00:00:00",
     *   "deleted_at": "1970-01-01T00:00:00"
     *   }
     * ```
     */
    struct [[eosio::table("rounds")]] rounds_row {
        uint64_t                round;
        set<name>               grant_ids;
        set<name>               user_ids;
        vector<extended_asset>  accepted_tokens;
        time_point_sec          start_at;
        time_point_sec          end_at;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          deleted_at;

        uint64_t primary_key() const { return round; };
    };
    typedef eosio::multi_index< "rounds"_n, rounds_row > rounds_table;


    /**
     * ## TABLE `donate.user`
     *
     * *scope*: `round` (uint64_t)
     *
     * - `{uint64_t} id` - (primary key) donate ID
     * - `{name} eos_account` - EOS account name
     * - `{uint64_t} round` - participating round
     * - `{uint64_t} grant_id` - grant ID
     * - `{extended_asset} amount` - amount of tokens donated
     * - `{double} value` - USD valuation at time of received
     * - `{checksum256} trx_id` - transaction ID
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} deleted_at` - deleted at time
     *
     * *Multi-indexes*
     * - `{uint64_t} byaccount` - by `account`
     * - `{uint64_t} bygrant` - by `grant_id`
     * - `{uint64_t} byvalue` - by `value`
     * - `{uint64_t} byupdated` - by `updated_at`
     *
     * ### example
     *
     * ```json
     * {
     *   "id": 1001,
     *   "eos_account": "myaccount",
     *   "round": 1,
     *   "grant_id": 123,
     *   "amount": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
     *   "value": 100,
     *   "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
     *   "created_at": "2020-12-06T00:00:00",
     *   "updated_at": "2020-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("donate.user")]] donate_user_row {
        uint64_t                id;
        name                    eos_account;
        uint64_t                round;
        uint64_t                grant_id;
        extended_asset          amount;
        double                  value;
        checksum256             trx_id;
        time_point_sec          created_at;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return id; };
        uint64_t byaccount() const { return eos_account.value; };
        uint64_t bygrant() const { return grant_id; };
        uint64_t byvalue() const { return static_cast<uint64_t> (value * 100); };
        uint64_t byupdated() const { return updated_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "donate.user"_n, donate_user_row,
        indexed_by< "byaccount"_n, const_mem_fun<donate_user_row, uint64_t, &donate_user_row::byaccount> >,
        indexed_by< "bygrant"_n, const_mem_fun<donate_user_row, uint64_t, &donate_user_row::bygrant> >,
        indexed_by< "byvalue"_n, const_mem_fun<donate_user_row, uint64_t, &donate_user_row::byvalue> >,
        indexed_by< "byupdated"_n, const_mem_fun<donate_user_row, uint64_t, &donate_user_row::byupdated> >
     > donate_user_table;

    /**
     * ## TABLE `donate.grant`
     *
     * *scope*: `round` (uint64_t)
     *
     * - `{uint64_t} grant_id` - (primary key) grant ID
     * - `{map<name, double>} user_value` - user value contributions
     * - `{map<name, double>} user_match` - user match contributions
     * - `{map<name, double>} user_sqrt` - user sqrt contributions (quadratic funding metric)
     * - `{uint64_t} total_users` - total number of users
     * - `{double} sum_value` - sum of all user value contributions
     * - `{double} sum_match` - sum of all user match contributions
     * - `{double} sum_sqrt` - sum of square root of contributions (quadratic funding metric)
     * - `{double} square` - total square of the square roots (quadratic funding metric)
     * - `{time_point_sec} updated_at` - updated at time
     *
     * *Multi-indexes*
     * - `{uint64_t} bygrant` - by `grant_id`
     * - `{uint64_t} byusers` - by `total_users`
     * - `{uint64_t} byvalue` - by `value`
     * - `{uint64_t} bymatch` - by `sum_match`
     *
     * ### example
     *
     * ```json
     * {
     *   "grant_id": 123,
     *   "user_value": [{ "key": "account1", "value": 100.0 }, { "key": "account2", "value": 50.0 }],
     *   "user_match": [{ "key": "account1", "value": 225.0 }, { "key": "account2", "value": 100.0 }],
     *   "user_sqrt":  [{ "key": "account1", "value": 15.0 }, { "key": "account2", "value": 10.0 }],
     *   "total_users": 2,
     *   "sum_value": 150.0,
     *   "sum_match": 325.0,
     *   "sum_sqrt": 25.0,
     *   "square": 625.0,
     *   "updated_at": "2020-12-06T00:00:00"
     * }
     * ```
     */

    struct [[eosio::table("donate.grant")]] donate_grant_row {
        uint64_t                grant_id;
        map<name, double>       user_value;
        map<name, double>       user_match;
        map<name, double>       user_sqrt;
        uint64_t                total_users;
        double                  sum_value;
        double                  sum_match;
        double                  sum_sqrt;
        double                  square;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return grant_id; };
        uint64_t byusers() const { return total_users; };
        uint64_t byvalue() const { return static_cast<uint64_t> (sum_value * 100); };
        uint64_t bymatch() const { return static_cast<uint64_t> (sum_match * 100); };
    };
    typedef eosio::multi_index< "donate.grant"_n, donate_grant_row,
        indexed_by< "byusers"_n, const_mem_fun<donate_grant_row, uint64_t, &donate_grant_row::byusers> >,
        indexed_by< "byvalue"_n, const_mem_fun<donate_grant_row, uint64_t, &donate_grant_row::byvalue> >,
        indexed_by< "bymatch"_n, const_mem_fun<donate_grant_row, uint64_t, &donate_grant_row::bymatch> >
     > donate_grant_table;

};