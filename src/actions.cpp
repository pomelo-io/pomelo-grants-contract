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

// @admin
[[eosio::action]]
void pomelo::unjoinround( const name grant_id, const uint64_t round_id )
{
    require_auth( get_self() );

    pomelo::rounds_table _rounds( get_self(), get_self().value );
    const auto round_itr = _rounds.find( round_id );
    check( round_itr != _rounds.end(),  "pomelo::unjoinround: [round_id] does not exist" );
    check( get_index(round_itr->grant_ids, grant_id ) != -1, "pomelo::unjoinround: grant does not exist in this round");

    // remove from match table
    pomelo::match_table _match( get_self(), round_id );
    const auto match_itr = _match.find( grant_id.value );
    if(match_itr == _match.end()) return;   //no donations yet made to this project during this round
    _match.erase( match_itr );

    // recalculate matchings for this round
    double sum_value = 0, sum_boost = 0, sum_square = 0;
    for(const auto& grant: _match) {
        sum_value += grant.sum_value;
        sum_boost += grant.sum_boost;
        sum_square += grant.square;
    }

    // remove from rounds table
    _rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.grant_ids = remove_element(row.grant_ids, grant_id);
        row.sum_value = sum_value;
        row.sum_boost = sum_boost;
        row.sum_square = sum_square;
        row.updated_at = current_time_point();
        // no easy way to update row.accepted_tokens since they were already converted to values
    });

    // update users table by removing all donations to this project
    pomelo::users_table _users( get_self(), round_id );
    auto users_itr = _users.begin();
    while(users_itr != _users.end()) {
        int index = get_index(users_itr->contributions, grant_id);
        if(index != -1) {
            if(users_itr->contributions.size() == 1){
                users_itr = _users.erase(users_itr);
                continue;
            }
            _users.modify( users_itr, get_self(), [&]( auto & row ) {
                const auto value = users_itr->contributions[index].value;
                const auto donated = value / ( users_itr->multiplier + 1);
                const auto boost = donated * users_itr->multiplier;
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

// @admin
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


// @admin
[[eosio::action]]
void pomelo::removeuser( const name user_id, const uint64_t round_id )
{
    require_auth( get_self() );

    // remove from users table
    pomelo::users_table _users( get_self(), round_id );
    auto user_itr = _users.find( user_id.value );
    check(user_itr != _users.end(), "pomelo::removeuser: no donations from [user_id] during [round_id]" );
    const auto user = *user_itr;
    _users.erase(user_itr);

    // update match table
    pomelo::match_table _match( get_self(), round_id );
    for(const auto grant: user.contributions){
        const auto match_itr = _match.find( grant.id.value );
        if(match_itr->total_users == 1){
            _match.erase(match_itr);
            continue;
        }
        _match.modify( match_itr, get_self(), [&]( auto & row ) {
            const auto donated = grant.value / (1 + user.multiplier);
            row.total_users--;
            row.sum_value -= donated;
            row.sum_boost -= donated * user.multiplier;
            row.sum_sqrt -= sqrt( grant.value );
            row.square = row.sum_sqrt * row.sum_sqrt;
            row.updated_at = current_time_point();
        });

    }

    // remove user_id and update sums in rounds table
    double sum_value = 0, sum_boost = 0, sum_square = 0;
    for(const auto& grant: _match) {
        sum_value += grant.sum_value;
        sum_boost += grant.sum_boost;
        sum_square += grant.square;
    }

    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr != rounds.end(),  "pomelo::removeuser: [round_id] does not exist" );
    check( get_index(round_itr->user_ids, user_id ) != -1, "pomelo::removeuser: grant does not exist in this round");

    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.user_ids = remove_element(row.user_ids, user_id);
        row.sum_value = sum_value;
        row.sum_boost = sum_boost;
        row.sum_square = sum_square;
        row.updated_at = current_time_point();
        // no easy way to update row.accepted_tokens since they were already converted to values
    });


}

[[eosio::action]]
void pomelo::collapse(set<name> user_ids, name user_id, uint64_t round_id)
{
    require_auth( get_self() );
    check(user_ids.count(user_id) == 0, "pomelo::collapse: [user_ids] cannot contain [user_id] itself" );

    pomelo::users_table _users( get_self(), round_id );
    pomelo::match_table _match( get_self(), round_id );
    pomelo::rounds_table _rounds( get_self(), get_self().value );
    auto round_itr = _rounds.find(round_id);
    auto user_itr = _users.find( user_id.value );
    check( round_itr != _rounds.end(),  "pomelo::collapse: [round_id] does not exist" );
    check( user_itr != _users.end(), "pomelo::collapse: no donations from [user_id] during [round_id]" );
    auto user = *user_itr;
    vector<name> round_users = round_itr->user_ids;

    int erased = 0;
    auto from_itr = _users.begin();
    while(from_itr != _users.end()){
        if(user_ids.count(from_itr->user_id)){
            for(const auto c: from_itr->contributions){
                const auto donated = c.value / (from_itr->multiplier + 1);
                const auto old_contribution = user.contributions[get_index(user.contributions, c.id)].value;
                user.contributions[get_index(user.contributions, c.id)].value += donated * (user.multiplier + 1);
                auto match_itr = _match.find(c.id.value);
                _match.modify( match_itr, get_self(), [&]( auto & row ) {
                    row.total_users--;
                    row.sum_boost += donated * (user.multiplier - from_itr->multiplier);
                    row.sum_sqrt += sqrt(user.contributions[get_index(user.contributions, c.id)].value) - sqrt(donated * (from_itr->multiplier + 1)) - sqrt(old_contribution);
                    row.square = row.sum_sqrt * row.sum_sqrt;
                    row.updated_at = current_time_point();
                });
            }
            user.value += from_itr->value;
            user.boost += from_itr->value * user.multiplier;
            round_users = remove_element(round_users, from_itr->user_id);
            from_itr = _users.erase(from_itr);
            erased++;
            continue;
        }
        ++from_itr;
    }
    check(erased == user_ids.size(), "pomelo::collapse: [user_ids] contains user_ids that didn't take part in [round_id]: " + (*user_ids.begin()).to_string() );
    _users.modify( user_itr, get_self(), [&]( auto & row ) {
        row.contributions = user.contributions;
        row.value = user.value;
        row.boost = user.boost;
        row.updated_at = current_time_point();
    });

    // recalculate all matched amounts for this round
    double round_sum_boost = 0, round_sum_square = 0;
    for(const auto& grant: _match){
        round_sum_boost += grant.sum_boost;
        round_sum_square += grant.square;
    }
     _rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.sum_boost = round_sum_boost;
        row.sum_square = round_sum_square;
        row.user_ids = round_users;
        row.updated_at = current_time_point();
    });

}
