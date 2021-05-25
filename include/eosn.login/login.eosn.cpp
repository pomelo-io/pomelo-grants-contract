#include "login.eosn.hpp"

namespace eosn {

[[eosio::action]]
void login::create( const name user_id, const set<public_key> public_key )
{
    require_auth( get_self() );
    require_auth_user_id( user_id );

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr == _users.end(), get_self().to_string() + "::create: [user_id] already exists" );
    check( !is_account( user_id ), get_self().to_string() + "::create: [user_id] account already exists" );

    // create user row
    _users.emplace( get_self(), [&]( auto & row ) {
        row.user_id = user_id;
        row.public_key = public_key;
        row.status = "created"_n;
        row.created_at = current_time_point();
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::status( const name user_id, const name status )
{
    require_auth( get_self() );
    require_auth_user_id( user_id );

    login::users_table _users( get_self(), get_self().value );

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), get_self().to_string() + "::status: [user_id] does not exist" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        check( row.status != status, get_self().to_string() + "::status: was not modified" );
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
    check( itr != _users.end(), get_self().to_string() + "::deluser: [user_id] does not exist" );
    check( itr->deleted_at.sec_since_epoch() == 0, get_self().to_string() + "::deluser: [user_id] is already deleted" );

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
    check( itr != _users.end(), get_self().to_string() + "::link: [user_id] does not exist" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.accounts = accounts;
        row.updated_at = current_time_point();
    });

    // remove any accounts linked to user id
    unlink_user( user_id );

    // link all accounts related to user id
    for ( const name account : accounts ) {
        check( is_account( account ), get_self().to_string() + "::link: [" + account.to_string() + "] account does not exist" );
        auto accounts_itr = _accounts.find( account.value );
        check( accounts_itr == _accounts.end(), get_self().to_string() + "::link: [" + account.to_string() + "] account already linked with [" + accounts_itr->user_id.to_string() + "] user_id" );

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
    check( itr != _users.end(), get_self().to_string() + "::unlink: [user_id] does not exist" );
    check( itr->accounts.size(), get_self().to_string() + "::unlink: [user_id] has no linked accounts" );

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

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), get_self().to_string() + "::social: [user_id] does not exist" );

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

    // get user
    login::users_table _users( get_self(), get_self().value );
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), get_self().to_string() + "::authorize: [user_id] does not exist" );

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.authorize_at = current_time_point();
    });
}

bool login::is_auth( const name user_id )
{
    login::users_table _users( get_self(), get_self().value );
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), get_self().to_string() + "::is_auth: [user_id] does not exist" );

    if ( !is_account(user_id) ) return true; // TEMP - if account is not created, skip
    if ( has_auth( user_id ) ) return true;

    for ( const name account : itr->accounts ) {
        if ( has_auth(account)) return true;
    }
    return false;
}

void login::require_auth_user_id( const name user_id )
{
    check( is_auth( user_id ), get_self().to_string() + "::require_auth_user_id: [user_id] is not authorized" );
}

// void login::increase_authorize_nonce( const uint64_t nonce )
// {
//     login::global_table _global( get_self(), get_self().value );
//     auto itr = _global.find( "nonce"_n.value );

//     // update global row
//     // nonce must match current authorization nonce
//     if ( itr != _global.end() ) {
//         check( itr->value == nonce, get_self().to_string() + "::increase_authorize_nonce: [nonce] does not match current authorization nonce" );
//         _global.modify( itr, get_self(), [&]( auto & row ) {
//             row.value += 1;
//         });
//     } else {
//         _global.emplace( get_self(), [&]( auto & row ) {
//             row.key = "nonce"_n;
//             row.value = 1;
//         });
//     }
// }

} // namespace eosn
