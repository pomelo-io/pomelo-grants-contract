#include <eosn-login-contract/login.eosn.hpp>
#include <eosio.token/eosio.token.hpp>
#include <sx.utils/utils.hpp>
#include "pomelo.hpp"


#include "src/config.cpp"
#include "src/utils.cpp"
#include "src/getters.cpp"

namespace eosn {

/**
 * Notify contract when any token transfer notifiers relay contract
 */
[[eosio::on_notify("*::transfer")]]
void pomelo::on_transfer( const name from, const name to, const asset quantity, const string memo )
{
    // authenticate incoming `from` account
    require_auth( from );

    // config
    check( config.exists(), "pomelo: config does not exist" );
    const name status = config.get().status;
    check( (status == "ok"_n || status == "testing"_n ), "pomelo: contract is under maintenance");

    // ignore outgoing/RAM/self-funding transfers
    if ( to != get_self() || memo == get_self().to_string() || from == "eosio.ram"_n ) return;

    // parse memo
    const auto memo_parts = sx::utils::split(memo, ":");
    check(memo_parts.size() == 2, ERROR_INVALID_MEMO);

    const auto project = sx::utils::parse_name(memo_parts[1]);
    check(project.value, "pomelo: invalid project name");

    if(memo_parts[0] == "grant"){

        pomelo::grants_table grants( get_self(), get_self().value );
        fund_project(grants, project, extended_asset{ quantity, get_first_receiver() }, from);
    }
    else if(memo_parts[0] == "bounty"){

        pomelo::bounties_table bounties( get_self(), get_self().value );
        fund_project(bounties, project, extended_asset{ quantity, get_first_receiver() }, from);
    }
    else {
        check(false, ERROR_INVALID_MEMO);
    }

    check(false, "done");

}

template <typename T>
void pomelo::fund_project(const T& table, const name project_id, const extended_asset ext_quantity, const name user)
{
    const auto project = table.get(project_id.value, "pomelo: project not found");

    check(project.status == "ok"_n, "pomelo: project not available for funding");
    check(project.accepted_tokens.count(ext_quantity.get_extended_symbol()), "pomelo: not accepted tokens for this project");

    const auto value = get_value( ext_quantity );

    print("Funding ", project_id, " with ", ext_quantity, " == ", value, " value");

    log_transfer(project.id, user, ext_quantity, value);

    eosio::token::transfer_action transfer(ext_quantity.contract, { get_self(), "active"_n });
    transfer.send( get_self(), project.funding_account, ext_quantity.quantity, "Funded!");
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
        check( type == itr->type, "pomelo: project type cannot be changed" );
        check( author_id == itr->author_user_id, "pomelo: project author cannot be changed" );
    }
    check( is_account(funding_account), "pomelo: funding account must exist on chain" );

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
    check( status == "ok"_n || status == "pending"_n || status == "disabled"_n, "pomelo: invalid status" );

    auto modify = [&]( auto & row ) {
        row.status = status;
        row.updated_at = current_time_point();
    };

    pomelo::grants_table grants( get_self(), get_self().value );
    const auto itr1 = grants.find( project_id.value );
    if( itr1 != grants.end() ){
        check( itr1->status != status, "pomelo: status must be different");
        grants.modify( itr1, get_self(), modify);
        return;
    }

    pomelo::bounties_table bounties( get_self(), get_self().value );
    const auto itr2 = bounties.find( project_id.value );
    if( itr2 != bounties.end() ){
        check( itr2->status != status, "pomelo: status must be different");
        bounties.modify( itr2, get_self(), modify);
        return;
    }

    check( false, "pomelo: project doesn't exist" );
}

}