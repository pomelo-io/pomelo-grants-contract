#include <sx.defibox/defibox.hpp>

// @admin
[[eosio::action]]
void pomelo::token( const symbol sym, const name contract, const uint64_t min_amount, const uint64_t pair_id )
{
    // authenticate
    require_auth( get_self() );

    pomelo::tokens_table tokens( get_self(), get_self().value );

    const asset supply = token::get_supply( contract, sym.code() );
    check( supply.symbol == sym, "pomelo::token: [sym] symbol does not match with token supply");
    check( supply.amount, "pomelo::token: [sym] has no supply");

    // check if Defibox pair ID exists
    if ( is_account( defibox::code ) && extended_symbol{ sym, contract } != VALUE_SYM ) defibox::get_reserves( pair_id, sym );

    const auto insert = [&]( auto & row ) {
        row.sym = sym;
        row.contract = contract;
        row.min_amount = min_amount;
        row.pair_id = pair_id;
    };

    const auto itr = tokens.find( sym.code().raw() );
    if ( itr == tokens.end() ) tokens.emplace( get_self(), insert );
    else tokens.modify( itr, get_self(), insert );
}

// @admin
[[eosio::action]]
void pomelo::deltoken( const symbol_code symcode )
{
    // authenticate
    require_auth( get_self() );

    pomelo::tokens_table tokens( get_self(), get_self().value );
    const auto & itr = tokens.get( symcode.raw(), "pomelo::deltoken: [symcode] token not found" );
    tokens.erase( itr );
}

// @user
[[eosio::action]]
void pomelo::setproject( const name author_id, const name project_type, const name project_id, const name funding_account, const set<symbol_code> accepted_tokens )
{
    eosn::login::require_auth_user_id( author_id, get_globals().login_contract );

    // tables
    pomelo::grants_table grants( get_self(), get_self().value );
    pomelo::bounties_table bounties( get_self(), get_self().value );

    // set project
    if ( project_type == "grant"_n ) set_project( grants, "grant"_n, project_id, author_id, funding_account, accepted_tokens );
    else if ( project_type == "bounty"_n ) set_project( bounties, "bounty"_n, project_id, author_id, funding_account, accepted_tokens );
    else check( false, "pomelo::setproject: invalid [project_type]");
}

