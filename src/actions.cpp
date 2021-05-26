[[eosio::action]]
void pomelo::setgrant( const name id, const name author_id, const set<name> authorized_user_ids, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    require_auth( get_self() );

    pomelo::grants_table grants( get_self(), get_self().value );
    set_project( grants, "grant"_n, id, author_id, authorized_user_ids, funding_account, accepted_tokens );
}

[[eosio::action]]
void pomelo::setbounty( const name id, const name author_id, const set<name> authorized_user_ids, const name funding_account, const set<extended_symbol> accepted_tokens )
{
    require_auth( get_self() );

    pomelo::bounties_table bounties( get_self(), get_self().value );
    set_project( bounties, "bounty"_n, id, author_id, authorized_user_ids, funding_account, accepted_tokens );
}

[[eosio::action]]
void pomelo::enable( const name project_type, const name project_id, const name status )
{
    require_auth( get_self() );

    // tables
    pomelo::grants_table _grants( get_self(), get_self().value );
    pomelo::bounties_table _bounties( get_self(), get_self().value );

    // validate
    check( status == "ok"_n || status == "pending"_n || status == "disabled"_n, "pomelo::enable: invalid [status]" );

    if ( project_type == "grant"_n ) enable_project( _grants, project_id, status );
    else if ( project_type == "bounty"_n ) enable_project( _bounties, project_id, status );
    else check( false, "pomelo::enable: invalid [project_type]");
}

template <typename T>
void pomelo::enable_project( T& table, const name id, const name status )
{
    const auto & itr = table.get( id.value, "pomelo::enable_project: [project_id] does not exist");
    table.modify( itr, get_self(), [&]( auto & row ) {
        check( row.status != status, "pomelo::enable_project: status was not modified");
        row.status = status;
        row.updated_at = current_time_point();
    });
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
void pomelo::joinround( const name grant_id, const uint64_t round_id )
{
    require_auth( get_self() );

    pomelo::grants_table grants( get_self(), get_self().value );
    const auto grant = grants.get( grant_id.value, "pomelo::joinround: [grant_id] does not exist" );

    pomelo::rounds_table rounds( get_self(), get_self().value );
    const auto round_itr = rounds.find( round_id );
    check( round_itr != rounds.end(),  "pomelo::joinround: [round_id] does not exist" );
    check( round_itr->grant_ids.count( grant_id ) == 0, "pomelo::joinround: grant already exists in this round");

    rounds.modify( round_itr, get_self(), [&]( auto & row ) {
        row.grant_ids.insert(grant_id);
        row.updated_at = current_time_point();
    });
}

[[eosio::action]]
void pomelo::init( const uint64_t round_id, const uint64_t status )
{
    require_auth( get_self() );

    set_key_value("round.id"_n, round_id );
    set_key_value("status"_n, status );

    // // skip round validation
    // if ( round_id == 0 ) return;

    // // make sure round exist and is not over
    // pomelo::rounds_table _rounds( get_self(), get_self().value );
    // const auto round = _rounds.get( round_id, "pomelo::init: [round_id] is not found" );
    // const auto now = current_time_point().sec_since_epoch();
    // check( round.end_at.sec_since_epoch() > now, "pomelo::init: [round_id] has already ended" );
}

// #include <eosio/permission.hpp>
// #include <eosio/crypto.hpp>

// [[eosio::action]]
// void pomelo::test()
// {
//     size_t size = transaction_size();
//     char buf[size];
//     size_t read = read_transaction( buf, size );
//     print("\nRead: ", read);
//     const transaction* trx = (transaction *) buf;
//     check( size == read, "pomelo::get_trx_id: read_transaction failed");


//     const string key_str = "EOS5nJQ931u41YtNBCDhXXdySDTEvmzbsaqwVq3XGPK23BzyEFMC3";
//     eosio::public_key key;
//     eosio::permission_level permission{ "user1.eosn"_n, "active"_n };
//     bool res = check_transaction_authorization(*trx, set<permission_level>{}, set<public_key>{});
//     print("\nRes: ", res);

//     check(false, "BYE");
// }

// [[eosio::action]]
// void pomelo::ecrecover(std::string data, const signature &sig)
// {
//     checksum256 digest;
//     sha256(&data[0], data.size(), &digest);
//     uint8_t pub[34];
//     auto res = recover_key(&digest, (char *)&sig, sizeof(sig), (char*)pub, 34);
//     printhex(pub, sizeof(pub));
// }