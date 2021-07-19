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

    static constexpr name LOGIN_CONTRACT = "login.eosn"_n;
    static constexpr name POMELO_CONTRACT = "app.pomelo"_n;

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
     * - `{set<public_key>} public_keys` - public key for account creation permission
     * - `{set<name>} [accounts=[]]` - EOS account names
     * - `{set<name>} [socials=[]]` - social accounts enabled
     * - `{name} [status="pending"]` - user status (ex: `created/ok/deleted`)
     * - `{time_point_sec} created_at` - created at time
     * - `{time_point_sec} updated_at` - updated at time
     * - `{time_point_sec} deleted_at` - deleted at time
     *
     * ### example
     *
     * ```json
     * {
     *     "user_id": "123.eosn",
     *     "public_keys": ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"],
     *     "accounts": ["myaccount"],
     *     "socials": ["github"],
     *     "status": "ok",
     *     "created_at": "2020-12-06T00:00:00",
     *     "updated_at": "2020-12-06T00:00:00",
     *     "deleted_at": "1970-01-01T00:00:00"
     * }
     * ```
     */
    struct [[eosio::table("users")]] users_row {
        name                user_id;
        set<public_key>     public_keys;
        set<name>           accounts;
        set<name>           socials;
        name                status;
        time_point_sec      created_at;
        time_point_sec      updated_at;
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
     * ## TABLE `socials`
     *
     * scope: contract, i.e. "app.pomelo"_n.value
     *
     * ### params
     *
     * - `{name} social` - (primary key) social
     * - `{uint32_t} weight` - social weight(0-100), i.e. 50 gives 50% boost
     *
     * ### example
     *
     * ```json
     * {
     *     "social": "eden"_n,
     *     "weight": "50"
     * }
     * ```
     */
    struct [[eosio::table("socials")]] socials_row {
        name                social;
        uint32_t            weight;

        uint64_t primary_key() const { return social.value; }
    };
    typedef eosio::multi_index< "socials"_n, socials_row> socials_table;

    /**
     * ## TABLE `proofs`
     *
     * ### multi-indexes
     *
     * | `param`        | `index_position` | `key_type` |
     * |--------------- |------------------|------------|
     * | `byaccount`    | 2                | i64        |
     * | `bynonce`      | 3                | i64        |
     * | `bycreated`    | 4                | i64        |
     *
     * ### params
     *
     * - `{uint64_t} id` - (primary key) incremental ID
     * - `{name} account` - nonce number
     * - `{uint64_t} nonce` - nonce number
     * - `{string} data` - (optional) string data
     * - `{time_point_sec} created_at` - created at time
     *
     * ### example
     *
     * ```json
     * {
     *     "id": 1,
     *     "account": "myaccount",
     *     "nonce": 1,
     *     "data": "any data",
     *     "created_at": "2020-12-06T00:00:00",
     * }
     * ```
     */
    struct [[eosio::table("proofs")]] proofs_row {
        uint64_t            id;
        name                account;
        uint64_t            nonce;
        string              data;
        time_point_sec      created_at;

        uint64_t primary_key() const { return id; }
        uint64_t byaccount() const { return account.value; };
        uint64_t bynonce() const { return nonce; };
        uint64_t bycreated() const { return created_at.sec_since_epoch(); };
    };
    typedef eosio::multi_index< "proofs"_n, proofs_row,
        indexed_by< "byaccount"_n, const_mem_fun<proofs_row, uint64_t, &proofs_row::byaccount> >,
        indexed_by< "bynonce"_n, const_mem_fun<proofs_row, uint64_t, &proofs_row::bynonce> >,
        indexed_by< "bycreated"_n, const_mem_fun<proofs_row, uint64_t, &proofs_row::bycreated> >
        // indexed_by< "byexpired"_n, const_mem_fun<proofs_row, uint64_t, &proofs_row::byexpired> >
    > proofs_table;

    /**
     * ## ACTION `create`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     * - `{set<public_key>} public_keys` - public keys for account creation permission
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn create '["123.eosn", ["EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV"]]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void create( const name user_id, const set<public_key> public_keys );

    /**
     * ## ACTION `status`
     *
     * - **authority**: `get_self()` and (`user_id` or `accounts`)
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
     * - **authority**: `get_self()` and (`user_id` or `accounts`)
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
     * - **authority**: `get_self()` and (`user_id` or `accounts`)
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
     * - **authority**: `get_self()` and (`user_id` or `accounts`)
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
     * - **authority**: `get_self()` and (`user_id` or `accounts`)
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

    /**
     * ## ACTION `setsocial`
     *
     * - **authority**: `get_self()`
     *
     * ### params
     *
     * - `{name} contract` - app contract
     * - `{name} social` - social
     * - `{uint32_t} weight` - social weight
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn setsocial '["app.pomelo", "eden", 50]' -p login.eosn
     * ```
     */
    [[eosio::action]]
    void setsocial( const name contract, const name social, const uint32_t weight );

    /**
     * ## ACTION `proof`
     *
     * - **authority**: `account`
     *
     * ### params
     *
     * - `{name} account` - account name
     * - `{uint64_t} nonce` - proof nonce
     * - `{string} [data=""]` - (optional) string data
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn proof '["myaccount", 123, "any data"]' -p myaccount
     * ```
     */
    [[eosio::action]]
    void proof( const name account, const uint64_t nonce, const optional<string> data );

    /**
     * ## ACTION `authorize`
     *
     * - **authority**: `user_id` or `accounts`
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     *
     * ### Example
     *
     * ```bash
     * $ cleos push action login.eosn authorize '["123.eosn"]' -p 123.eosn
     * ```
     */
    [[eosio::action]]
    void authorize( const name user_id );

    /**
     * ## STATIC `is_auth`
     *
     * Returns true/false if user ID is authorized
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     *
     * ### returns
     *
     * - `{bool}` - [true/false] if user is authorized
     *
     * ### example
     *
     * ```c++
     * const name user_id = "123.eosn"_n;
     * const bool is_auth  = eosn::login::is_auth( user_id );
     * //=> true
     * ```
     */
    static bool is_auth( const name user_id )
    {
        login::users_table _users( LOGIN_CONTRACT, LOGIN_CONTRACT.value );
        auto users = _users.get( user_id.value, "login::is_auth: [user_id] does not exist");

        if ( has_auth( user_id ) ) return true;

        for ( const name account : users.accounts ) {
            if ( has_auth(account) ) return true;
        }
        return false;
    }

    /**
     * ## STATIC `require_auth_user_id`
     *
     * Asserts error if user ID is authorized or not
     *
     * ### params
     *
     * - `{name} user_id` - user ID
     *
     * ### returns
     *
     * - `{void}` - throw error if user ID is authorized or not
     *
     * ### example
     *
     * ```c++
     * const name user_id = "123.eosn"_n;
     * eosn::login::require_auth_user_id( user_id );
     * ```
     */
    static void require_auth_user_id( const name user_id )
    {
        check( is_auth( user_id ), "login::require_auth_user_id: [user_id] is not authorized" );
    }

    /**
     * ## STATIC `get_user_weight`
     *
     * Calculate user weight based on socials (sum of all weights)
     *
     * ### params
     *
     * - `{name} contract` - contract, i.e. "app.pomelo"_n
     * - `{name} user_id` - user ID
     *
     * ### returns
     *
     * - `{uint32_t}` - resulting weight
     *
     * ### example
     *
     * ```c++
     * const name contract = "app.pomelo"_n;
     * const name user_id = "123.eosn"_n;
     * //=> 150
     * ```
     */
    static uint32_t get_user_weight( const name contract, const name user_id )
    {
        login::users_table _users( LOGIN_CONTRACT, LOGIN_CONTRACT.value );
        login::socials_table _socials( LOGIN_CONTRACT, contract.value );

        auto user = _users.get( user_id.value, "login::get_user_weight: [user_id] does not exist" );
        check( user.status != "deleted"_n, "login::get_user_weight: user is deleted" );
        uint32_t total_weight = 0;
        for ( const auto& row : _socials ) {
            if(user.socials.count( row.social ))
                total_weight += row.weight;
        }
        return total_weight;
    }

    using create_action = eosio::action_wrapper<"create"_n, &eosn::login::create>;
    using status_action = eosio::action_wrapper<"status"_n, &eosn::login::status>;
    using deluser_action = eosio::action_wrapper<"deluser"_n, &eosn::login::deluser>;
    using link_action = eosio::action_wrapper<"link"_n, &eosn::login::link>;
    using unlink_action = eosio::action_wrapper<"unlink"_n, &eosn::login::unlink>;
    using social_action = eosio::action_wrapper<"social"_n, &eosn::login::social>;
    using authorize_action = eosio::action_wrapper<"authorize"_n, &eosn::login::authorize>;

private:
    void unlink_user( const name user_id );
};

} // namespace eosn