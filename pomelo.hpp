#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;
using namespace eosn;

namespace eosn {

class [[eosio::contract("pomelo")]] pomelo : public eosio::contract {
public:

    pomelo(name rec, name code, datastream<const char*> ds)
      : eosio::contract(rec, code, ds)
      , config(get_self(), get_self().value)
    {};

    /**
     * ## TABLE `config`
     *
     * - `{name} status` - contract status ("ok", "testing", "maintenance")
     *
     * ### example
     *
     * ```json
     * {
     *   "status": "ok",
     *   "value_symbol": "4,USDT@tethertether",
     *   "login_contract": "login.eosn"
     * }
     * ```
     */
    struct [[eosio::table("config")]] config_row {
        name                status = "testing"_n;
        extended_symbol     value_symbol = { symbol {"USDT", 4}, "tethertether"_n };
        name                login_contract = "login.eosn"_n;
    };
    typedef eosio::singleton< "config"_n, config_row > config_table;

    /**
     * ## TABLE `state`
     *
     * - `{int64_t} round` - funding round
     *
     * ### example
     *
     * ```json
     * {
     *   "round": 1,
     * }
     * ```
     */
    struct [[eosio::table("state")]] state_row {
        uint64_t             round_id = 0;
    };
    typedef eosio::singleton< "state"_n, state_row > state_table;

    /**
     * ## TABLE `grants` & `bounties`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{name} id` - (primary key) project name ID (used in memo to receive funds, must be unique)
     * - `{name} type` - (❗️IMMUTABLE) project type (ex: `grant/bounty`)
     * - `{name} author_user_id - (❗️IMMUTABLE) author (Pomelo User Id)
     * - `{set<name>} authorized_user_ids = {}` - authorized admins (Pomelo User Id)
     * - `{name} funding_account - ""` - funding account (EOS account)
     * - `{set<extended_symbol>} accepted_tokens = ["4,EOS@eosio.token"]]` - accepted tokens (ex: `EOS/USDT`)
     * - `{name} status = "pending" - status (`pending/ok/disabled`)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} deleted_at` - deleted at time
     *
     * *Multi-indexes*
     * - `{uint64_t} byname` - by `id`
     * - `{uint64_t} byauthor` - by `author_user_id`
     * - `{uint64_t} bystatus` - by `status`
     * - `{uint64_t} byupdated` - by `updated_at`
     *
     * ### example
     *
     * ```json
        * {
        *   "id": "mygrant",
        *   "type": "grant",
        *   "author_user_id": "user1.eosn",
        *   "authorized_user_ids": ["user1.eosn"],
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
        name                    id;
        name                    type;
        name                    author_user_id;
        set<name>               authorized_user_ids;
        name                    funding_account;
        set<extended_symbol>    accepted_tokens = { extended_symbol {symbol{"EOS", 4}, "eosio.token"_n}};
        name                    status = "pending"_n;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          deleted_at;

        uint64_t primary_key() const { return id.value; }
        uint64_t byauthor() const { return author_user_id.value; };
        uint64_t bystatus() const { return status.value; };
        uint64_t byupdated() const { return updated_at.sec_since_epoch(); };
    };

    typedef eosio::multi_index< "grants"_n, projects_row,
        indexed_by< "byauthor"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byauthor> >,
        indexed_by< "bystatus"_n, const_mem_fun<projects_row, uint64_t, &projects_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byupdated> >
    > grants_table;

    typedef eosio::multi_index< "bounties"_n, projects_row,
        indexed_by< "byauthor"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byauthor> >,
        indexed_by< "bystatus"_n, const_mem_fun<projects_row, uint64_t, &projects_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<projects_row, uint64_t, &projects_row::byupdated> >
    > bounties_table;


    /**
     * ## TABLE `transfer`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{uint64_t} transfer_id` - (primary key) token transfer ID
     * - `{name} user_id` - Pomelo user account ID
     * - `{uint64_t} round_id` - participating round ID
     * - `{name} grant_id` - grant ID
     * - `{name} eos_account` - EOS account sending transfer
     * - `{extended_asset} amount - amount of tokens donated
     * - `{double} value` - USD valuation at time of received
     * - `{checksum256} trx_id` - transaction ID
     * - `{string} memo` - transfer memo
     * - `{time_point_sec} created_at` - created at time
     *
     * *Multi-indexes*
     * - `{uint64_t} byuser` - by `user_id`
     * - `{uint64_t} byround` - by `round_id`
     * - `{uint64_t} bygrant` - by `grant_id`
     * - `{uint64_t} byvalue` - by `value`
     * - `{uint64_t} bycreated` - by `bycreated_at`
     *
     * ### example
     *
     * ```json
        * {
        *   "transfer_id": 10001,
        *   "user_id": "user1.eosn",
        *   "round": 1,
        *   "grant_id": "grant1",
        *   "eos_account": "myaccount",
        *   "amount": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
        *   "value": 100.0,
        *   "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
        *   "memo": "grant:grant1",
        *   "created_at": "2020-12-06T00:00:00"
        * }
     * ```
     */

