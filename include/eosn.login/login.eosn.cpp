#include <eosio/native.hpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>

#include "login.eosn.hpp"

namespace eosn {

[[eosio::action]]
void login::create( const name user_id, const set<public_key> public_keys )
{
    require_auth( get_self() );

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    const name suffix = user_id.suffix();
    auto itr = _users.find( user_id.value );
    check( itr == _users.end(), "login::create: [user_id] already exists" );
    check( !is_account( user_id ), "login::create: [user_id] account already exists" );
    check( public_keys.size(), "login::create: [public_keys] is empty" );
    check( suffix.value, "login::create: [user_id] does not include a suffix" );
    check( suffix == "eosn"_n, "login::create: [user_id] suffix must be *.eosn" );

    // create user row
    _users.emplace( get_self(), [&]( auto & row ) {
        row.user_id = user_id;
        row.public_keys = public_keys;
        row.status = "created"_n;
        row.created_at = current_time_point();
        row.updated_at = current_time_point();
    });

    // setup account permission
    vector<eosiosystem::key_weight> keys;
    for ( const public_key key : public_keys ) {
        keys.push_back({ key, 1 });
    }
    const vector<eosiosystem::permission_level_weight> accounts = {{ { get_self(), "eosio.code"_n }, 1 }};
    const eosiosystem::authority owner = { 1, {}, accounts, {} };
    const eosiosystem::authority active = { 1, keys, {}, {} };

    // create new account
    eosiosystem::native::newaccount_action newaccount( "eosio"_n, { suffix, "active"_n} );
    newaccount.send( suffix, user_id, owner, active );

    // purchase 1,735 bytes worth of RAM (no excess RAM)
    eosiosystem::system_contract::buyrambytes_action buyrambytes( "eosio"_n, { get_self(), "active"_n });
    buyrambytes.send( get_self(), user_id, 1735 );

    // open EOS token balance
    eosio::token::open_action open( "eosio.token"_n, { get_self(), "active"_n });
    open.send( get_self(), symbol{"EOS", 4}, get_self() );
}

[[eosio::action]]
void login::status( const name user_id, const name status )
{
    require_auth( get_self() );
    require_auth_user_id( user_id );

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::status: [user_id] does not exist" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        check( row.status != status, "login::status: was not modified" );
        row.status = status;
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::deluser( const name user_id )
{
    require_auth( get_self() );
    require_auth_user_id( user_id );

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::deluser: [user_id] does not exist" );
    check( itr->deleted_at.sec_since_epoch() == 0, "login::deluser: [user_id] is already deleted" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.deleted_at = current_time_point();
    });
}

[[eosio::action]]
void login::link( const name user_id, const set<name> accounts )
{
    require_auth( get_self() );
    require_auth_user_id( user_id );

    login::users_table _users( get_self(), get_self().value );
    login::accounts_table _accounts( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::link: [user_id] does not exist" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.accounts = accounts;
        row.updated_at = current_time_point();
    });

    // remove any accounts linked to user id
    unlink_user( user_id );

    // link all accounts related to user id
    for ( const name account : accounts ) {
        check( is_account( account ), "login::link: [" + account.to_string() + "] account does not exist" );
        auto accounts_itr = _accounts.find( account.value );
        check( accounts_itr == _accounts.end(), "login::link: [" + account.to_string() + "] account already linked with [" + accounts_itr->user_id.to_string() + "] user_id" );

        _accounts.emplace( get_self(), [&]( auto & row ) {
            row.account = account;
            row.user_id = user_id;
        });
    }
}

[[eosio::action]]
void login::unlink( const name user_id )
{
    require_auth( get_self() );
    require_auth_user_id( user_id );

    login::users_table _users( get_self(), get_self().value );

    // remove any accounts linked to user id
    unlink_user( user_id );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::unlink: [user_id] does not exist" );
    check( itr->accounts.size(), "login::unlink: [user_id] has no linked accounts" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.accounts = {};
        row.updated_at = current_time_point();
    });
}

void login::unlink_user( const name user_id )
{
    login::accounts_table _accounts( get_self(), get_self().value );

    auto index = _accounts.get_index<"byuser"_n>();
    auto itr = index.find( user_id.value );
    if ( itr != index.end() ) index.erase( itr );
}

[[eosio::action]]
void login::social( const name user_id, const set<name> socials )
{
    require_auth( get_self() );
    require_auth_user_id( user_id );

    login::users_table _users( get_self(), get_self().value );

    // notify contracts
    if ( is_account("pomelo"_n) ) require_recipient("pomelo"_n);

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::social: [user_id] does not exist" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.socials = socials;
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::authorize( const name user_id )
{
    require_auth_user_id( user_id );
}

} // namespace eosn
