#include "ebonhaven.hpp"
#include "util.cpp"

using namespace eosio;

void ebonhaven::tokencreate( name   issuer,
                             asset  maximum_supply )
{
    require_auth( _self );

    auto sym = maximum_supply.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( maximum_supply.is_valid(), "invalid supply");
    eosio_assert( maximum_supply.amount > 0, "max-supply must be positive");

    auto seed_itr = _config.begin();

    // Initialize the seed with default value if it is not found
    if (seed_itr == _config.end()) {
        seed_itr = _config.emplace( _self, [&]( auto& globals ) { });
    }

    _config.modify( seed_itr, _self, [&]( auto& s ) {
       s.max_supply    = maximum_supply;
       s.issuer        = issuer;
    });
}

void ebonhaven::tokenreward( name to, asset quantity )
{
    auto _conf = _config.get(1, "Token does not exist");

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == _conf.max_supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= _conf.max_supply.amount - _conf.supply.amount, "quantity exceeds available supply");

    auto itr = _config.begin();
    _config.modify( itr, to, [&]( auto& s ) {
       s.supply += quantity;
    });

    accounts_table _accounts(_code, _code.value);
    auto acc_itr = _accounts.find( to.value );
    eosio_assert( acc_itr != _accounts.end(), "No valid account found to send" );
    if( acc_itr != _accounts.end() ) {
        print("Updating balance");
        _accounts.modify( acc_itr, to, [&]( auto& a ) {
            a.balance += quantity;
        });
    }

}

void ebonhaven::tokenissue( name to, asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto _conf = _config.get(1, "Token does not exist");

    require_auth( _conf.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must issue positive quantity" );

    eosio_assert( quantity.symbol == _conf.max_supply.symbol, "symbol precision mismatch" );
    eosio_assert( quantity.amount <= _conf.max_supply.amount - _conf.supply.amount, "quantity exceeds available supply");

    auto itr = _config.begin();
    _config.modify( itr, same_payer, [&]( auto& s ) {
       s.supply += quantity;
    });

    add_balance( _conf.issuer, quantity, _conf.issuer );

    if( to != _conf.issuer ) {
      SEND_INLINE_ACTION( *this, tokenxfer, { {_conf.issuer, "active"_n} },
                          { _conf.issuer, to, quantity, memo }
      );
    }
}

void ebonhaven::tokenretire( asset quantity, string memo )
{
    auto sym = quantity.symbol;
    eosio_assert( sym.is_valid(), "invalid symbol name" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto _conf = _config.get(1, "Globals do not exist" );

    require_auth( _conf.issuer );
    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must retire positive quantity" );

    eosio_assert( quantity.symbol == _conf.supply.symbol, "symbol precision mismatch" );

    auto itr = _config.begin();
    _config.modify( itr, same_payer, [&]( auto& s ) {
       s.supply -= quantity;
    });

    sub_balance( _conf.issuer, quantity );
}

void ebonhaven::tokenxfer( name    from,
                           name    to,
                           asset   quantity,
                           string  memo )
{
    eosio_assert( from != to, "cannot transfer to self" );
    require_auth( from );
    // eosio_assert( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    auto _conf = _config.get(1, "globals does not exist");

    require_recipient( from );
    require_recipient( to );

    eosio_assert( quantity.is_valid(), "invalid quantity" );
    eosio_assert( quantity.amount > 0, "must transfer positive quantity" );
    eosio_assert( quantity.symbol == _conf.max_supply.symbol, "symbol precision mismatch" );
    eosio_assert( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void ebonhaven::sub_balance( name owner, asset value ) {

    accounts_table _accounts(_code, _code.value);
    auto from = _accounts.get( owner.value );
    eosio_assert( from.balance.amount >= value.amount, "overdrawn balance" );
    auto itr = _accounts.find( owner.value );

    _accounts.modify( itr, owner, [&]( auto& a ) {
        a.balance -= value;
    });
}

void ebonhaven::add_balance( name owner, asset value, name ram_payer )
{
    accounts_table _accounts(_code, _code.value);
    auto to = _accounts.find( owner.value );
    eosio_assert( to != _accounts.end(), "No valid account found to send" );
    if( to != _accounts.end() ) {
        _accounts.modify( to, same_payer, [&]( auto& a ) {
            a.balance += value;
        });
    }
}

// void ebonhaven::tokenopen( name owner, const symbol& symbol, name ram_payer )
// {
//    require_auth( ram_payer );

//    auto sym_code_raw = symbol.code().raw();

//    stats statstable( _self, sym_code_raw );
//    const auto& st = statstable.get( sym_code_raw, "symbol does not exist" );
//    eosio_assert( st.supply.symbol == symbol, "symbol precision mismatch" );

//    accounts acnts( _self, owner.value );
//    auto it = acnts.find( sym_code_raw );
//    if( it == acnts.end() ) {
//       acnts.emplace( ram_payer, [&]( auto& a ){
//         a.balance = asset{0, symbol};
//       });
//    }
// }

// void ebonhaven::tokenclose( name owner, const symbol& symbol )
// {
//    require_auth( owner );
//    accounts acnts( _self, owner.value );
//    auto it = acnts.find( symbol.code().raw() );
//    eosio_assert( it != acnts.end(), "Balance row already deleted or never existed. Action won't have any effect." );
//    eosio_assert( it->balance.amount == 0, "Cannot close because the balance is not zero." );
//    acnts.erase( it );
// }