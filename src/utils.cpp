#include <eosio/transaction.hpp>
#include <eosio/crypto.hpp>

namespace eosn {

checksum256 get_trx_id()
{
    size_t size = transaction_size();
    char buf[size];
    size_t read = read_transaction( buf, size );
    check( size == read, "pomelo::get_trx_id: read_transaction failed");
    return sha256( buf, read );
}

}
