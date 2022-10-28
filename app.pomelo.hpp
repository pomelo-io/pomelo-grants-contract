#pragma once

#include <eosio/eosio.hpp>
#include <eosio/time.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

using namespace eosio;
using namespace std;

// static values

static constexpr extended_symbol VALUE_SYM = { symbol {"USDT", 4}, "tethertether"_n };
static set<name> STATUS_TYPES = set<name>{"published"_n, "pending"_n, "retired"_n, "banned"_n, "denied"_n};
static constexpr uint32_t DAY = 86400;

static string ERROR_INVALID_MEMO = "invalid transfer memo (ex: \"grant:mygrant\" or \"bounty:mybounty\")";

class [[eosio::contract("app.pomelo")]] pomelo : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `status`
     *
     * - `vector<uint32_t>` counters;   // 0 - total rewdards claimed
     * - `time_point_sec` last_updated;
     *
     * ### example
     *
     * ```json
     * {
     *     "counters": [1234, 12],
     *     "last_updated": "2021-04-12T12:23:42"
     * }
     * ```
     */
    struct [[eosio::table("status")]] status_row {
        vector<uint32_t>        counters;
        time_point_sec          last_updated;
    };
    typedef eosio::singleton< "status"_n, status_row > status_table;

    /**
     * ## TABLE `globals`
     *
     * - `{uint16_t} season_id` - season ID (0 = not active)
     * - `{uint64_t} grant_fee` - grant fee (bips - 1/100 1%)
     * - `{uint64_t} bounty_fee` - bounty fee (bips - 1/100 1%)
     * - `{name} login_contract` - EOSN Login contract
     * - `{name} fee_account` - fee
     *
     * ### example
     *
     * ```json
     * {
     *     "season_id": 1,
     *     "grant_fee": 500,
     *     "bounty_fee": 500,
     *     "login_contractt": "login.eosn",
     *     "fee_account": "fee.pomelo",
     * }
     * ```
     */
    struct [[eosio::table("globals")]] globals_row {
        uint16_t        season_id = 0;
        uint64_t        grant_fee = 500;
        uint64_t        bounty_fee = 500;
        name            login_contract = "login.eosn"_n;
        name            fee_account = "fee.pomelo"_n;
    };
    typedef eosio::singleton< "globals"_n, globals_row > globals_table;

    /**
     * ## TABLE `seasons`
     *
     * ### params
     *
     * - `{uint16_t} season_id` - (primary key) season_id
     * - `{string} description` - season description
     * - `{vector<uint16_t>} round_ids` - round ids participating in this season
     * - `{double} match_value` - total matching pool value for this season
     * - `{time_point_sec} start_at` - start at time
     * - `{time_point_sec} end_at` - end at time
     * - `{time_point_sec} submission_start_at` - submission start time
     * - `{time_point_sec} submission_end_at` - submission end time
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *      "season_id": 1,
     *      "description": "Season #1",
     *      "round_ids": [101, 102, 103],
     *      "match_value": 100000,
     *      "start_at": "2020-12-06T00:00:00",
     *      "end_at": "2020-12-12T00:00:00",
     *      "submission_start_at": "2020-11-06T00:00:00",
     *      "submission_end_at": "2020-12-06T00:00:00",
     *      "created_at": "2020-12-06T00:00:00",
     *      "updated_at": "2020-12-06T00:00:00",
     * }
     * ```
     */
    struct [[eosio::table("seasons")]] seasons_row {
        uint16_t            season_id;
        string              description;
        vector<uint16_t>    round_ids;
        double              match_value;
        time_point_sec      start_at;
        time_point_sec      end_at;
        time_point_sec      submission_start_at;
        time_point_sec      submission_end_at;
        time_point_sec      created_at;
        time_point_sec      updated_at;

        uint64_t primary_key() const { return season_id; }
    };
    typedef eosio::multi_index< "seasons"_n, seasons_row> seasons_table;

    /**
     * ## TABLE `tokens`
     *
     * ### params
     *
     * - `{symbol} sym` - (primary key) symbol
     * - `{name} contract` - token contract
     * - `{uint64_t} min_amount` - min amount required when donating
     * - `{uint64_t} oracle_id` - Defibox Oracle ID
     *
     * ### example
     *
     * ```json
     * {
     *     "sym": "4,EOS",
     *     "contract": "eosio.token",
     *     "min_amount": 10000,
     *     "oracle_id": 1
     * }
     * ```
     */
    struct [[eosio::table("tokens")]] tokens_row {
        symbol              sym;
        name                contract;
        uint64_t            min_amount;
        uint64_t            oracle_id;

        uint64_t primary_key() const { return sym.code().raw(); }
    };
    typedef eosio::multi_index< "tokens"_n, tokens_row> tokens_table;

    /**
     * ## TABLE `grants`
     *
     * *scope*: `get_self()` (name)
     *
     * - `{name} id` - (primary key) project name ID (used in memo to receive funds, must be unique)
     * - `{name} type` - (❗️IMMUTABLE) project type (ex: `grant/bounty`)
     * - `{name} author_user_id - (❗️IMMUTABLE) author (Pomelo User Id)
     * - `{name} funding_account - ""` - funding account (EOS account)
     * - `{set<symbol_code>} accepted_tokens (ex: `["EOS"]`)
     * - `{name} status = "pending" - status (`pending/published/banned/retired/denied`)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
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
     *     "id": "mygrant",
     *     "type": "grant",
     *     "author_user_id": "123.eosn",
     *     "funding_account": "myreceiver",
     *     "accepted_tokens": ["EOS"],
     *     "status": "published",
     *     "created_at": "2020-12-06T00:00:00",
     *     "updated_at": "2020-12-06T00:00:00",
     * }
     * ```
     */
    struct [[eosio::table]] grants_row {
        name                    id;
        name                    type;
        name                    author_user_id;
        name                    funding_account;
        set<symbol_code>        accepted_tokens = { symbol_code{"EOS"} };
        name                    status = "pending"_n;
        time_point_sec          created_at;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return id.value; }
        uint64_t byauthor() const { return author_user_id.value; };
        uint64_t bystatus() const { return status.value; };
        uint64_t byupdated() const { return updated_at.sec_since_epoch(); };
    };

    typedef eosio::multi_index< "grants"_n, grants_row,
        indexed_by< "byauthor"_n, const_mem_fun<grants_row, uint64_t, &grants_row::byauthor> >,
        indexed_by< "bystatus"_n, const_mem_fun<grants_row, uint64_t, &grants_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<grants_row, uint64_t, &grants_row::byupdated> >
    > grants_table;

    /**
     * ## TABLE `transfer`
     *
     * - **scope**: `get_self()`
     *
     * ### params
     *
     * - `{uint64_t} transfer_id` - (primary key) token transfer ID
     * - `{name} from` - EOS account sender
     * - `{name} to` - EOS account receiver
     * - `{extended_asset} ext_quantity` - amount of tokens transfered
     * - `{asset} fee` - fee charged and sent to `global.fee_account`
     * - `{string} memo` - transfer memo
     * - `{name} user_id` - Pomelo user account ID
     * - `{uint16_t} season_id` - participating season ID
     * - `{uint16_t} round_id` - participating round ID
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
     *     "season_id": 1,
     *     "round_id": 101,
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
        uint16_t                season_id;
        uint16_t                round_id;
        name                    project_type;
        name                    project_id;
        double                  value;
        checksum256             trx_id;
        time_point_sec          created_at;

        uint64_t primary_key() const { return transfer_id; };
    };

    typedef eosio::multi_index< "transfers"_n, transfers_row> transfers_table;

    /**
     * ## TABLE `match`
     *
     * *scope*: `{uint16_t} round_id`
     *
     * - `{name} grant_id` - (primary key) grant ID
     * - `{uint16_t} round_id` - round ID
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
     *     "grant_id": "grant_id",
     *     "round_id": 101,
     *     "total_users": 2,
     *     "sum_value": 150.0,
     *     "sum_boost": 325.0,
     *     "sum_sqrt": 25.0,
     *     "square": 625.0,
     *     "updated_at": "2020-12-06T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("match")]] match_row {
        name                    grant_id;
        uint16_t                round_id;
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
     * *scope*: `{uint16_t} round_id`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `bydonated`    | 2                | i64        |
     * | `byboosted`    | 3                | i64        |
     *
     * ### params
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
     * ### params
     *
     * - `{uint16_t} round_id` - (primary key) matching rounds
     * - `{string} description` - grant text description
     * - `{uint16_t} season_id` - season ID
     * - `{set<name>} grant_ids` - grants IDs participating
     * - `{set<name>} user_ids` - user IDs participating
     * - `{vector<extended_asset>} donated_tokens` - donated tokens
     * - `{double} match_value` - estimated value of the matching pool
     * - `{double} sum_value` - total value donated this round
     * - `{double} sum_boost` - total boost received this round
     * - `{double} sum_square` - square of total sqrt sum
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     *
     * ### example
     *
     * ```json
     * {
     *     "round_id": 101,
     *     "description": "Grant Round #1",
     *     "season_id": 1,
     *     "grant_ids": ["grant1"],
     *     "user_ids": ["user1.eosn"],
     *     "donated_tokens": [{"contract": "eosio.token", "quantity": "100.0000 EOS"}],
     *     "match_value": 100000,
     *     "sum_value": 12345,
     *     "sum_boost": 3231,
     *     "sum_square": 423451.1233,
     *     "created_at": "2020-12-06T00:00:00",
     *     "updated_at": "2020-12-06T00:00:00",
     * }
     * ```
     */
    struct [[eosio::table("rounds")]] rounds_row {
        uint16_t                round_id;
        string                  description;
        uint16_t                season_id;
        vector<name>            grant_ids;
        vector<name>            user_ids;
        vector<extended_asset>  donated_tokens;
        double                  match_value;
        double                  sum_value;
        double                  sum_boost;
        double                  sum_square;
        time_point_sec          created_at;
        time_point_sec          updated_at;

        uint64_t primary_key() const { return round_id; };
        uint64_t byseason() const { return season_id; };
    };
    typedef eosio::multi_index< "rounds"_n, rounds_row,
        indexed_by< "byseason"_n, const_mem_fun<rounds_row, uint64_t, &rounds_row::byseason> >
    > rounds_table;

    /**
     * ## ACTION `setconfig`
     *
     * ### params
     *
     * - `{uint16_t} season_id` - season ID (0 = not active)
     * - `{uint64_t} grant_fee` - grant fee (bips - 1/100 1%)
     * - `{uint64_t} bounty_fee` - bounty fee (bips - 1/100 1%)
     * - `{name} login_contract` - EOSN Login contract
     * - `{name} fee_account` - fee account
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo setconfig '[1, 500, 500, "login.eosn", "fee.pomelo"]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void setconfig( const optional<uint16_t> season_id, const optional<uint64_t> grant_fee, const optional<uint64_t> bounty_fee, const optional<name> login_contract, const optional<name> fee_account );

    /**
     * ## ACTION `setseason`
     *
     * Set season parameters. If optional parameter undefined - don't change it. If all parameters undefined - delete
     *
     * ### params
     *
     * - `{uint16_t} season_id` - season ID (should be > 0)
     * - `{optional<time_point_sec>} start_at` - round start time
     * - `{optional<time_point_sec>} end_at` - round end time
     * - `{optional<time_point_sec>} submission_start_at` - round submission start time
     * - `{optional<time_point_sec>} submission_end_at` - round submission end time
     * - `{optional<string>} description` - season description
     * - `{optional<double>} match_value` - match value (for information purposes)
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo setseason '[1, "2021-05-19T20:00:00", "2021-05-25T20:00:00", "2021-05-19T20:00:00", "2021-05-25T20:00:00", "Season 1", 100000]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void setseason( const uint16_t season_id, const optional<time_point_sec> start_at, const optional<time_point_sec> end_at, const optional<time_point_sec> submission_start_at, const optional<time_point_sec> submission_end_at, const optional<string> description, const optional<double> match_value );

    /**
     * ❗️DEPRECATED
     */
    [[eosio::action]]
    void setproject( const name author_id, const name project_type, const name project_id, const name funding_account, const set<symbol_code> accepted_tokens );

    /**
     * ## ACTION `setgrant`
     *
     * Create/update grant - wrapper for setproject for grants
     *
     * ### params
     *
     * - `{name} author_id` - author user id
     * - `{name} project_id` - project ID
     * - `{name} funding_account` - account to forward donations to
     * - `{set<symbol_code>} accepted_tokens` - accepted tokens (ex: `["EOS"]`)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action app.pomelo setgrant '["123.eosn", "mygrant", "project2fund", ["EOS"]]' -p app.pomelo -p 123.eosn
     * ```
     */
    [[eosio::action]]
    void setgrant( const name author_id, const name project_id, const name funding_account, const set<symbol_code> accepted_tokens );

    /**
     * ## ACTION `setstate`
     *
     * Set grant or bounty state
     *
     * ### params
     *
     * - `{name} project_id` - project ID
     * - `{name} status` - status `pending/published/banned/retired/denied'
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo setstate '["grant1", "published"]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void setstate( const name project_id, const name state );

    /**
     * ## ACTION `setround`
     *
     * Create/update round. If a parameter is null - don't change it
     *
     * ### params
     *
     * - `{uint16_t} round_id` - round id
     * - `{uint16_t} season_id` - season id
     * - `{optional<string>} description` - grant description
     * - `{optional<double>} match_value` - total value of the matching pool
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo setround '[101, 1, "Grant Round #1", 100000]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void setround( const uint16_t round_id, const uint16_t season_id, const optional<string> description, const optional<double> match_value );

    /**
     * ## ACTION `joinround`
     *
     * Add grant to matching round
     *
     * ### params
     *
     * - `{name} grant_id` - grant ID
     * - `{uint16_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo joinround '["grant1", 101]' -p app.pomelo -p 123.eosn
     * ```
     */
    [[eosio::action]]
    void joinround( const name grant_id, const uint16_t round_id );

    /**
     * ## ACTION `unjoinround`
     *
     * Remove grant from matching round
     *
     * ### params
     *
     * - `{name} grant_id` - grant ID
     * - `{uint16_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo unjoinround '["grant1", 101]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void unjoinround( const name grant_id, const uint16_t round_id );

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
     * - `{name} social` - socials for that user
     *
     */
    [[eosio::on_notify("*::social")]]
    void on_social( const name user_id, const name social );

    /**
     * ## SOCIAL NOTIFY HANDLER `on_social`
     *
     * Update boosts and matching for user when their socials change
     *
     * ### params
     *
     * - `{name} user_id` - Pomelo user_id
     * - `{name} social` - socials for that user
     *
     */
    [[eosio::on_notify("*::unsocial")]]
    void on_unsocial( const name user_id, const optional<name> social );

    /**
     * ## ACTION `cleartable`
     *
     * Clear table
     *
     * ### params
     *
     * - `{name} table_name` - table name, i.e. "transfers"
     * - `{uint16_t} [round_id]` - (optional) round ID
     * - `{uint64_t} [max_rows]` - (optional) max number of rows to clear, if 0 - clear all
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo cleartable '["transfers", 101, 500]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void cleartable( const name table_name, const optional<uint16_t> round_id, const optional<uint64_t> max_rows );

    /**
     * ## ACTION `removeuser`
     *
     * Remove users from all projects in this round and update all matchings
     *
     * ### params
     *
     * - `{vector<name>} user_ids` - user IDs to remove
     * - `{uint16_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo removeuser '[["user1.eosn", "user2.eosn"], 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void removeuser( const vector<name> user_ids, const uint16_t round_id );

    /**
     * ## ACTION `collapse`
     *
     * Collapse donations from {user_ids} users into {user_id} in {round_id} and recalculate all matchings
     *
     * ### params
     *
     * - `{set<name>} user_ids` - user IDs to collapse
     * - `{name} user_id` - user ID to collapse into
     * - `{uint16_t} round_id` - round ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo collapse '[["user2.eosn","user3.eosn","user4.eosn"], "user1.eosn", 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void collapse(set<name> user_ids, name user_id, uint16_t round_id);

    /**
     * ## ACTION `token`
     *
     * Set token information
     *
     * ### params
     *
     * - `{symbol} sym` - (primary key) symbol
     * - `{name} contract` - token contract
     * - `{uint64_t} min_amount` - min amount required when donating
     * - `{uint64_t} oracle_id` - Defibox oracle ID
     *
     * ### example
     *
     * ```bash
     * $ cleos push action app.pomelo token '["4,EOS", "eosio.token", 10000, 1]' -p app.pomelo
     * ```
     */
    [[eosio::action]]
    void token( const symbol sym, const name contract, const uint64_t min_amount, const uint64_t oracle_id );

    [[eosio::action]]
    void deltoken( const symbol_code symcode );

    [[eosio::action]]
    void setfunding( const name grant_id, const name user_id);

    [[eosio::action]]
    void setgrantid( const name grant_id, const name new_grant_id );

