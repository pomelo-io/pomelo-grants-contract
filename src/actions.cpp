// @user
[[eosio::action]]
void pomelo::setproject( const name author_id, const name project_type, const name project_id, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    // authenticate
    require_auth( get_self() );
    eosn::login::require_auth_user_id( author_id );

    // tables
    pomelo::grants_table grants( get_self(), get_self().value );
    pomelo::bounties_table bounties( get_self(), get_self().value );

    // validate input
    check( is_account(funding_account), "pomelo::set_project: [funding_account] does not exists" );

    // set project
    if ( project_type == "grant"_n ) set_project( grants, "grant"_n, project_id, author_id, funding_account, accepted_tokens );
    else if ( project_type == "bounty"_n ) set_project( bounties, "bounty"_n, project_id, author_id, funding_account, accepted_tokens );
    else check( false, "pomelo::enable: invalid [project_type]");
}

// @user
[[eosio::action]]
void pomelo::joinround( const name grant_id, const uint64_t round_id )
{
    require_auth( get_self() );

    // authenticate user
    pomelo::grants_table grants( get_self(), get_self().value );
    const auto grant = grants.get( grant_id.value, "pomelo::joinround: [grant_id] does not exist" );
    eosn::login::require_auth_user_id( grant.author_user_id );

    // join round
    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr != rounds.end(),  "pomelo::joinround: [round_id] does not exist" );
    check( get_index(round_itr->grant_ids, grant_id ) == -1, "pomelo::joinround: grant already exists in this round");

    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.grant_ids.push_back(grant_id);
        row.updated_at = current_time_point();
    });
}

// @user
[[eosio::action]]
void pomelo::unjoinround( const name grant_id, const uint64_t round_id )
{
    require_auth( get_self() );

    print("\nHi111");
    // remove from rounds table
    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr != rounds.end(),  "pomelo::unjoinround: [round_id] does not exist" );
    check( get_index(round_itr->grant_ids, grant_id ) != -1, "pomelo::unjoinround: grant does not exist in this round");

    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        vector<name> grants;
        for(const auto grant: row.grant_ids)
            if(grant != grant_id) grants.push_back(grant);
        row.grant_ids = grants;
        // what do we do with user_ids vector?
        row.updated_at = current_time_point();
    });

    // remove from match table
    pomelo::match_table _match( get_self(), round_id );
    const auto match_itr = _match.find( grant_id.value );
    if(match_itr == _match.end()) return;   //no donations yet made
    _match.erase( match_itr );
    print("\nHi");

    // update users table
    pomelo::users_table _users( get_self(), round_id );
    auto users_itr = _users.begin();
    while(users_itr != _users.end()) {
        print("\n ", users_itr->user_id);
        int index = get_index(users_itr->contributions, grant_id);
        if(index != -1) {
            if(users_itr->contributions.size() == 1){
                users_itr = _users.erase(users_itr);
                continue;
            }
            _users.modify( users_itr, get_self(), [&]( auto & row ) {
                const auto value = users_itr->contributions[index].value;
                const auto donated = value / ( users_itr->multiplier + 1);
                const auto boost = donated * users_itr->Fmultiplier;
                vector<contribution_t> vec;
                for(const auto c: users_itr->contributions)
                    if(c.id != grant_id) vec.push_back(c);
                row.contributions = vec;
                row.value -= donated;
                row.boost -= boost;
                row.updated_at = current_time_point();
            });
        }
        ++users_itr;
    }
}

// @user
[[eosio::action]]
void pomelo::enable( const name project_type, const name project_id, const name status )
{
    require_auth( get_self() );

    // tables
    pomelo::grants_table _grants( get_self(), get_self().value );
    pomelo::bounties_table _bounties( get_self(), get_self().value );

    // validate
    check( status == "ok"_n || status == "pending"_n || status == "disabled"_n, "pomelo::enable: invalid [status]" );

    if ( project_type == "grant"_n ) enable_project( _grants, project_id, status );
    else if ( project_type == "bounty"_n ) enable_project( _bounties, project_id, status );
    else check( false, "pomelo::enable: invalid [project_type]");
}

template <typename T>
void pomelo::enable_project( T& table, const name project_id, const name status )
{
    const auto & itr = table.get( project_id.value, "pomelo::enable_project: [project_id] does not exist");
    table.modify( itr, get_self(), [&]( auto & row ) {
        // authenticate user
        eosn::login::require_auth_user_id( row.author_user_id );
        check( row.status != status, "pomelo::enable_project: status was not modified");
        row.status = status;
        row.updated_at = current_time_point();
    });
}

// @admin
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

// @admin
[[eosio::action]]
void pomelo::init( const uint64_t round_id, const uint64_t status )
{
    require_auth( get_self() );

    set_key_value("round.id"_n, round_id );
    set_key_value("status"_n, status );
}


// @admin
[[eosio::action]]
void pomelo::cleartable( const name table_name )
{
    require_auth( get_self() );

    if(table_name == "transfers"_n){
        pomelo::transfers_table table( get_self(), get_self().value );
        clear_table( table );
    }
    else {
        check(false, "pomelo::cleartable: [table_name] clearing not allowed" );
    }
}