
[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // config
    check( config.exists(), get_self().to_string() + "::on_transfer: config does not exist" );
    const name status = config.get().status;
    check( (status == "ok"_n || status == "testing"_n ), get_self().to_string() + ": contract is under maintenance");

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check( memo_parts.size() == 2 && (memo_parts[0] == "grant" || memo_parts[0] == "bounty"),
        get_self().to_string() + "::on_transfer: invalid memo, use \"grant:mygrant\" or \"bounty:mybounty\"" );

    const auto project = sx::utils::parse_name(memo_parts[1]);
    check(project.value, get_self().to_string() + "::on_transfer: invalid project name");

    if(memo_parts[0] == "grant"){

        pomelo::grants_table grants( get_self(), get_self().value );
        fund_project(grants, project, extended_asset{ quantity, get_first_receiver() }, from, memo);
    }
    if(memo_parts[0] == "bounty"){

        pomelo::bounties_table bounties( get_self(), get_self().value );
        fund_project(bounties, project, extended_asset{ quantity, get_first_receiver() }, from, memo);
    }
}

[[eosio::on_notify("*::social")]]
void pomelo::on_social( const name user_id, const set<name> socials )
{
    const auto round_id = get_current_round( );
    if(round_id == 0 || get_first_receiver() != config.get_or_default().login_contract) return;

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