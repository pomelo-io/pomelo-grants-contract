
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

    // state
    const uint64_t status = get_key_value("status"_n);
    check( status <= 1, "pomelo::on_transfer: contract is under maintenance");

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check( memo_parts.size() == 2, ERROR_INVALID_MEMO);
    const name project_type = sx::utils::parse_name(memo_parts[0]);
    const name project_id = sx::utils::parse_name(memo_parts[1]);
    const extended_asset ext_quantity = { quantity, get_first_receiver() };

    if (project_type == "grant"_n) {
        donate_project(_grants, project_id, from, to, ext_quantity, memo );
    } else if (project_type == "bounty"_n) {
        donate_project(_bounties, project_id, from, to, ext_quantity, memo );
    } else {
        check( false, ERROR_INVALID_MEMO);
    }
}

[[eosio::on_notify("*::social")]]
void pomelo::on_social( const name user_id, const set<name> socials )
{
    const auto round_id = get_key_value("round.id"_n);

    if ( round_id == 0 || get_first_receiver() != LOGIN_CONTRACT ) return;

    pomelo::rounds_table _rounds( get_self(), get_self().value );
    pomelo::match_table _match( get_self(), round_id );
    pomelo::users_table _users( get_self(), round_id );

    const auto round_itr = _rounds.find( round_id );
    if (round_itr == _rounds.end() || get_index( round_itr->user_ids, user_id ) == -1) return;

    const auto new_multiplier = get_user_boost_mutliplier( user_id );

    const auto user_itr = _users.find( user_id.value );
    if (user_itr == _users.end() || user_itr->multiplier == new_multiplier) return;

    const auto old_multiplier = user_itr->multiplier;
    for (int i = 0; i < user_itr->contributions.size(); i++) {

        const auto old_value = user_itr->contributions[i].value;
        const auto donated = old_value / (1 + old_multiplier);
        const auto new_value = donated * (1 + new_multiplier);
        const auto old_boost = donated * old_multiplier;
        const auto new_boost = donated * new_multiplier;

        const auto match_itr = _match.find( user_itr->contributions[i].id.value );
        double old_square = match_itr->square, new_square = 0;

        _match.modify( match_itr, get_self(), [&]( auto & row ) {
            row.sum_boost += new_boost - old_boost;
            row.sum_sqrt += sqrt(new_value) - sqrt(old_value);
            new_square = row.square = row.sum_sqrt * row.sum_sqrt;
            row.updated_at = current_time_point();
        });

        _rounds.modify( round_itr, get_self(), [&]( auto & row ) {
            row.sum_boost += new_boost - old_boost;
            row.sum_square += new_square - old_square;
            row.updated_at = current_time_point();
        });

        _users.modify( user_itr, get_self(), [&]( auto & row ) {
            row.multiplier = new_multiplier;
            row.contributions[i].value = new_value;
            row.updated_at = current_time_point();
        });
    }

}