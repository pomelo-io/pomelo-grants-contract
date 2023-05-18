#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>

using namespace eosio;

namespace defi {

    class [[eosio::contract("swap.defi")]] swap : public eosio::contract {
        public:
        using contract::contract;

        static constexpr name code = "swap.defi"_n;

        /**
         * Custom Token struct
         */
        struct token {
            name contract;
            symbol symbol;

            std::string to_string() const {
                return contract.to_string() + "-" + symbol.code().to_string();
            };
        };

        /**
         * Defibox pairs
         */
        struct [[eosio::table]] pairs_row {
            uint64_t            id;
            token               token0;
            token               token1;
            asset               reserve0;
            asset               reserve1;
            uint64_t            liquidity_token;
            double              price0_last;
            double              price1_last;
            double              price0_cumulative_last;
            double              price1_cumulative_last;
            time_point_sec      block_time_last;

            uint64_t primary_key() const { return id; }
        };
        typedef eosio::multi_index< "pairs"_n, pairs_row > pairs;

        /**
         * ## STATIC `convert`
         *
         * Given an asset and Defibox pair id, return corresponding amount based on Defibox swap
         *
         * ### params
         *
         * - `{extended_asset} in` - input tokens
         * - `{uint64_t} pair_id` - pair id
         *
         * ### example
         *
         * ```c++
         * // Inputs
         * const extended_asset in = { asset { 100000, "TLOS" }, "ibc.wt.tlos"_n };
         * const symbol pair_id = 2136;
         *
         * // Calculation
         * const auto out = defi::swap::convert( in, pair_id );
         * // => "1.9123 EOS @ eosio.token"
         * ```
         */
        static extended_asset convert( const extended_asset in, const uint64_t pair_id )
        {
            pairs _pairs( code, code.value );
            auto pairs = _pairs.get( pair_id, "Defiswap: invalid pair id" );

            if (pairs.token0.contract == in.contract && pairs.token0.symbol == in.quantity.symbol) {
                return extended_asset {static_cast<int64_t>(in.quantity.amount * pairs.price0_last), extended_symbol(pairs.token1.symbol, pairs.token1.contract)};
            }
            else if (pairs.token1.contract == in.contract && pairs.token1.symbol == in.quantity.symbol) {
                return extended_asset {static_cast<int64_t>(in.quantity.amount * pairs.price1_last), extended_symbol(pairs.token0.symbol, pairs.token0.contract)};
            }
            eosio::check(false, "Defiswap: pair id does not match input token");
            return extended_asset {0, in.get_extended_symbol()};    // never reached
        }
    };
}
