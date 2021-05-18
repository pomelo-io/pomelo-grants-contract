#include <eosn-login-contract/login.eosn.hpp>
#include <sx.utils/utils.hpp>
#include "pomelo.hpp"

namespace eosn {

[[eosio::action]]
void pomelo::setstatus( const name status )
{
    require_auth( get_self() );

    auto _config = config.get_or_default();

    _config.status = status;
    config.set( _config, get_self() );
}


[[eosio::action]]
void pomelo::setvaluesym( const extended_symbol value_symbol )
{
    require_auth( get_self() );

    auto _config = config.get_or_default();

    _config.value_symbol = value_symbol;
    config.set( _config, get_self() );
}

/**
 * Notify contract when any token transfer notifiers relay contract
 */
[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // config
    check( config.exists(), "pomelo: config does not exist" );
    const name status = config.get().status;
    check( (status == "ok"_n || status == "testing"_n ), "pomelo::on_transfer: contract is under maintenance");

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check(memo_parts.size() == 2, ERROR_INVALID_MEMO);

    const auto project = sx::utils::parse_name(memo_parts[1]);
    check(project.value, "pomelo: invalid project name");

    const auto value = get_value( extended_asset{ quantity, get_first_receiver() });

    print("Funding ", project, " with ", quantity, " == ", value, " value");
    if(memo_parts[0] == "grant"){

    }
    else if(memo_parts[0] == "bounty"){

    }
    else {
        check(false, ERROR_INVALID_MEMO);
    }

    check(false, "TODO");

}

double pomelo::get_value( const extended_asset ext_quantity )
{
    const auto value_sym = config.get().value_symbol;

    if(ext_quantity.get_extended_symbol() == value_sym)
        return ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision() );

    //TODO: lookup on defibox current rate and convert

    return 10 * ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision() );
}

}