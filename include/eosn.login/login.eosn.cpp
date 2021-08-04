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

    // add created account to linked table
    login::accounts_table _accounts( get_self(), get_self().value );
    _accounts.emplace( get_self(), [&]( auto & row ) {
        row.account = user_id;
        row.user_id = user_id;
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

    // notify contracts
    if ( is_account(POMELO_CONTRACT) ) require_recipient(POMELO_CONTRACT);

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

    // notify contracts
    if ( is_account(POMELO_CONTRACT) ) require_recipient(POMELO_CONTRACT);

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

    // notify contracts
    if ( is_account(POMELO_CONTRACT) ) require_recipient(POMELO_CONTRACT);

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
    login::socials_table _socials( get_self(), get_self().value );

    // notify contracts
    if ( is_account(POMELO_CONTRACT) ) require_recipient(POMELO_CONTRACT);

    // validate user ID
    auto itr = _users.find( user_id.value );
    check( itr != _users.end(), "login::social: [user_id] does not exist" );
    for(const auto& social: socials){
        check( _socials.find( social.value ) != _socials.end(), "login::social: one of the [socials] is unknown" );
    }

    // modify user row
    _users.modify( itr, get_self(), [&]( auto & row ) {
        row.socials = socials;
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void login::proof( const name account, const uint64_t nonce, const optional<string> data )
{
    require_auth( account );

    login::proofs_table _proofs( get_self(), get_self().value );

    // create row
    _proofs.emplace( get_self(), [&]( auto & row ) {
        row.id = _proofs.available_primary_key();
        row.account = account;
        row.nonce = nonce;
        row.data = *data;
        row.created_at = current_time_point();
    });

    // delete last 10 rows after 24 hours
    auto index = _proofs.get_index<"bycreated"_n>();
    auto itr = index.begin();
    vector<uint64_t> to_erase;
    while ( true ) {
        const uint64_t time_delta = current_time_point().sec_since_epoch() - itr->created_at.sec_since_epoch();
        if ( itr == index.end() ) break;
        if ( time_delta <= 86400 ) break;
        if ( to_erase.size() >= 10 ) break;
        print(to_string(itr->id) + ":" + to_string(time_delta) + "\n");
        to_erase.push_back( itr->id );
        itr++;
    }
    for ( const uint64_t id : to_erase ) {
        _proofs.erase(_proofs.find( id ));
    }
}

[[eosio::action]]
void login::authorize( const name user_id )
{
    require_auth_user_id( user_id );
}

[[eosio::action]]
void login::setsocial( const name social, const uint32_t weight )
{
    require_auth( get_self() );
    check( weight <= 200, "login::setsocial: [weight] should be <= 200");

    login::socials_table _socials( get_self(), get_self().value );

    auto insert = [&]( auto & row ) {
        row.social = social;
        row.weight = weight;
    };
    const auto itr = _socials.find( social.value );
    if ( itr == _socials.end() ) _socials.emplace( get_self(), insert );
    else {
        if( weight == 0 ) _socials.erase( itr );
        else _socials.modify( itr, get_self(), insert );
    }
}

} // namespace eosn
