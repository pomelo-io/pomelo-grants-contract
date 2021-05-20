#include <eosn-login-contract/login.eosn.hpp>
#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>
#include "pomelo.hpp"


#include "src/config.cpp"
#include "src/utils.cpp"
#include "src/getters.cpp"

namespace eosn {

static const string ERROR_INVALID_MEMO = "invalid memo, use \"grant:mygrant\" or \"bounty:mybounty\"";

/**
 * Notify contract when any token transfer notifiers relay contract
 */
[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // config
    check( config.exists(), get_self().to_string() + "::on_transfer: config does not exist" );
    const name status = config.get().status;
    check( (status == "ok"_n || status == "testing"_n ), get_self().to_string() + ": contract is under maintenance");

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check(memo_parts.size() == 2, get_self().to_string() + "::on_transfer: " + ERROR_INVALID_MEMO);

    const auto project = sx::utils::parse_name(memo_parts[1]);
    check(project.value, get_self().to_string() + "::on_transfer: invalid project name");

    if(memo_parts[0] == "grant"){

        pomelo::grants_table grants( get_self(), get_self().value );
        fund_project(grants, project, extended_asset{ quantity, get_first_receiver() }, from);
    }
    else if(memo_parts[0] == "bounty"){

        pomelo::bounties_table bounties( get_self(), get_self().value );
        fund_project(bounties, project, extended_asset{ quantity, get_first_receiver() }, from);
    }
    else {
        check(false, get_self().to_string() + "::on_transfer: " + ERROR_INVALID_MEMO);
    }

    //check(false, "done");

}

template <typename T>
void pomelo::fund_project(const T& table, const name project_id, const extended_asset ext_quantity, const name user)
{
    const auto project = table.get(project_id.value, "pomelo::fund_project: project not found");

    check(project.status == "ok"_n, get_self().to_string() + "::fund_project: project not available for funding");
    check(project.accepted_tokens.count(ext_quantity.get_extended_symbol()), get_self().to_string() + "::fund_project: not accepted tokens for this project");

    if( project.type == "grant"_n){
        const auto user_id = get_user_id( user );
        fund_grant( project_id, ext_quantity, user_id );
    }

    const auto value = get_value( ext_quantity );

    print("Funding ", project_id, " with ", ext_quantity, " == ", value, " value");

    log_transfer(project.id, user, ext_quantity, value);

    eosio::token::transfer_action transfer(ext_quantity.contract, { get_self(), "active"_n });
    transfer.send( get_self(), project.funding_account, ext_quantity.quantity, "Funded!");
}

void pomelo::fund_grant(const name grant_id, const extended_asset ext_quantity, const name user_id)
{
    const auto round_id = get_current_round( );
    check(round_id > 0, get_self().to_string() + "::fund_grant: no funding round ongoing");

    // update round
    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check(round_itr != rounds.end() && round_itr->grant_ids.count( grant_id ), get_self().to_string() + "::fund_grant: grant is not part of this funding round");

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
        row.updated_at = current_time_point();
    });

    // do matching
    match_grant( grant_id, round_id, ext_quantity, user_id );
}


void pomelo::match_grant(const name grant_id, const uint64_t round_id, const extended_asset ext_quantity, const name user_id)
{
    // do matching
    const auto value = get_value( ext_quantity );
    const double multiplier = get_user_mutliplier( user_id );
    const double matched = multiplier * value;   //for now - match 150%
    const auto sqrt_value = sqrt(value);

    pomelo::match_grant_table match( get_self(), grant_id.value );
    const auto match_itr = match.find( round_id );

    auto insert = [&]( auto & row ) {
        row.round_id = round_id;
        row.grant_id = grant_id;
        row.user_value[user_id] += value;
        row.user_multiplier[user_id] = multiplier;  //what if user's multiplier changed between donations - keep last?
        row.user_match[user_id] += matched;
        row.user_sqrt[user_id] += sqrt_value;
        row.total_users = row.user_value.size();
        row.sum_value += value;
        row.sum_match += matched;
        row.sum_sqrt += sqrt_value;
        row.square = row.sum_sqrt * row.sum_sqrt;
        row.updated_at = current_time_point();
    };

    if ( match_itr == match.end() ) match.emplace( get_self(), insert );
    else match.modify( match_itr, get_self(), insert );
}

void pomelo::log_transfer(const name project_id, const name user, const extended_asset ext_quantity, const double value)
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
        row.created_at = current_time_point();
    });
}

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


template <typename T>
void pomelo::set_project( T& projects, const name type, const name id, const name author_id, const set<name> authorized_ids, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    const auto itr = projects.find( id.value );
    if(itr != projects.end()){
        check( type == itr->type, get_self().to_string() + "::set_project: project type cannot be changed" );
        check( author_id == itr->author_user_id, get_self().to_string() + "::set_project: project author cannot be changed" );
    }
    check( is_account(funding_account), get_self().to_string() + "::set_project: funding account must exist on chain" );

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
void pomelo::addgrant( const name grant_id, const uint64_t round_id )
{
    require_auth( get_self() );

    pomelo::grants_table grants( get_self(), get_self().value );
    const auto grant = grants.get( grant_id.value, "pomelo::addgrant: grant doesn't exist" );

    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr != rounds.end(),  get_self().to_string() + "::addgrant: round doesn't exist" );
    check( round_itr->grant_ids.count( grant_id ) == 0, get_self().to_string() + "::addgrant: grant already exists in this round");

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

}