    struct [[eosio::table]] transfers_row {
        uint64_t                transfer_id;
        name                    user_id;
        uint64_t                round_id;
        name                    project_id;
        name                    eos_account;
        extended_asset          amount;
        double                  value;
        checksum256             trx_id;
        string                  memo;
        time_point_sec          created_at;

        uint64_t primary_key() const { return transfer_id; }
        uint64_t byuser() const { return user_id.value; };
        uint64_t byround() const { return round_id; };
        uint64_t byproject() const { return project_id.value; };
        uint64_t byvalue() const { return static_cast<uint64_t> (value * 100); };
        uint64_t bycreated() const { return created_at.sec_since_epoch(); };
    };

    typedef eosio::multi_index< "transfers"_n, transfers_row,
        indexed_by< "byuser"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byuser> >,
        indexed_by< "byround"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byround> >,
        indexed_by< "byproject"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byproject> >,
        indexed_by< "byvalue"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byvalue> >,
        indexed_by< "bycreated"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::bycreated> >
    > transfers_table;

    /**
     * ## TABLE `match.grant`
     *
     * *scope*: `round_id`
     *
     * - `{name} grant_id` - (primary key) grant ID
     * - `{uint64_t} round_id` - round ID
     * - `{map<name, double>} user_value` - user value contributions
     * - `{map<name, double>} user_multiplier` - user boost multiplier
     * - `{map<name, double>} user_boost` - user contributions boosts
     * - `{map<name, double>} user_sqrt` - user sqrt contributions (quadratic funding metric)
     * - `{uint64_t} total_users` - total number of users
     * - `{double} sum_value` - sum of all user value contributions
     * - `{double} sum_boost` - sum of all user contribution boosts
     * - `{double} sum_sqrt` - sum of square root of contributions (quadratic funding metric)
     * - `{double} square` - total square of the square roots (quadratic funding metric)
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *   "round_id": 1,
     *   "grant_id": "grant_id",
     *   "user_value": [{ "key": "myaccount", "value": 100.0 }, { "key": "toaccount", "value": 50.0 }],
     *   "user_multiplier": [{ "key": "myaccount", "value": 2.25 }, { "key": "toaccount", "value": 2.0 }],
     *   "user_boost": [{ "key": "myaccount", "value": 225.0 }, { "key": "toaccount", "value": 100.0 }],
     *   "user_sqrt":  [{ "key": "myaccount", "value": 15.0 }, { "key": "toaccount", "value": 10.0 }],
     *   "total_users": 2,
     *   "sum_value": 150.0,
     *   "sum_boost": 325.0,
     *   "sum_sqrt": 25.0,
     *   "square": 625.0,
     *   "updated_at": "2020-12-06T00:00:00"
     *   }
     * ```
     */

    struct [[eosio::table("match.grant")]] match_grant_row {
        name                    grant_id;
        uint64_t                round_id;
        map<name, double>       user_value;
        map<name, double>       user_multiplier;
        map<name, double>       user_boost;
        map<name, double>       user_sqrt;
        uint64_t                total_users;
        double                  sum_value;
        double                  sum_boost;
        double                  sum_sqrt;
        double                  square;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return grant_id.value; };
    };
    typedef eosio::multi_index< "match.grant"_n, match_grant_row > match_grant_table;

    /**
     * ## TABLE `rounds`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{uint64_t} round` - (primary key) matching round
     * - `{set<name>} grant_ids` - grants IDs participating
     * - `{set<name>} user_ids` - user IDs participating
     * - `{vector<extended_asset>} accepted_tokens` - accepted tokens
     * - `{double} sum_value` - total value donated this round
     * - `{double} sum_boost` - total boost received this round
     * - `{double} sum_square` - square of total sqrt sum
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
     *   "grant_ids": ["grant1"],
     *   "user_ids": ["user1.eosn"],
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
        double                  sum_value;
        double                  sum_boost;
        double                  sum_square;
        time_point_sec          start_at;
        time_point_sec          end_at;
        time_point_sec          created_at;
        time_point_sec          updated_at;
        time_point_sec          deleted_at;

        uint64_t primary_key() const { return round; };
    };
    typedef eosio::multi_index< "rounds"_n, rounds_row > rounds_table;

    /**
     * ## ACTION `setstatus`
     *
     * Set contract status
     *
     * ### params
     *
     * - `{name} status` - status `testing/ok/maintenance`
     *
     */

