#include <sx.defibox/defibox.hpp>

using namespace sx;

double pomelo::calculate_value( const extended_asset ext_quantity )
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
    check(false, "pomelo::calculate_value: can't convert amount to base symbol");
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

void pomelo::validate_round( const uint64_t round_id )
{
    pomelo::rounds_table _rounds( get_self(), get_self().value );

    check(round_id != 0, "pomelo::validate_round: [round_id] is not active");

    const auto now = current_time_point().sec_since_epoch();
    const auto rounds = _rounds.get( round_id, "pomelo::validate_round: [round_id] not found");
    check(rounds.start_at.sec_since_epoch() <= now, "pomelo::validate_round: [round_id] has not started");
    check(now <= rounds.end_at.sec_since_epoch(), "pomelo::validate_round: [round_id] has expired");
}

void pomelo::set_key_value( const name key, const uint64_t value )
{
    globals_table _globals(get_self(), get_self().value);
    auto insert = [&]( auto & row ) {
        row.key = key;
        row.value = value;
    };
    auto itr = _globals.find( key.value );
    if ( itr == _globals.end() ) _globals.emplace( get_self(), insert );
    else _globals.modify( itr, get_self(), insert );
}

uint64_t pomelo::get_key_value( const name key )
{
    globals_table _globals(get_self(), get_self().value);
    auto itr = _globals.find( key.value );
    check( itr != _globals.end() ,"pomelo::get_key_value: [" + key.to_string() + "] key does not exists");
    return itr->value;
}

bool pomelo::del_key( const name key )
{
    globals_table _globals(get_self(), get_self().value);
    auto itr = _globals.find( key.value );
    if ( itr != _globals.end() ) {
        _globals.erase( itr );
        return true;
    }
    return false;
}