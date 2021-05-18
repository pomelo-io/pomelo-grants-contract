#include "pomelo.hpp"

[[eosio::action]]
void pomelo::setuser( const uint64_t user_id, const name eos_account, const map<name, uint8_t>& social )
{
    check(false, "TODO");
}

[[eosio::action]]
void pomelo::userstatus( const uint64_t user_id, const name status )
{
    check(false, "TODO");
}

[[eosio::action]]
void pomelo::setstatus( const name status )
{
    require_auth( get_self() );

    pomelo::config_table _config( get_self(), get_self().value );
    auto config = _config.get_or_default();

    config.status = status;
    _config.set( config, get_self() );
}


/**
 * Notify contract when any token transfer notifiers relay contract
 */
[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    pomelo::config_table _config( get_self(), get_self().value );

    // config
    check( _config.exists(), "pomelo: config does not exist" );
    const name status = _config.get().status;
    check( (status == "ok"_n || status == "testing"_n ), "pomelo::on_transfer: contract is under maintenance");

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;



    check(false, "TODO");

}