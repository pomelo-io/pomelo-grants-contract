#include <math.h>
#include <gems.random/random.gems.hpp>

#include "play.pomelo.hpp"

namespace pomelo {

void playtoken::create( const name&   issuer,
                        const asset&  maximum_supply )
{
    require_auth( get_self() );

    auto sym = maximum_supply.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( maximum_supply.is_valid(), "invalid supply");
    check( maximum_supply.amount > 0, "max-supply must be positive");

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing == statstable.end(), "token with symbol already exists" );

    statstable.emplace( get_self(), [&]( auto& s ) {
        s.supply.symbol = maximum_supply.symbol;
        s.max_supply    = maximum_supply;
        s.issuer        = issuer;
    });
}

void playtoken::issue( const name& to, const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist, create token before issue" );
    const auto& st = *existing;
    check( to == st.issuer, "tokens can only be issued to issuer account" );

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");

    statstable.modify( st, same_payer, [&]( auto& s ) {
        s.supply += quantity;
    });

    add_balance( st.issuer, quantity, st.issuer );
}

void playtoken::retire( const asset& quantity, const string& memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    stats statstable( get_self(), sym.code().raw() );
    auto existing = statstable.find( sym.code().raw() );
    check( existing != statstable.end(), "token with symbol does not exist" );
    const auto& st = *existing;

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );

    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    statstable.modify( st, same_payer, [&]( auto& s ) {
        s.supply -= quantity;
    });

    sub_balance( st.issuer, quantity );
}

void playtoken::transfer( const name&    from,
                          const name&    to,
                          const asset&   quantity,
                          const string&  memo )
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );

    // only pomelo accounts can transfer to defibox
    if ( from.suffix() != "pomelo"_n ) check( to != "swap.defi"_n, "cannot transfer to defibox");

    check( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    stats statstable( get_self(), sym.raw() );
    const auto& st = statstable.get( sym.raw() );

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void playtoken::sub_balance( const name& owner, const asset& value ) {
    accounts from_acnts( get_self(), owner.value );

    const auto& from = from_acnts.get( value.symbol.code().raw(), "no balance object found" );
    check( from.balance.amount >= value.amount, "overdrawn balance" );

    from_acnts.modify( from, owner, [&]( auto& a ) {
        a.balance -= value;
    });
}

void playtoken::add_balance( const name& owner, const asset& value, const name& ram_payer )
{
   accounts to_acnts( get_self(), owner.value );
   auto to = to_acnts.find( value.symbol.code().raw() );
   if ( to == to_acnts.end() ) {
        to_acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = value;
        });
    } else {
        to_acnts.modify( to, same_payer, [&]( auto& a ) {
            a.balance += value;
        });
    }
}

void playtoken::open( const name& owner, const symbol& symbol, const name& ram_payer )
{
    require_auth( ram_payer );

    check( is_account( owner ), "owner account does not exist" );

    auto sym_code_raw = symbol.code().raw();
    stats statstable( get_self(), sym_code_raw );
    const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
    check( st.supply.symbol == symbol, "symbol precision mismatch" );

    accounts acnts( get_self(), owner.value );
    auto it = acnts.find( sym_code_raw );
    if ( it == acnts.end() ) {
        acnts.emplace( ram_payer, [&]( auto& a ){
            a.balance = asset{0, symbol};
        });
    }
}

void playtoken::close( const name& owner, const symbol& symbol )
{
    if ( !has_auth( get_self() )) require_auth( owner );
    accounts acnts( get_self(), owner.value );
    auto it = acnts.find( symbol.code().raw() );
    check( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );

    // allow closing balance with greater than 0 balance
    if ( it->balance.amount > 0 ) {
        stats statstable( get_self(), symbol.code().raw() );
        const auto& st = statstable.get( symbol.code().raw(), "symbol does not exist" );
        statstable.modify( st, same_payer, [&]( auto& s ) {
            s.supply -= it->balance;
        });
    }
    acnts.erase( it );
}

void playtoken::faucet( const name owner, const symbol_code sym_code )
{
    if ( !has_auth( get_self() )) require_auth( owner );

    // only allowed on empty balance
    accounts acnts( get_self(), owner.value );
    auto it = acnts.find( sym_code.raw() );
    if ( it != acnts.end() ) check( it->balance.amount <= 0, "faucet already used for this account");

    // random amount of tokens
    const asset supply = get_supply( get_self(), sym_code );
    const auto index = gems::random::generate( 1, 0, FAUCET_VALUES.size() - 1 );
    const auto amount = FAUCET_VALUES.at( index[0] ) * pow( 10, supply.symbol.precision());

    // issue + transfer
    playtoken::issue_action issue( get_self(), { get_self(), "active"_n });
    issue.send( get_self(), asset{ static_cast<int64_t>( amount ), supply.symbol }, "🍈 Pomelo play tokens" );

    playtoken::transfer_action transfer( get_self(), { get_self(), "active"_n });
    transfer.send( get_self(), owner, asset{ static_cast<int64_t>( amount ), supply.symbol }, "🍈 Pomelo play tokens" );
}

} /// namespace eosio