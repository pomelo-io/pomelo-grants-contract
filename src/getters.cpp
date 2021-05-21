#include <sx.defibox/defibox.hpp>

using namespace sx;

double pomelo::get_value( const extended_asset ext_quantity )
{
    const double value = ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision());
    const auto value_sym = config.get().value_symbol;

    if(ext_quantity.get_extended_symbol() == value_sym)
        return  value;

    //if local node - just multiply by 10, i.e. 10 EOS => USDT = 100.0 value
    if(!is_account(defibox::code)){
        return 10 * value;
    }

    // loop through all defibox pairs and find the one that works (alternatively: hardcode pairs)
    double rate = 0;
    defibox::pairs _pairs( defibox::code, defibox::code.value );
    for( const auto& row: _pairs ){
        if( row.token0.contract == ext_quantity.contract && row.token0.symbol == ext_quantity.quantity.symbol
            && row.token1.contract == value_sym.get_contract() && row.token1.symbol == value_sym.get_symbol() ){
            return value * row.price0_last;
        }
        if( row.token1.contract == ext_quantity.contract && row.token1.symbol == ext_quantity.quantity.symbol
            && row.token0.contract == value_sym.get_contract() && row.token0.symbol == value_sym.get_symbol() ){
            return value * row.price1_last;
        }
    }

    check(false, get_self().to_string() + "::get_value: can't convert amount to base symbol");
    return value;
}

name pomelo::get_user_id( const name user ){

    eosn::login::accounts_table accounts( config.get_or_default().login_contract, config.get_or_default().login_contract.value );
    const auto account = accounts.get(user.value, "pomelo::get_user_id: user doesn't exist");

    return account.user_id;
}

double pomelo::get_user_boost_mutliplier( const name user_id ){

    eosn::login::users_table users( config.get_or_default().login_contract, config.get_or_default().login_contract.value );
    const auto user = users.get(user_id.value, "pomelo::get_user_boost_mutliplier: user id doesn't exist");

    check(user.status != "deleted"_n, get_self().to_string() + "::get_user_boost_mutliplier: user is not allowed to donate");

    double multiplier = 0;

    //each social gives 25% boost
    multiplier += user.socials.size() * 0.25;

    return multiplier;
}

uint64_t pomelo::get_current_round(){

    state_table state(get_self(), get_self().value);
    auto round = state.get_or_default().round_id;
    if(round == 0) return 0;    //return 0 if no active rounds - still allowed for bounties

    pomelo::rounds_table rounds( get_self(), get_self().value );

    const auto now = current_time_point().sec_since_epoch();
    const auto row = rounds.get( round, "pomelo::get_current_round: invalid state.round");
    check(row.start_at.sec_since_epoch() <= now && now <= row.end_at.sec_since_epoch(), get_self().to_string() + "::get_current_round: invalid state.round");

    return row.round;
}
