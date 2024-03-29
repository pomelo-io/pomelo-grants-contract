#include <eosn.login/login.eosn.hpp>
#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>

#include "app.pomelo.hpp"

#include "src/utils.cpp"
#include "src/getters.cpp"
#include "src/actions.cpp"
#include "src/notifiers.cpp"

template <typename T>
void pomelo::donate_project(const T& table, const name project_id, const name from, const extended_asset ext_quantity, const string memo )
{
    const auto project = table.get(project_id.value, "pomelo::donate_project: project not found");

    const asset quantity = ext_quantity.quantity;
    const symbol_code symcode = quantity.symbol.code();
    const int64_t min_amount = get_token( ext_quantity ).min_amount;

    // validate incoming transfer
    check( quantity.amount >= min_amount, "pomelo::donate_project("+project_id.to_string()+"): " + ext_quantity.quantity.to_string() + " is less than min amount");
    check( project.status == "published"_n, "pomelo::donate_project("+project_id.to_string()+"): grant is not published");
    check( project.accepted_tokens.count(symcode), "pomelo::donate_project("+project_id.to_string()+"): " + symcode.to_string() + " is not accepted by this project");
    check( project.funding_account.value, "pomelo::donate_project("+project_id.to_string()+"): grant funding account is not set");
    check( is_token_enabled( symcode ), "pomelo::donate_project("+project_id.to_string()+"): token " + symcode.to_string() + " is disabled");

    // check sender is not self (prevent circular donations) only from funding account
    const name user_id = get_user_id( from );
    check( project.funding_account != from, "pomelo::donate_project("+project_id.to_string()+"): sender is the same as grant funding account");
    check( project.author_user_id != user_id, "pomelo::donate_project("+project_id.to_string()+"): sender is linked to grant author");
    check( project.author_user_id != from, "pomelo::donate_project("+project_id.to_string()+"): sender is the same as grant author id");

    // calculate fee
    const extended_asset fee = calculate_fee( ext_quantity );
    double value = calculate_value( ext_quantity - fee );
    print("\n", ext_quantity - fee, " == ", value);

    // track for matching bonus
    if ( project.type == "grant"_n ) {
        donate_grant( project_id, ext_quantity - fee, user_id, value );
    }

    // transfer quantity to funding account & system fee
    transfer( get_self(), project.funding_account, ext_quantity - fee, "🍈 " + project_id.to_string() + " donation received via Pomelo");
    const name fee_account = get_globals().fee_account;
    if ( is_account( fee_account ) && fee.quantity.amount > 0 ) {
        transfer( get_self(), fee_account, fee, "🍈 " + project_id.to_string() + " re-allocation to next Pomelo season");
    }

    // save for logging
    save_transfer( from, project.funding_account, ext_quantity, fee.quantity, memo, project.type, project.id, value );
}

void pomelo::donate_grant(const name grant_id, const extended_asset ext_quantity, const name user_id, const double value )
{
    const auto round_id = get_active_round( grant_id );
    check(round_id != 0, "pomelo::donate_grant("+grant_id.to_string()+"): has not joined active rounds");

    validate_round( round_id );

    // get round
    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    const symbol_code symcode = ext_quantity.quantity.symbol.code();
    check( get_index(round_itr->grant_ids, grant_id) != -1, "pomelo::donate_grant("+grant_id.to_string()+"): has not joined the matching round");

    // update project matching records
    const auto weight = eosn::login::get_user_weight( user_id, get_globals().login_contract );
    const double multiplier = static_cast<double>( weight ) / 100;
    const auto boost = value * multiplier;  // boost based on socials and other boosters

    pomelo::users_table _users( get_self(), round_id );
    const auto users_itr = _users.find( user_id.value );

    pomelo::match_table _match( get_self(), round_id );
    const auto match_itr = _match.find( grant_id.value );
    const auto old_square = match_itr->square;
    double new_square = 0;
    double contribution = 0, old_contribution = 0;

    auto insert_user = [&]( auto & row ) {
        row.user_id = user_id;
        row.multiplier = multiplier;
        const auto index = get_index(row.contributions, grant_id);
        old_contribution = index == -1 ? 0 : row.contributions[index].value;
        contribution = old_contribution + value + boost;
        row.value += value;
        row.boost += boost;
        if(index == -1) row.contributions.push_back({ grant_id, contribution });
        else row.contributions[index].value = contribution;
        row.updated_at = current_time_point();
    };

    if ( users_itr == _users.end() ) _users.emplace( get_self(), insert_user );
    else _users.modify( users_itr, get_self(), insert_user );

    auto insert_match = [&]( auto & row ) {
        row.grant_id = grant_id;
        row.round_id = round_id;
        row.total_users += old_contribution == 0 ? 1 : 0;
        row.sum_value += value;
        row.sum_boost += boost;
        row.sum_sqrt += sqrt(contribution) - sqrt(old_contribution);
        row.square = new_square = row.sum_sqrt * row.sum_sqrt;
        row.updated_at = current_time_point();
    };

    if ( match_itr == _match.end() ) _match.emplace( get_self(), insert_match );
    else _match.modify( match_itr, get_self(), insert_match );

    // update round
    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        // TO-DO refactor adding extended asset into dedicated method
        // row.donated_tokens = sum_extended_asset( row.donated_tokens, ext_quantity );
        bool added = false;
        for ( extended_asset & donated_token : row.donated_tokens ) {
            if ( donated_token.get_extended_symbol() == ext_quantity.get_extended_symbol() ) {
                donated_token += ext_quantity;
                added = true;
            }
        }
        if (!added) row.donated_tokens.push_back(ext_quantity);
        if( get_index(row.user_ids, user_id) == -1) row.user_ids.push_back(user_id);
        row.sum_value += value;
        row.sum_boost += boost;
        row.sum_square += new_square - old_square;
        row.updated_at = current_time_point();
    });
}