private:
    void transfer( const name from, const name to, const extended_asset value, const string memo );

    // getters
    double calculate_value(const extended_asset ext_quantity );
    name get_user_id( const name user );
    bool is_user( const name user_id );
    void validate_round( const uint16_t round_id );
    uint16_t get_active_round( const name grant_id );
    extended_asset calculate_fee( const extended_asset ext_quantity );

    // tokens
    tokens_row get_token( const extended_symbol ext_sym );
    tokens_row get_token( const extended_asset ext_quantity );
    bool is_token_enabled( const symbol_code symcode );

    // globals key/value
    // void set_key_value( const name key, const uint64_t value );
    // uint64_t get_key_value( const name key );
    // bool del_key( const name key );

    globals_row get_globals();

    template <typename T>
    void donate_project(const T& table, const name project_id, const name from, const extended_asset ext_quantity, const string memo );

    void donate_grant(const name grant_id, const extended_asset ext_quantity, const name user_id, const double value);

    template <typename T>
    void set_project(T& table, const name project_type, const name project_id, const name author_id, const name funding_account, const set<symbol_code> accepted_tokens );

    void save_transfer( const name from, const name to, const extended_asset ext_quantity, const asset fee, const string& memo, const name project_type, const name project_id, const double value );

    int get_index(const vector<name>& vec, name value);
    int get_index(const vector<contribution_t>& vec, name id);
    int get_index(const vector<uint16_t>& vec, uint16_t id);

    template <typename T>
    vector<T> remove_element(const vector<T>& vec, T id);

    template <typename T>
    vector<T> remove_elements(const vector<T>& vec, const vector<T>& id);

    template <typename T>
    void clear_table( T& table, uint64_t rows_to_clear );

    // update counters in status singleton
    void update_status( const uint32_t index, const uint32_t count );

    void update_social( const name user_id );
};
