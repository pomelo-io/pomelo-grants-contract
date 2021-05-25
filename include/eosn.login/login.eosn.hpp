#pragma once

#include <eosio/eosio.hpp>
#include <eosio/crypto.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

#include <optional>
#include <set>

using namespace eosio;
using namespace std;

namespace eosn {

class [[eosio::contract("login.eosn")]] login : public eosio::contract {
public:
    using contract::contract;

    /**
     * ## TABLE `users`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `bystatus`     | 2                | i64        |
     * | `byupdated`    | 3                | i64        |
     *
     * ### params
     *
     * - `{name} user_id` - (primary key) user ID
     * - `{set<public_key>} public_key` - public key for account creation permission
     * - `{set<name>} [accounts=[]]` - EOS account names
     * - `{set<name>} [socials=[]]` - social accounts enabled
     * - `{name} [status="pending"]` - user status (ex: `created/ok/deleted`)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} authorize_at` - authorize at time
     * - `{time_point_sec} deleted_at` - deleted at time
     *
     * ### example
     *
     * ```json
     * {
     *     "user_id": "123.eosn",
     *     "public_key": ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"],
     *     "accounts": ["myaccount"],
     *     "socials": ["github"],
     *     "status": "ok",
     *     "created_at": "2020-12-06T00:00:00",
     *     "updated_at": "2020-12-06T00:00:00",
     *     "authorize_at": "2020-12-06T00:00:00",
     *     "deleted_at": "1970-01-01T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("users")]] users_row {
        name                user_id;
        set<public_key>     public_key;
        set<name>           accounts;
        set<name>           socials;
        name                status;
        time_point_sec      created_at;
        time_point_sec      updated_at;
        time_point_sec      authorize_at;
        time_point_sec      deleted_at;

        uint64_t primary_key() const { return user_id.value; }
        uint64_t bystatus() const { return status.value; };
        uint64_t byupdated() const { return updated_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "users"_n, users_row,
        indexed_by< "bystatus"_n, const_mem_fun<users_row, uint64_t, &users_row::bystatus> >,
        indexed_by< "byupdated"_n, const_mem_fun<users_row, uint64_t, &users_row::byupdated> >
    > users_table;

    /**
     * ## TABLE `accounts`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `byuser`       | 2                | i64        |
     *
     * ### params
     *
     * - `{name} account` - (primary key) EOS account
     * - `{name} user_id` - user ID
     *
     * ### example
     *
     * ```json
     * {
     *     "account": "myaccount",
     *     "user_id": "123.eosn"
     * }
     * ```
     */
    struct [[eosio::table("accounts")]] accounts_row {
        name                account;
        name                user_id;

        uint64_t primary_key() const { return account.value; }
        uint64_t byuser() const { return user_id.value; };
    };
    typedef eosio::multi_index< "accounts"_n, accounts_row,
        indexed_by< "byuser"_n, const_mem_fun<accounts_row, uint64_t, &accounts_row::byuser> >
    > accounts_table;

    /**
     * ## TABLE `global`
     *
     * ### params
     *
     * - `{name} key` - (primary key) key
     * - `{uint64_t} value` - value
     *
     * ### example
     *
     * ```json
     * {
     *     "key": "nonce",
     *     "value": 1
     * }
     * ```
     */
    struct [[eosio::table("global")]] global_row {
        name                key;
        uint64_t            value;

        uint64_t primary_key() const { return key.value; }
    };
    typedef eosio::multi_index< "global"_n, global_row> global_table;

    /**
     * ## ACTION `create`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{set<public_key>} public_key` - public key for account creation permission
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn create '["123.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void create( const name user_id, const set<public_key> public_key );

    /**
     * ## ACTION `status`
     *
     * - **authority**: `get_self()` or `signature`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} status` - user status (ex: `created/ok/deleted`)
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn status '["123.eosn", "ok"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void status( const name user_id, const name status );

    /**
     * ## ACTION `deluser`
     *
     * - **authority**: `get_self()` or `signature`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn deluser '["123.eosn"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void deluser( const name user_id );

    /**
     * ## ACTION `link`
     *
     * - **authority**: `get_self()` or `signature`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{name} [accounts=[]]` - EOS account names
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn link '["123.eosn", ["myaccount"]]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void link( const name user_id, const set<name> accounts );

    /**
     * ## ACTION `unlink`
     *
     * - **authority**: `get_self()` or `signature`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn unlink '["123.eosn"]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void unlink( const name user_id );

    /**
     * ## ACTION `social`
     *
     * - **authority**: `get_self()` or `signature`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{set<name>} [socials=[]]` - social accounts enabled
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn social '["123.eosn", ["github"]]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void social( const name user_id, const set<name> socials );

    [[eosio::action]]
    void authorize( const name user_id );

    using create_action = eosio::action_wrapper<"create"_n, &eosn::login::create>;
    using status_action = eosio::action_wrapper<"status"_n, &eosn::login::status>;
    using deluser_action = eosio::action_wrapper<"deluser"_n, &eosn::login::deluser>;
    using link_action = eosio::action_wrapper<"link"_n, &eosn::login::link>;
    using unlink_action = eosio::action_wrapper<"unlink"_n, &eosn::login::unlink>;
    using social_action = eosio::action_wrapper<"social"_n, &eosn::login::social>;
    using authorize_action = eosio::action_wrapper<"authorize"_n, &eosn::login::authorize>;

private:
    void unlink_user( const name user_id );
    bool is_auth( const name user_id );
    void require_auth_user_id( const name user_id );
    // void increase_authorize_nonce( const uint64_t nonce );
};

} // namespace eosn