void pomelo::save_transfer( const name from, const name to, const extended_asset ext_quantity, const asset fee, const string& memo, const name project_type, const name project_id, const double value )
{
    const auto user_id = get_user_id( from );
    const auto round_id = project_type == "grant"_n ? get_active_round( project_id ) : 0;
    const auto season_id = get_globals().season_id;

    pomelo::transfers_table transfers( get_self(), get_self().value );
    transfers.emplace( get_self(), [&]( auto & row ) {
        row.transfer_id = transfers.available_primary_key();
        row.from = from;
        row.to = to;
        row.ext_quantity = ext_quantity;
        row.fee = fee;
        row.memo = memo;
        row.user_id = user_id;
        row.season_id = season_id;
        row.round_id = round_id;
        row.project_type = project_type;
        row.project_id = project_id;
        row.value = value;
        row.trx_id = get_trx_id();
        row.created_at = current_time_point();
    });
}

template <typename T>
void pomelo::set_project( T& projects, const name project_type, const name project_id, const name author_id, const name funding_account, const set<symbol_code> accepted_tokens )
{
    // create/update project
    const auto itr = projects.find( project_id.value );
    if (itr != projects.end()) {
        check( project_type == itr->type, "pomelo::set_project: project [type] cannot be modified" );
        check( author_id == itr->author_user_id, "pomelo::set_project: project [author_id] cannot be modifed" );
        check( itr->funding_account.value == 0 || funding_account == itr->funding_account, "pomelo::set_project: existing [funding_account] cannot be modifed" );
        check( is_account(funding_account) || (project_type == "bounty"_n && funding_account.value == 0), "pomelo::set_project: [funding_account] does not exists" );
    }
    else {  // new project
        if ( project_type == "bounty"_n ) check( funding_account.value == 0, "pomelo::set_project: [funding_account] must be empty for bounties" );
        else {
            check( is_account(funding_account), "pomelo::set_project: [funding_account] does not exists" );
            //reinstantiate table - compiler fails if we just use [projects] here
            pomelo::grants_table grants( get_self(), get_self().value );
            auto byauthor = grants.get_index<"byauthor"_n>();
            int active = 0;
            for( auto itr1 = byauthor.lower_bound(author_id.value); itr1 != byauthor.end() && itr1->author_user_id == author_id; ++itr1){
                if( itr1->status == "pending"_n || itr1->status == "published"_n) active++;
                check(active < MAX_GRANTS_PER_AUTHOR, "pomelo::set_project: limit of active grants per author reached");
            }
        }
    }

    for ( const symbol_code accepted_token : accepted_tokens ) {
        check( is_token_enabled( accepted_token ), "pomelo::set_project: [accepted_token=" + accepted_token.to_string() +"] token is not available" );
    }

    auto insert = [&]( auto & row ) {
        row.id = project_id;
        row.type = project_type;
        row.author_user_id = author_id;
        row.funding_account = funding_account;
        if ( accepted_tokens.size() ) row.accepted_tokens = accepted_tokens;
        check( accepted_tokens.size(), "pomelo::set_project: [accepted_tokens] cannot be empty");
        if ( itr == projects.end() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    if ( itr == projects.end() ) projects.emplace( get_self(), insert );
    else projects.modify( itr, get_self(), insert );
}

int pomelo::get_index(const vector<name>& vec, name value)
{
    for(int i = 0; i < vec.size(); i++){
        if(vec[i] == value ) return i;
    }
    return -1;
}

int pomelo::get_index(const vector<contribution_t>& vec, name id)
{
    for(int i = 0; i < vec.size(); i++){
        if(vec[i].id == id ) return i;
    }
    return -1;
}

int pomelo::get_index(const vector<uint16_t>& vec, uint16_t value)
{
    for(int i = 0; i < vec.size(); i++){
        if(vec[i] == value ) return i;
    }
    return -1;
}

template <typename T>
void pomelo::clear_table( T& table, uint64_t rows_to_clear )
{
    auto itr = table.begin();
    while ( itr != table.end() && rows_to_clear-- ) {
        itr = table.erase( itr );
    }
}

template <typename T>
vector<T> pomelo::remove_element(const vector<T>& vec, T id)
{
    vector<T> res;
    for(const auto e: vec){
        if(e != id) res.push_back(e);
    }
    return res;
}


template <typename T>
vector<T> pomelo::remove_elements(const vector<T>& vec, const vector<T>& ids)
{
    vector<T> res;
    for(const auto id: vec){
        if(get_index(ids, id) == -1) res.push_back(id);
    }
    return res;
}

void pomelo::update_status( const uint32_t index, const uint32_t count )
{
    status_table _status( get_self(), get_self().value );
    auto status = _status.get_or_default();

    if ( status.counters.size() <= index ) status.counters.resize( index + 1);
    status.counters[index] += count;
    status.last_updated = current_time_point();
    _status.set( status, get_self() );
}