// @user
[[eosio::action]]
void pomelo::joinround( const name grant_id, const uint16_t round_id )
{
    // authenticate user
    pomelo::grants_table grants( get_self(), get_self().value );
    const auto grant = grants.get( grant_id.value, "pomelo::joinround: [grant_id] does not exist" );
    eosn::login::require_auth_user_id( grant.author_user_id, get_globals().login_contract );

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
void pomelo::unjoinround( const name grant_id, const uint16_t round_id )
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
    for (const auto& grant: _match) {
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
    while (users_itr != _users.end()) {
        int index = get_index(users_itr->contributions, grant_id);
        if (index != -1) {
            if (users_itr->contributions.size() == 1) {
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
    auto & itr = table.get( project_id.value, "pomelo::enable_project: [project_id] does not exist");
    // eosn::login::require_auth_user_id( itr.author_user_id );

    table.modify( itr, get_self(), [&]( auto & row ) {
        check( row.status != status, "pomelo::enable_project: status was not modified");
        row.status = status;
        row.updated_at = current_time_point();
    });
}

// @admin
[[eosio::action]]
void pomelo::setround( const uint16_t round_id, const time_point_sec start_at, const time_point_sec end_at, const string description, const vector<extended_asset> match_tokens )
{
    require_auth( get_self() );

    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto itr = rounds.find( round_id );

    // validate input
    check( end_at >= start_at, "pomelo::setround: [end_at] must be after [start_at]");
    check( end_at.sec_since_epoch() - start_at.sec_since_epoch() >= DAY * 7, "pomelo::setround: minimum period must be at least 7 days");
    for ( const extended_asset match_token : match_tokens ) {
        check( is_token_enabled( match_token.quantity.symbol.code() ), "pomelo::setround: [match_token=" + match_token.quantity.to_string() +"] token is not available" );
    }

    const auto insert = [&]( auto & row ) {
        row.round = round_id;
        row.description = description;
        row.start_at = start_at;
        row.end_at = end_at;
        row.match_tokens = match_tokens;
        row.updated_at = current_time_point();
        if( itr == rounds.end() ) row.created_at = current_time_point();
    };

    if ( itr == rounds.end() ) rounds.emplace( get_self(), insert );
    else rounds.modify( itr, get_self(), insert );
}

// @admin
[[eosio::action]]
void pomelo::setconfig( const optional<uint16_t> round_id, const optional<uint64_t> grant_fee, const optional<uint64_t> bounty_fee, const optional<name> login_contract, const optional<name> fee_account )
{
    require_auth( get_self() );

    pomelo::globals_table _globals( get_self(), get_self().value );
    auto globals = _globals.get_or_default();

    if ( round_id ) globals.round_id = *round_id;
    if ( grant_fee ) globals.grant_fee = *grant_fee;
    if ( bounty_fee ) globals.bounty_fee = *bounty_fee;
    if ( login_contract ) globals.login_contract = *login_contract;
    if ( fee_account ) globals.fee_account = *fee_account;
    _globals.set( globals, get_self() );
}

// @admin
[[eosio::action]]
void pomelo::cleartable( const name table_name, const optional<uint16_t> round_id, const optional<uint64_t> max_rows )
{
    require_auth( get_self() );
    const uint64_t rows_to_clear = *max_rows == 0 ? -1 : *max_rows;
    const uint64_t scope = round_id ? *round_id : get_self().value;

    // tables
    pomelo::transfers_table transfers( get_self(), scope );
    pomelo::rounds_table rounds( get_self(), scope );
    pomelo::match_table match( get_self(), scope );
    pomelo::bounties_table bounties( get_self(), scope );
    pomelo::grants_table grants( get_self(), scope );
    pomelo::globals_table globals( get_self(), scope );
    pomelo::tokens_table tokens( get_self(), scope );
    pomelo::status_table status( get_self(), scope );

    if (table_name == "transfers"_n) clear_table( transfers, rows_to_clear );
    else if (table_name == "rounds"_n) clear_table( rounds, rows_to_clear );
    else if (table_name == "match"_n) clear_table( match, rows_to_clear );
    else if (table_name == "bounties"_n) clear_table( bounties, rows_to_clear );
    else if (table_name == "grants"_n) clear_table( grants, rows_to_clear );
    else if (table_name == "tokens"_n) clear_table( tokens, rows_to_clear );
    else if (table_name == "globals"_n) globals.remove();
    else if (table_name == "status"_n) status.remove();
    // else if (table_name == "global"_n) global.remove();
    else check(false, "pomelo::cleartable: [table_name] unknown table to clear" );
}


// @admin
[[eosio::action]]
void pomelo::removeuser( const name user_id, const uint16_t round_id )
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
    for (const auto grant: user.contributions) {
        const auto match_itr = _match.find( grant.id.value );
        if (match_itr->total_users == 1) {
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
void pomelo::collapse(set<name> user_ids, name user_id, uint16_t round_id)
{
    require_auth( get_self() );
    check(user_ids.count(user_id) == 0, "pomelo::collapse: [user_ids] cannot contain [user_id] itself" );

    pomelo::users_table _users( get_self(), round_id );
    pomelo::match_table _match( get_self(), round_id );
    pomelo::rounds_table _rounds( get_self(), get_self().value );
    auto user = _users.get( user_id.value, "pomelo::collapse: no donations from [user_id] during [round_id]" );
    auto round_itr = _rounds.find(round_id);
    check( round_itr != _rounds.end(),  "pomelo::collapse: [round_id] does not exist" );
    vector<name> round_users = round_itr->user_ids;

    for(const auto& user_to_erase: user_ids) {
        auto erase_itr = _users.find(user_to_erase.value);
        check(erase_itr != _users.end(), "pomelo::collapse: [user_ids] contains user_ids that didn't take part in [round_id]: " + (*user_ids.begin()).to_string() );
        for(const auto c: erase_itr->contributions){
            const auto donated = c.value / (erase_itr->multiplier + 1);
            const auto old_contribution = user.contributions[get_index(user.contributions, c.id)].value;
            user.contributions[get_index(user.contributions, c.id)].value += donated * (user.multiplier + 1);
            auto match_itr = _match.find(c.id.value);
            _match.modify( match_itr, get_self(), [&]( auto & row ) {
                row.total_users--;
                row.sum_boost += donated * (user.multiplier - erase_itr->multiplier);
                row.sum_sqrt += sqrt(user.contributions[get_index(user.contributions, c.id)].value) - sqrt(donated * (erase_itr->multiplier + 1)) - sqrt(old_contribution);
                row.square = row.sum_sqrt * row.sum_sqrt;
                row.updated_at = current_time_point();
            });
        }
        user.value += erase_itr->value;
        user.boost += erase_itr->value * user.multiplier;
        round_users = remove_element(round_users, user_to_erase);
        _users.erase(erase_itr);
    }
    const auto user_itr = _users.find( user_id.value );
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

void pomelo::transfer( const name from, const name to, const extended_asset value, const string memo )
{
    eosio::token::transfer_action transfer( value.contract, { from, "active"_n });
    transfer.send( from, to, value.quantity, memo );
}