    [[eosio::action]]
    void setstatus( const name status );

    /**
     * ## ACTION `setvaluesym`
     *
     * Set value extended symbol
     *
     * ### params
     *
     * - `{extended_symbol} value_symbol` - value symbol, i.e 4,USDT@tethertether
     *
     */

    [[eosio::action]]
    void setvaluesym( const extended_symbol value_symbol );

    /**
     * ## ACTION `setgrant`
     *
     * Create/update grant project without modifying project status
     *
     * ### params
     *
     * - `{name} id` - project id
     * - `{name} author_id` - author user id
     * - `{set<name>} authorized_ids` - authorized user ids
     * - `{name} funding_account` - account to forward donations to
     * - `{set<extended_symbol>} accepted_tokens` - accepted tokens
     */

    [[eosio::action]]
    void setgrant( const name id, const name author_id, const set<name> authorized_ids, const name funding_account, const set<extended_symbol> accepted_tokens );

    /**
     * ## ACTION `setbounty`
     *
     * Create/update bounty project without modifying project status
     *
     * ### params
     *
     * - `{name} id` - project id
     * - `{name} author_id` - author user id
     * - `{set<name>} authorized_ids` - authorized user ids
     * - `{name} funding_account` - account to forward donations to
     * - `{set<extended_symbol>} accepted_tokens` - accepted tokens
     */

    [[eosio::action]]
    void setbounty( const name id, const name author_id, const set<name> authorized_ids, const name funding_account, const set<extended_symbol> accepted_tokens );


    /**
     * ## ACTION `setprjstatus`
     *
     * Update project status
     *
     * ### params
     *
     * - `{name} status` - new project status
     */

    [[eosio::action]]
    void setprjstatus( const name project_id, const name status );

    /**
     * ## ACTION `setround`
     *
     * Create/update round
     *
     * ### params
     *
     * - `{uint64_t} round_id` - round id
     * - `{time_point_sec} start_at` - round start time
     * - `{time_point_sec} end_at` - round end time
     */

    [[eosio::action]]
    void setround( const uint64_t round_id, const time_point_sec start_at, const time_point_sec end_at );

    /**
     * ## ACTION `joinround`
     *
     * Add project to funding round
     *
     * ### params
     *
     * - `{name} project_id` - project id
     * - `{uint64_t} round_id` - round id
     */

    [[eosio::action]]
    void joinround( const name project_id, const uint64_t round_id );

    /**
     * ## ACTION `startround`
     *
     * Start round by making sure round is defined and changing the state table
     *
     * ### params
     *
     * - `{uint64_t} round_id` - round id, 0 if no round is ongoing
     */

    [[eosio::action]]
    void startround( const uint64_t round_id );

    /**
     * ## TRANSFER NOTIFY HANDLER `on_transfer`
     *
     * Process incoming transfer
     *
     * ### params
     *
     * - `{name} from` - from EOS account (donation sender)
     * - `{name} to` - to EOS account (process only incoming)
     * - `{asset} quantity` - quantity received
     * - `{string} memo` - transfer memo, i.e. "grant:myproject"
     *
     */
    [[eosio::on_notify("*::transfer")]]
    void on_transfer( const name from, const name to, const asset quantity, const string memo );

    /**
     * ## SOCIAL NOTIFY HANDLER `on_social`
     *
     * Update boosts and matching for user when their socials change
     *
     * ### params
     *
     * - `{name} user_id` - Pomelo user_id
     * - `{set<name>} socials` - socials for that user
     *
     */
    [[eosio::on_notify("*::social")]]
    void on_social( const name user_id, const set<name> socials );

private:

    config_table config;

    double get_value(const extended_asset ext_quantity);

    name get_user_id( const name user );

    bool is_user( const name user_id );

    double get_user_boost_mutliplier( const name user_id );

    uint64_t get_current_round();

    template <typename T>
    void fund_project(const T& table, const name project, const extended_asset ext_quantity, const name user, const string& memo);

    void fund_grant(const name grant_id, const extended_asset ext_quantity, const name user_id, const double value);

    template <typename T>
    void set_project(T& table, const name type, const name id, const name author_id, const set<name> authorized_ids, const name funding_account, const set<extended_symbol> accepted_tokens );

    void log_transfer(const name project_id, const name user, const extended_asset ext_quantity, const double value, const string& memo);

};
}