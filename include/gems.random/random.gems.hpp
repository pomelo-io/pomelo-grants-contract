#include <eosio/eosio.hpp>
#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

namespace gems {
namespace random {
    using namespace eosio;
    using std::vector;


    // trx_id helper
    checksum256 get_trx_id()
    {
        size_t size = transaction_size();
        char buf[size];
        size_t read = read_transaction( buf, size );
        return sha256( buf, read );
    }

    vector<int64_t> generate(
        uint8_t size,
        checksum256 trx_id,
        time_point time,
        int64_t min_value = 0,
        int64_t max_value = 0,
        uint64_t salt = 0,
        bool unique = true )
    {
        if( size == 0 ) size = 1;
        if( time.time_since_epoch().count() == 0) time = current_time_point();
        if( *((uint128_t *) &trx_id) == 0 ) trx_id = get_trx_id();
        if( min_value >= max_value ) max_value = INT64_MAX;
        check( !unique || (int128_t) max_value - min_value + 1 >= size, "gems.random::generate: can't generate unique numbers for range: " + std::to_string(min_value)+ ":"+ std::to_string(max_value) );

        vector<int64_t> res;
        salt += tapos_block_prefix() + tapos_block_num();   // initial salt
        while( res.size() < size ){
            uint128_t mix = salt++;
            mix += *((uint64_t *) &trx_id);
            mix += *((uint64_t *) &trx_id + 1);
            mix += time.time_since_epoch().count() / 500000 * salt;
            mix += min_value * salt;
            mix += max_value * salt;

            const checksum256 sha = eosio::sha256( (const char*) &mix, sizeof( mix )); // generate hash for uniform distribution
            const uint64_t offset = ( salt + res.size() ) % 4;               // offset in sha
            const uint64_t rnd = *((uint64_t *) &sha + offset );             // take 8 random bytes
            const int64_t val = min_value + (int64_t) (rnd % ( (int128_t) max_value - min_value + 1 ));  // normalize to min/max values
            if( !unique || std::find( res.begin(), res.end(), val ) == res.end() ) res.push_back( val );
        }
        return res;
    }


    vector<int64_t> generate( uint8_t size = 1 ){
        return generate ( size, checksum256{}, time_point{}, 0, 0, 0 );
    }

    vector<int64_t> generate( uint8_t size, checksum256 trx_id ){
        return generate ( size, trx_id, time_point{}, 0, 0, 0 );
    }

    vector<int64_t> generate( uint8_t size, time_point time ){
        return generate ( size, checksum256{}, time, 0, 0, 0 );
    }

    vector<int64_t> generate( uint8_t size, int64_t min_value, int64_t max_value ){
        return generate ( size, checksum256{}, time_point{}, min_value, max_value, 0 );
    }

}
}