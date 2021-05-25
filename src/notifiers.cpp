
[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // tables
    pomelo::grants_table _grants( get_self(), get_self().value );
    pomelo::bounties_table _bounties( get_self(), get_self().value );
    pomelo::state_table _state( get_self(), get_self().value );

    // state
    const name status = get_status();
    check( (status == "ok"_n || status == "testing"_n ), "pomelo::on_transfer: contract is under maintenance");

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check( memo_parts.size() == 2, ERROR_INVALID_MEMO);
    const name project_type = sx::utils::parse_name(memo_parts[0]);
    const name project_id = sx::utils::parse_name(memo_parts[1]);

    if (project_type == "grant"_n) {
        fund_project(_grants, project_id, from, to, extended_asset{ quantity, get_first_receiver() }, memo );

    } else if (project_type == "bounty"_n) {
        fund_project(_bounties, project_id, from, to, extended_asset{ quantity, get_first_receiver() }, memo );

    } else {
        check( false, ERROR_INVALID_MEMO);
    }
}

[[eosio::on_notify("*::social")]]
void pomelo::on_social( const name user_id, const set<name> socials )
{
    const auto round_id = get_current_round( );
    if ( round_id == 0 || get_first_receiver() != LOGIN_CONTRACT ) return;

    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    if(round_itr == rounds.end() || round_itr->user_ids.count( user_id ) == 0) return;

    const auto multiplier = get_user_boost_mutliplier( user_id );

    pomelo::match_grant_table match( get_self(), round_id );
    for(const auto grant_id: round_itr->grant_ids){
        const auto match_itr = match.find( grant_id.value );
        if(match_itr->user_multiplier.count( user_id ) == 0 || match_itr->user_multiplier.at(user_id) == multiplier) continue;

        const auto old_boost = match_itr->user_boost.at(user_id);
        const auto old_square = match_itr->square;
        const auto boost = match_itr->user_value.at(user_id) * multiplier;
        const auto user_sqrt = sqrt( match_itr->user_value.at(user_id) + boost );
        const auto sum_boost = match_itr->sum_boost - old_boost + boost;
        const auto sum_sqrt = match_itr->sum_sqrt - match_itr->user_sqrt.at(user_id) + user_sqrt;

        match.modify( match_itr, get_self(), [&]( auto & row ) {
            row.user_multiplier[user_id] = multiplier;
            row.user_boost[user_id] = boost;
            row.user_sqrt[user_id] = user_sqrt;
            row.sum_boost = sum_boost;
            row.sum_sqrt = sum_sqrt;
            row.square = sum_sqrt * sum_sqrt;
            row.updated_at = current_time_point();
        });

        rounds.modify( round_itr, get_self(), [&]( auto & row ) {
            row.sum_boost += boost - old_boost;
            row.sum_square += match_itr->square - old_square;
            row.updated_at = current_time_point();
        });

    }

}