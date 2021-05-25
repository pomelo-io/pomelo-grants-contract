#include <sx.defibox/defibox.hpp>

using namespace sx;

double pomelo::get_value( const extended_asset ext_quantity )
{
    const double value = ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision());

    if (ext_quantity.get_extended_symbol() == VALUE_SYM)
        return  value;

    //if local node - just multiply by 10, i.e. 10 EOS => USDT = 100.0 value
    if (!is_account(defibox::code)) {
        return 10 * value;
    }

    // loop through all defibox pairs and find the one that works (alternatively: hardcode pairs)
    double rate = 0;
    defibox::pairs _pairs( defibox::code, defibox::code.value );
    for( const auto& row: _pairs ) {
        if ( row.token0.contract == ext_quantity.contract && row.token0.symbol == ext_quantity.quantity.symbol
            && row.token1.contract == VALUE_SYM.get_contract() && row.token1.symbol == VALUE_SYM.get_symbol() ) {
            return value * row.price0_last;
        }
        if ( row.token1.contract == ext_quantity.contract && row.token1.symbol == ext_quantity.quantity.symbol
            && row.token0.contract == VALUE_SYM.get_contract() && row.token0.symbol == VALUE_SYM.get_symbol() ) {
            return value * row.price1_last;
        }
    }

    check(false, "pomelo::get_value: can't convert amount to base symbol");
    return value;
}

name pomelo::get_user_id( const name account )
{
    eosn::login::accounts_table _accounts( LOGIN_CONTRACT, LOGIN_CONTRACT.value );
    const auto accounts = _accounts.get(account.value, "pomelo::get_user_id: account is not linked with EOSN Login");

    return accounts.user_id;
}

bool pomelo::is_user( const name user_id )
{
    eosn::login::users_table users( LOGIN_CONTRACT, LOGIN_CONTRACT.value );

    return users.find(user_id.value) != users.end();
}

double pomelo::get_user_boost_mutliplier( const name user_id )
{
    eosn::login::users_table users( LOGIN_CONTRACT, LOGIN_CONTRACT.value );
    const auto user = users.get(user_id.value, "pomelo::get_user_boost_mutliplier: user id doesn't exist");

    check(user.status != "deleted"_n, "pomelo::get_user_boost_mutliplier: user is not allowed to donate");

    double multiplier = 0;

    //each social gives 25% boost
    multiplier += user.socials.size() * 0.25;

    return multiplier;
}

uint64_t pomelo::get_current_round()
{
    state_table state(get_self(), get_self().value);
    auto round = state.get_or_default().round_id;
    if(round == 0) return 0;    //return 0 if no active rounds - still allowed for bounties

    pomelo::rounds_table rounds( get_self(), get_self().value );

    const auto now = current_time_point().sec_since_epoch();
    const auto row = rounds.get( round, "pomelo::get_current_round: invalid state.round");
    check(row.start_at.sec_since_epoch() <= now && now <= row.end_at.sec_since_epoch(), "pomelo::get_current_round: invalid state.round");

    return row.round;
}

name pomelo::get_status()
{
    state_table _state(get_self(), get_self().value);
    check( _state.exists(), "pomelo::get_status: contract is not initialized" );
    return _state.get().status;
}