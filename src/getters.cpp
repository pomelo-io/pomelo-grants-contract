#include <sx.defibox/defibox.hpp>

using namespace sx;

extended_asset pomelo::calculate_fee( const extended_asset ext_quantity )
{
    const int64_t amount = ext_quantity.quantity.amount * get_globals().system_fee / 10000;
    return { amount, ext_quantity.get_extended_symbol() };
}

pomelo::globals_row pomelo::get_globals()
{
    pomelo::globals_table _globals( get_self(), get_self().value );
    check( _globals.exists(), "pomelo::get_global: contract is under maintenance");
    return _globals.get();
}

extended_symbol pomelo::get_token( const symbol_code symcode )
{
    pomelo::tokens_table _tokens( get_self(), get_self().value );
    auto itr = _tokens.find( symcode.raw() );
    if ( itr == _tokens.end() ) return {};
    return extended_symbol{ itr->sym, itr->contract };
}

bool pomelo::is_token_enabled( const symbol_code symcode )
{
    pomelo::tokens_table _tokens( get_self(), get_self().value );
    auto itr = _tokens.find( symcode.raw() );
    return itr != _tokens.end();
}

int64_t pomelo::get_token_min_amount( const symbol_code symcode )
{
    pomelo::tokens_table _tokens( get_self(), get_self().value );
    auto itr = _tokens.get( symcode.raw(), "pomelo::get_token_min_amount: [symcode] not found" );
    return itr.min_amount;
}

double pomelo::calculate_value( const extended_asset ext_quantity )
{
    const double value = ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision());

    if ( ext_quantity.get_extended_symbol() == VALUE_SYM ) {
        return value;
    }

    //if local node - just divide by 10, i.e. 10 USDT => EOS = 1.0 value
    if (!is_account(defibox::code)) {
        return value / 10;
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
    const auto itr = _accounts.find(account.value);
    // check( itr != _accounts.end(), "pomelo::get_user_id: [account=" + account.to_string() + "] is not linked with EOSN Login");
    if ( itr == _accounts.end() ) return account;
    return itr->user_id;
}

bool pomelo::is_user( const name user_id )
{
    eosn::login::users_table users( LOGIN_CONTRACT, LOGIN_CONTRACT.value );

    return users.find(user_id.value) != users.end();
}

void pomelo::validate_round( const uint16_t round_id )
{
    pomelo::rounds_table _rounds( get_self(), get_self().value );

    check(round_id != 0, "pomelo::validate_round: [round_id] is not active");

    const auto now = current_time_point().sec_since_epoch();
    const auto rounds = _rounds.get( round_id, "pomelo::validate_round: [round_id] not found");
    check(rounds.start_at.sec_since_epoch() <= now, "pomelo::validate_round: [round_id] has not started");
    check(now <= rounds.end_at.sec_since_epoch(), "pomelo::validate_round: [round_id] has expired");
}
