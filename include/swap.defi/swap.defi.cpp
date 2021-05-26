#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>


class [[eosio::contract("swap.defi")]] swapdefi : public eosio::contract {
public:
    swapdefi(eosio::name rec, eosio::name code, eosio::datastream<const char*> ds)
      : eosio::contract(rec, code, ds)
    {};

    struct token {
        eosio::name contract;
        eosio::symbol symbol;
    };

    struct [[eosio::table]] pairs_row {
        uint64_t                id;
        token                   token0;
        token                   token1;
        eosio::asset            reserve0;
        eosio::asset            reserve1;
        uint64_t                liquidity_token;
        double                  price0_last;
        double                  price1_last;
        double                  price0_cumulative_last;
        double                  price1_cumulative_last;
        eosio::time_point_sec   block_time_last;

        uint64_t primary_key() const { return id; }
    };
    typedef eosio::multi_index< "pairs"_n, pairs_row > pairs;

    [[eosio::action]]
    void setprice( uint64_t pair_id, eosio::extended_symbol ext_sym0, eosio::extended_symbol ext_sym1, double price ){
        pairs _pairs( get_self(), get_self().value );
        eosio::check( _pairs.find(pair_id) == _pairs.end(), "swap.defi: pair already exists" );

        // create user row
        _pairs.emplace( get_self(), [&]( auto & row ) {
            row.id = pair_id;
            row.token0.contract = ext_sym0.get_contract();
            row.token0.symbol = ext_sym0.get_symbol();
            row.token1.contract = ext_sym1.get_contract();
            row.token1.symbol = ext_sym1.get_symbol();
            row.reserve0 = { 0, ext_sym0.get_symbol() };
            row.reserve1 = { 0, ext_sym1.get_symbol() };
            row.liquidity_token = 0;
            row.price0_last = price;
            row.price1_last = 1 / price;
            row.price0_cumulative_last = price;
            row.price1_cumulative_last = 1 / price;
            row.block_time_last = eosio::current_time_point();
        });

    }
};