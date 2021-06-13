#include <eosn.login/login.eosn.hpp>
#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>
#include "pomelo.hpp"

#include "src/utils.cpp"
#include "src/actions.cpp"
#include "src/getters.cpp"
#include "src/notifiers.cpp"

template <typename T>
void pomelo::donate_project(const T& table, const name project_id, const name from, const name to, const extended_asset ext_quantity, const string memo )
{
    const auto project = table.get(project_id.value, "pomelo::donate_project: project not found");

    check(project.status == "ok"_n, "pomelo::donate_project: project not available for donation");
    check(project.accepted_tokens.count(ext_quantity.get_extended_symbol()), "pomelo::donate_project: not acceptable tokens for this project");

    const auto value = calculate_value( ext_quantity );

    print("Donate ", project_id, " with ", ext_quantity, " == ", value, " value");

    if ( project.type == "grant"_n ) {
        const auto user_id = get_user_id( from );
        donate_grant( project_id, ext_quantity, user_id, value );
    }

    save_transfer( from, to, ext_quantity, memo, project.type, project.id, value );

    eosio::token::transfer_action transfer(ext_quantity.contract, { get_self(), "active"_n });
    transfer.send( get_self(), project.funding_account, ext_quantity.quantity, "🍈 " + memo + " donation via pomelo.io");
}

void pomelo::donate_grant(const name grant_id, const extended_asset ext_quantity, const name user_id, const double value )
{
    const auto round_id = get_key_value( "roundid"_n );
    validate_round( round_id );

    // get round
    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( get_index(round_itr->grant_ids, grant_id) != -1, "pomelo::donate_grant: [grant_id] has not joined current matching round");

    // update project matching records
    const auto multiplier = get_user_boost_mutliplier( user_id );
    const auto boost = multiplier * value;  // boost by 0-125% based on socials and other boosters

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

    //update round
    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        bool added = false;
        for ( auto& accepted_token : row.accepted_tokens ) {
            if ( accepted_token.get_extended_symbol() == ext_quantity.get_extended_symbol() ) {
                accepted_token += ext_quantity;
                added = true;
            }
        }
        if (!added) row.accepted_tokens.push_back(ext_quantity);
        if( get_index(row.user_ids, user_id) == -1) row.user_ids.push_back(user_id);
        row.sum_value += value;
        row.sum_boost += boost;
        row.sum_square += new_square - old_square;
        row.updated_at = current_time_point();
    });
}

void pomelo::save_transfer( const name from, const name to, const extended_asset ext_quantity, const string& memo, const name project_type, const name project_id, const double value )
{
    const auto user_id = get_user_id( from );
    const auto round_id = get_key_value( "roundid"_n );

    pomelo::transfers_table transfers( get_self(), get_self().value );
    transfers.emplace( get_self(), [&]( auto & row ) {
        row.transfer_id = transfers.available_primary_key();
        row.from = from;
        row.to = to;
        row.ext_quantity = ext_quantity;
        row.memo = memo;
        row.user_id = user_id;
        row.round_id = round_id;
        row.project_type = project_type;
        row.project_id = project_id;
        row.value = value;
        row.trx_id = get_trx_id();
        row.created_at = current_time_point();
    });
}

template <typename T>
void pomelo::set_project( T& projects, const name project_type, const name project_id, const name author_id, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    // create/update project
    const auto itr = projects.find( project_id.value );
    if (itr != projects.end()) {
        check( project_type == itr->type, "pomelo::set_project: project [type] cannot be modified" );
        check( author_id == itr->author_user_id, "pomelo::set_project: project [author_id] cannot be modifed" );
    }

    auto insert = [&]( auto & row ) {
        row.id = project_id;
        row.type = project_type;
        row.author_user_id = author_id;
        row.funding_account = funding_account;
        if ( accepted_tokens.size() ) row.accepted_tokens = accepted_tokens;
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

template <typename T>
void pomelo::clear_table( T& table )
{
    auto itr = table.begin();
    while ( itr != table.end() ) {
        itr = table.erase( itr );
    }
}

template <typename T>
vector<T> pomelo::remove_element(const vector<T>& vec, name id)
{
    vector<T> res;
    for(const auto e: vec){
        if(e != id) res.push_back(e);
    }
    return res;
}