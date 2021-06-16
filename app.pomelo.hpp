#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

// static values

static constexpr extended_symbol VALUE_SYM = { symbol {"EOS", 4}, "eosio.token"_n };
static constexpr name LOGIN_CONTRACT = "login.eosn"_n;
static constexpr name FEE_ACCOUNT = "fee.pomelo"_n;
static set<name> STATUS_TYPES = set<name>{"ok"_n, "testing"_n, "pending"_n, "disabled"_n};

static string ERROR_INVALID_MEMO = "invalid transfer memo (ex: \"grant:mygrant\" or \"bounty:mybounty\")";

class [[eosio::contract("app.pomelo")]] pomelo : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `globals`
     *
     * ### params
     *
     * - `{name} key` - (primary key) key
     * - `{uint64_t} value` - value
     *
     * ### example
     *
     * ```json
     * [
     *   { "key": "roundid", "value": 1 },
     *   { "key": "status", "value": 1 },
     *   { "key": "systemfee", "value": 500 },
     *   { "key": "minamount", "value": 1000 }
     * ]
     * ```
     */
    struct [[eosio::table("globals")]] globals_row {
        name                key;
        uint64_t            value;

        uint64_t primary_key() const { return key.value; }
    };
    typedef eosio::multi_index< "globals"_n, globals_row> globals_table;

    /**
     * ## TABLE `grants` & `bounties`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{name} id` - (primary key) project name ID (used in memo to receive funds, must be unique)
     * - `{name} type` - (❗️IMMUTABLE) project type (ex: `grant/bounty`)
     * - `{name} author_user_id - (❗️IMMUTABLE) author (Pomelo User Id)
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
     *   "author_user_id": "123.eosn",
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
     * - **scope**: `get_self() {name}`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `byfrom`       | 2                | i64        |
     * | `byuser`       | 3                | i64        |
     * | `byround`      | 4                | i64        |
     * | `bygrant`      | 5                | i64        |
     * | `byvalue`      | 6                | i64        |
     * | `bycreated`    | 7                | i64        |
     *
     * ### params
     *
     * - `{uint64_t} transfer_id` - (primary key) token transfer ID
     * - `{name} from` - EOS account sender
     * - `{name} to` - EOS account receiver
     * - `{extended_asset} ext_quantity` - amount of tokens transfered
     * - `{asset} fee` - fee charged and sent to `FEE_ACCOUNT`
     * - `{string} memo` - transfer memo
     * - `{name} user_id` - Pomelo user account ID
     * - `{uint64_t} round_id` - participating round ID
     * - `{name} project_type` - project type ("grant" / "bounty")
     * - `{name} project_id` - project ID
     * - `{double} value` - valuation at time of received
     * - `{checksum256} trx_id` - transaction ID
     * - `{time_point_sec} created_at` - created at time
     *
     * ### example
     *
     * ```json
     * {
     *     "transfer_id": 10001,
     *     "from": "myaccount",
     *     "to": "pomelo",
     *     "ext_quantity": {"contract": "eosio.token", "quantity": "15.0000 EOS"},
     *     "fee": "1.0000 EOS",
     *     "memo": "grant:grant1",
     *     "user_id": "user1.eosn",
     *     "round": 1,
     *     "project_type": "grant",
     *     "project_id": "grant1",
     *     "value": 100.0,
     *     "trx_id": "3bf31f6c32a8663bf3fdb0993a2bf3784d181dc879545603dca2046f05e0c9e1",
     *     "created_at": "2020-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table]] transfers_row {
        uint64_t                transfer_id;
        name                    from;
        name                    to;
        extended_asset          ext_quantity;
        asset                   fee;
        string                  memo;
        name                    user_id;
        uint64_t                round_id;
        name                    project_type;
        name                    project_id;
        double                  value;
        checksum256             trx_id;
        time_point_sec          created_at;

        uint64_t primary_key() const { return transfer_id; };
        uint64_t byfrom() const { return from.value; };
        uint64_t byuser() const { return user_id.value; };
        uint64_t byround() const { return round_id; };
        uint64_t byproject() const { return project_id.value; };
        uint64_t byvalue() const { return static_cast<uint64_t> ( value * VALUE_SYM.get_symbol().precision() ); };
        uint64_t bycreated() const { return created_at.sec_since_epoch(); };
    };

    typedef eosio::multi_index< "transfers"_n, transfers_row,
        indexed_by< "byfrom"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byfrom> >,
        indexed_by< "byuser"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byuser> >,
        indexed_by< "byround"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byround> >,
        indexed_by< "byproject"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byproject> >,
        indexed_by< "byvalue"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::byvalue> >,
        indexed_by< "bycreated"_n, const_mem_fun<transfers_row, uint64_t, &transfers_row::bycreated> >
    > transfers_table;

    /**
     * ## TABLE `match`
     *
     * *scope*: `round_id`
     *
     * - `{name} grant_id` - (primary key) grant ID
     * - `{uint64_t} round_id` - round ID
     * - `{uint64_t} total_users` - total number of users
     * - `{double} sum_value` - sum of all user value contributions
     * - `{double} sum_boost` - sum of all user contribution boosts
     * - `{double} sum_sqrt` - sum of square root of contributions (quadratic matching metric)
     * - `{double} square` - total square of the square roots (quadratic matching metric)
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *   "round_id": 1,
     *   "grant_id": "grant_id",
     *   "total_users": 2,
     *   "sum_value": 150.0,
     *   "sum_boost": 325.0,
     *   "sum_sqrt": 25.0,
     *   "square": 625.0,
     *   "updated_at": "2020-12-06T00:00:00"
     *   }
     * ```
     */
    struct [[eosio::table("match")]] match_row {
        name                    grant_id;
        uint64_t                round_id;
        uint64_t                total_users;
        double                  sum_value;
        double                  sum_boost;
        double                  sum_sqrt;
        double                  square;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return grant_id.value; };
    };
    typedef eosio::multi_index< "match"_n, match_row > match_table;


    /**
     * ## TABLE `users`
     *
     * *scope*: `round_id` (name)
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `bydonated`    | 2                | i64        |
     * | `byboosted`    | 3                | i64        |
     *
     * - `{name} user_id` - (primary key) user_id
     * - `{double} multiplier` - user multiplier this round
     * - `{double} value` - total amount contributed by user in this round
     * - `{double} boost` - total boost amount for user in this round
     * - `{vector<contribution_t>} contributions` - user contributions to projects this round
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *   "user_id": 1,
     *   "multiplier": "0.25",
     *   "contributions": [{ "id": "grant1", "value": 225.0 }, { "id": "grant2", "value": 100.0 }],
     *   "updated_at": "2020-12-06T00:00:00",
     * }
     * ```
     */
    struct contribution_t {
        name    id;
        double  value;
    };

    struct [[eosio::table("users")]] users_row {
        name                    user_id;
        double                  multiplier;
        double                  value;
        double                  boost;
        vector<contribution_t>  contributions;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return user_id.value; };
        uint64_t bydonated() const { return static_cast<uint64_t> (value * 100); };
        uint64_t byboosted() const { return static_cast<uint64_t> ((value + boost) * 100); };
    };
    typedef eosio::multi_index< "users"_n, users_row,
        indexed_by< "bydonated"_n, const_mem_fun<users_row, uint64_t, &users_row::bydonated> >,
        indexed_by< "byboosted"_n, const_mem_fun<users_row, uint64_t, &users_row::byboosted> >
    > users_table;

    /**
     * ## TABLE `rounds`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{uint64_t} round` - (primary key) matching round
     * - `{vector<name>} grant_ids` - grants IDs participating
     * - `{vector<name>} user_ids` - user IDs participating
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
     * }
     * ```
     */
    struct [[eosio::table("rounds")]] rounds_row {
        uint64_t                round;
        vector<name>            grant_ids;
        vector<name>            user_ids;
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
     * ## ACTION `init`
     *
     * Init contract config with default parameters (status=2, roundid=0, minamount=1000, systemfee=0)
     *
     * ### params
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo init '[]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void init( );

    /**
     * ## ACTION `setconfig`
     *
     * Set contract config key/value
     * - `status` - contract status (0=testing, 1=ok, 2=maintenance)
     * - `roundid` - ongoing round (0=not active)
     * - `minamount` - minimum grant donation value amount with precision = 4 (1234=0.1234 EOS)
     * - `systemfee` - donation fee (500=5%)
     *
     * ### params
     *
     * - `{name} key` - config key
     * - `{uint64_t} value` - config value
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo setconfig '["status", 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void setconfig( const name key, const uint64_t value );

    /**
     * ## ACTION `setproject`
     *
     * Create/update grant/bounty project without modifying project status
     *
     * ### params
     *
     * - `{name} author_id` - author user id
     * - `{name} project_type` - project type (grant/bounty)
     * - `{name} project_id` - project ID
     * - `{name} funding_account` - account to forward donations to
     * - `{set<extended_symbol>} accepted_tokens` - accepted tokens
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action app.pomelo setproject '["123.eosn", "grant", "mygrant", "project2fund", [["4,USDT", "tethertether"]]]' -p app.pomelo -p 123.eosn
     * ```
     */
    [[eosio::action]]
    void setproject( const name author_id, const name project_type, const name project_id, const name funding_account, const set<extended_symbol> accepted_tokens );

    /**
     * ## ACTION `enable`
     *
     * Enable/disable grant or bounty
     *
     * ### params
     *
     * - `{name} project_type` - project type (grant/bounty)
     * - `{name} project_id` - project ID
     * - `{name} status` - status `pending/ok/disabled'
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo enable '["grant", "grant1", 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void enable( const name project_type, const name project_id, const name status );

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
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo setround '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00"]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void setround( const uint64_t round_id, const time_point_sec start_at, const time_point_sec end_at );

    /**
     * ## ACTION `joinround`
     *
     * Add grant to matching round
     *
     * ### params
     *
     * - `{name} grant_id` - grant ID
     * - `{uint64_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo joinround '["grant1", 1]' -p app.pomelo -p 123.eosn
     * ```
     */
    [[eosio::action]]
    void joinround( const name grant_id, const uint64_t round_id );

    /**
     * ## ACTION `unjoinround`
     *
     * Add grant to matching round
     *
     * ### params
     *
     * - `{name} grant_id` - grant ID
     * - `{uint64_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo unjoinround '["grant1", 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void unjoinround( const name grant_id, const uint64_t round_id );

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

    /**
     * ## ACTION `cleartable`
     *
     * Clear table
     *
     * ### params
     *
     * - `{name} table_name` - table name, i.e. "transfers"
     * - `{uint64_t} max_rows` - max number of rows to clear, if 0 - clear all
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo cleartable '["transfers", 500]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void cleartable( const name table_name, const uint64_t max_rows );

    /**
     * ## ACTION `removeuser`
     *
     * Remove user from all projects at this round and update all matchings
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{uint64_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo removeuser '["user1.eosn", 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void removeuser( const name user_id, const uint64_t round_id );

    /**
     * ## ACTION `collapse`
     *
     * Collapse donations from {user_ids} users into {user_id} in {round_id} and recalculate all matchings
     *
     * ### params
     *
     * - `{set<name>} user_ids` - user IDs to collapse
     * - `{name} user_id` - user ID to collapse into
     * - `{uint64_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo collapse '[["user2.eosn","user3.eosn","user4.eosn"], "user1.eosn", 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void collapse(set<name> user_ids, name user_id, uint64_t round_id);

private:
    // state_table _state;

    // getters
    double calculate_value(const extended_asset ext_quantity);
    name get_user_id( const name user );
    bool is_user( const name user_id );
    double get_user_boost_mutliplier( const name user_id );
    void validate_round( const uint64_t round_id );

    // globals key/value
    void set_key_value( const name key, const uint64_t value );
    uint64_t get_key_value( const name key );
    bool del_key( const name key );

    template <typename T>
    void enable_project( T& table, const name project_id, const name status );

    template <typename T>
    void donate_project(const T& table, const name project_id, const name from, const extended_asset ext_quantity, const string memo );

    void donate_grant(const name grant_id, const extended_asset ext_quantity, const name user_id, const double value);

    template <typename T>
    void set_project(T& table, const name project_type, const name project_id, const name author_id, const name funding_account, const set<extended_symbol> accepted_tokens );

    void save_transfer( const name from, const name to, const extended_asset ext_quantity, const asset fee, const string& memo, const name project_type, const name project_id, const double value );

    int get_index(const vector<name>& vec, name value);
    int get_index(const vector<contribution_t>& vec, name id);

    template <typename T>
    vector<T> remove_element(const vector<T>& vec, name id);

    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );
};
