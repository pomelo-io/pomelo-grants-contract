
[[eosio::action]]
void pomelo::setstatus( const name status )
{
    require_auth( get_self() );

    auto _config = config.get_or_default();

    _config.status = status;
    config.set( _config, get_self() );
}


[[eosio::action]]
void pomelo::setvaluesym( const extended_symbol value_symbol )
{
    require_auth( get_self() );

    auto _config = config.get_or_default();

    _config.value_symbol = value_symbol;
    config.set( _config, get_self() );
}
