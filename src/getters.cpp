
double pomelo::get_value( const extended_asset ext_quantity )
{
    const auto value_sym = config.get().value_symbol;

    if(ext_quantity.get_extended_symbol() == value_sym)
        return ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision() );

    //TODO: lookup on defibox current rate and convert

    return 10 * ext_quantity.quantity.amount / pow( 10, ext_quantity.quantity.symbol.precision() );
}

name pomelo::get_user_id( const name user ){

    eosn::login::accounts_table accounts( config.get_or_default().login_contract, config.get_or_default().login_contract.value );
    const auto account = accounts.get(user.value, "pomelo::get_user_id: user doesn't exist");

    return account.user_id;
}

double pomelo::get_user_mutliplier( const name user_id ){

    eosn::login::users_table users( config.get_or_default().login_contract, config.get_or_default().login_contract.value );
    const auto user = users.get(user_id.value, "pomelo::get_user_mutliplier: user id doesn't exist");

    check(user.status != "deleted"_n, get_self().to_string() + "::get_user_mutliplier: user is not allowed to donate");

    double multiplier = 1;

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
