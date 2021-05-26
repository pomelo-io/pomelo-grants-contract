#include <eosio/native.hpp>
#include "login.eosn.hpp"

namespace eosn {

[[eosio::action]]
void login::create( const name user_id, const set<public_key> public_key )
{
    require_auth( get_self() );

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr == _users.end(), "login::create: [user_id] already exists" );
    check( !is_account( user_id ), "login::create: [user_id] account already exists" );
    check( public_key.size(), "login::create: [public_key] is empty" );

    // create user row
    _users.emplace( get_self(), [&]( auto & row ) {
        row.user_id = user_id;
        row.public_key = public_key;
        row.status = "created"_n;
        row.created_at = current_time_point();
        row.updated_at = current_time_point();
    });

    const auto authority = eosiosystem::authority{ 1, { { *public_key.begin(), 1 } }, { }, { } };

    eosiosystem::native::newaccount_action newaccount( "eosio"_n, { get_self(), "active"_n} );
    newaccount.send( get_self(), user_id, authority, authority );

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
