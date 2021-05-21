#include <eosn-login-contract/login.eosn.hpp>
#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>
#include "pomelo.hpp"

#include "src/config.cpp"
#include "src/utils.cpp"
#include "src/actions.cpp"
#include "src/getters.cpp"
#include "src/notifiers.cpp"


template <typename T>
void pomelo::fund_project(const T& table, const name project_id, const extended_asset ext_quantity, const name user, const string& memo)
{
    const auto project = table.get(project_id.value, "pomelo::fund_project: project not found");

    check(project.status == "ok"_n, get_self().to_string() + "::fund_project: project not available for funding");
    check(project.accepted_tokens.count(ext_quantity.get_extended_symbol()), get_self().to_string() + "::fund_project: not accepted tokens for this project");

    const auto value = get_value( ext_quantity );

    print("Funding ", project_id, " with ", ext_quantity, " == ", value, " value");

    if( project.type == "grant"_n){
        const auto user_id = get_user_id( user );
        fund_grant( project_id, ext_quantity, user_id, value );
    }

    log_transfer( project.id, user, ext_quantity, value, memo );

    eosio::token::transfer_action transfer(ext_quantity.contract, { get_self(), "active"_n });
    transfer.send( get_self(), project.funding_account, ext_quantity.quantity, "Funded through Pomelo!");
}

void pomelo::fund_grant(const name grant_id, const extended_asset ext_quantity, const name user_id, const double value)
{
    const auto round_id = get_current_round( );
    check(round_id > 0, get_self().to_string() + "::fund_grant: no funding round ongoing");

    // get round
    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr->grant_ids.count( grant_id ), get_self().to_string() + "::fund_grant: grant is not part of this funding round");

    // update project matching records
    const auto multiplier = get_user_boost_mutliplier( user_id );
    const auto boost = multiplier * value;  // boost by 0-125% based on socials and other boosters

    pomelo::match_grant_table match( get_self(), round_id );
    const auto match_itr = match.find( grant_id.value );
    const auto old_user_sqrt = match_itr->user_sqrt.count(user_id) ? match_itr->user_sqrt.at(user_id) : 0;
    const auto old_square = match_itr->square;
    double new_square = 0;

    auto insert = [&]( auto & row ) {
        row.grant_id = grant_id;
        row.round_id = round_id;
        row.user_multiplier[user_id] = multiplier;
        row.user_value[user_id] += value;
        row.user_boost[user_id] += boost;
        row.user_sqrt[user_id] = sqrt( old_user_sqrt * old_user_sqrt + value + boost );
        row.total_users = row.user_value.size();
        row.sum_value += value;
        row.sum_boost += boost;
        row.sum_sqrt += row.user_sqrt[user_id] - old_user_sqrt;
        row.square = new_square = row.sum_sqrt * row.sum_sqrt;
        row.updated_at = current_time_point();
    };

    if ( match_itr == match.end() ) match.emplace( get_self(), insert );
    else match.modify( match_itr, get_self(), insert );

    //update round
    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        bool added = false;
        for(auto& quan: row.accepted_tokens){
            if(quan.get_extended_symbol() == ext_quantity.get_extended_symbol()){
                quan += ext_quantity;
                added = true;
            }
        }
        if(!added) row.accepted_tokens.push_back(ext_quantity);
        row.user_ids.insert( user_id );
        row.sum_value += value;
        row.sum_boost += boost;
        row.sum_square += new_square - old_square;
        row.updated_at = current_time_point();
    });

}


void pomelo::log_transfer(const name project_id, const name user, const extended_asset ext_quantity, const double value, const string& memo)
{
    const auto user_id = get_user_id( user );
    const auto round_id = get_current_round( );

    pomelo::transfers_table transfers( get_self(), get_self().value );
    transfers.emplace( get_self(), [&]( auto & row ) {
        row.transfer_id = transfers.available_primary_key();
        row.user_id = user_id;
        row.round_id = round_id;
        row.project_id = project_id;
        row.amount = ext_quantity;
        row.value = value;
        row.eos_account = user;
        row.trx_id = get_trx_id();
        row.memo = memo;
        row.created_at = current_time_point();
    });
}


template <typename T>
void pomelo::set_project( T& projects, const name type, const name id, const name author_id, const set<name> authorized_ids, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    check( is_account(funding_account), get_self().to_string() + "::set_project: funding account must exist on chain" );

    // make sure Pomelo users exist
    check( is_user( author_id ), get_self().to_string() + "::set_project: author doesn't exist" );
    for(const auto user_id: authorized_ids)
        check( is_user( user_id ), get_self().to_string() + "::set_project: authorized_id doesn't exist" );

    // create/update project
    const auto itr = projects.find( id.value );
    if(itr != projects.end()){
        check( type == itr->type, get_self().to_string() + "::set_project: project type cannot be changed" );
        check( author_id == itr->author_user_id, get_self().to_string() + "::set_project: project author cannot be changed" );
    }

    auto insert = [&]( auto & row ) {
        row.id = id;
        row.type = type;
        row.author_user_id = author_id;
        row.authorized_user_ids = authorized_ids;
        row.funding_account = funding_account;
        if( accepted_tokens.size() ) row.accepted_tokens = accepted_tokens;
        if( itr == projects.end() ) row.created_at = current_time_point();
        row.updated_at = current_time_point();
    };

    if ( itr == projects.end() ) projects.emplace( get_self(), insert );
    else projects.modify( itr, get_self(), insert );
}