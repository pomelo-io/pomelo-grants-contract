
double pomelo::get_value( const extended_asset ext_quantity )
{
    const auto value_sym = config.get().value_symbol;

    if(ext_quantity.get_extended_symbol() == value_sym)
        return ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision() );

    //TODO: lookup on defibox current rate and convert

    return 10 * ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision() );
}

name pomelo::get_user_id( const name user ){

    eosn::login::accounts_table accounts( get_self(), get_self().value );
    const auto account = accounts.get(user.value, "pomelo: user doesn't exist");

    return account.user_id;
}

uint64_t pomelo::get_current_round(){

    state_table state(get_self(), get_self().value);
    auto round = state.get_or_default().round;
    check(round >= 0, "pomelo: no funding round ongoing");

    pomelo::rounds_table rounds( get_self(), get_self().value );

    const auto now = current_time_point().sec_since_epoch();
    const auto row = rounds.get(static_cast<uint64_t>( round ));
    check(row.start_at.sec_since_epoch() <= now && now <= row.end_at.sec_since_epoch(), "pomelo: invalid state.round");

    return row.round;
}
// alternative way to pull round - from rounds table
// uint64_t pomelo::get_current_round(){

//     const auto now = current_time_point().sec_since_epoch();
//     pomelo::rounds_table rounds( get_self(), get_self().value );
//     for(const auto& row: rounds){
//         if(row.start_at.sec_since_epoch() <= now && now <= row.end_at.sec_since_epoch()) return row.round;
//     }

//     check(false, "pomelo: no funding round ongoing");
//     return -1;
// }

