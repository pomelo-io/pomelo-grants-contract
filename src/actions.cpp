

[[eosio::action]]
void pomelo::setgrant( const name id, const name author_id, const set<name> authorized_ids, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    require_auth( get_self() );

    pomelo::grants_table grants( get_self(), get_self().value );
    set_project( grants, "grant"_n, id, author_id, authorized_ids, funding_account, accepted_tokens );

}

[[eosio::action]]
void pomelo::setbounty( const name id, const name author_id, const set<name> authorized_ids, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    require_auth( get_self() );

    pomelo::bounties_table bounties( get_self(), get_self().value );
    set_project( bounties, "bounty"_n, id, author_id, authorized_ids, funding_account, accepted_tokens );

}

[[eosio::action]]
void pomelo::setprjstatus( const name project_id, const name status )
{
    require_auth( get_self() );
    check( status == "ok"_n || status == "pending"_n || status == "disabled"_n, get_self().to_string() + "::setprjstatus: invalid status" );

    auto modify = [&]( auto & row ) {
        row.status = status;
        row.updated_at = current_time_point();
    };

    pomelo::grants_table grants( get_self(), get_self().value );
    const auto itr1 = grants.find( project_id.value );
    if( itr1 != grants.end() ){
        check( itr1->status != status, get_self().to_string() + "::setprjstatus: status must be different");
        grants.modify( itr1, get_self(), modify);
        return;
    }

    pomelo::bounties_table bounties( get_self(), get_self().value );
    const auto itr2 = bounties.find( project_id.value );
    if( itr2 != bounties.end() ){
        check( itr2->status != status, get_self().to_string() + "::setprjstatus: status must be different");
        bounties.modify( itr2, get_self(), modify);
        return;
    }

    check( false, get_self().to_string() + "::setprjstatus: project doesn't exist" );
}

[[eosio::action]]
void pomelo::setround( const uint64_t round_id, const time_point_sec start_at, const time_point_sec end_at )
{
    require_auth( get_self() );
    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto itr = rounds.find( round_id );

    const auto insert = [&]( auto & row ) {
        row.round = round_id;
        row.start_at = start_at;
        row.end_at = end_at;
        row.updated_at = current_time_point();
        if( itr == rounds.end() ) row.created_at = current_time_point();
    };

    if ( itr == rounds.end() ) rounds.emplace( get_self(), insert );
    else rounds.modify( itr, get_self(), insert );
}

[[eosio::action]]
void pomelo::joinround( const name grant_id, const uint64_t round_id )
{
    require_auth( get_self() );

    pomelo::grants_table grants( get_self(), get_self().value );
    const auto grant = grants.get( grant_id.value, "pomelo::joinround: grant doesn't exist" );

    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr != rounds.end(),  get_self().to_string() + "::joinround: round doesn't exist" );
    check( round_itr->grant_ids.count( grant_id ) == 0, get_self().to_string() + "::joinround: grant already exists in this round");

    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.grant_ids.insert(grant_id);
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void pomelo::startround( const uint64_t round_id )
{
    require_auth( get_self() );

    state_table state(get_self(), get_self().value);
    auto state_ = state.get_or_default();
    state_.round_id = round_id;
    state.set(state_, get_self());
    if(round_id == 0) return;     //close round and return

    //make sure round exist and is not over
    pomelo::rounds_table rounds( get_self(), get_self().value );

    const auto round = rounds.get( round_id, "pomelo::startround: round is not defined" );
    const auto now = current_time_point().sec_since_epoch();
    check( round.end_at.sec_since_epoch() > now, get_self().to_string() + "::startround: round has already ended" );


}
