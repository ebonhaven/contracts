#include <ebonhavencom.hpp>

ACTION ebonhavencom::tokenreward( name to, asset quantity )
{
  require_auth(get_self());

  check( quantity.is_valid(), "invalid quantity" );
  check( quantity.amount > 0, "must issue positive quantity" );

  globals_index globals_table( get_self(), get_self().value );
  auto global_singleton = globals_table.get();

  check( quantity.symbol == global_singleton.max_supply.symbol, "symbol precision mismatch" );
  check( quantity.amount <= global_singleton.max_supply.amount - global_singleton.supply.amount, "quantity exceeds available supply");

  global_singleton.supply += quantity;
  globals_table.set(global_singleton, get_self());

  accounts_index accounts_table(get_self(), to.value);
  auto acc_itr = accounts_table.find( to.value );
  check( acc_itr != accounts_table.end(), "no valid account found" );
  accounts_table.modify( acc_itr, get_self(), [&]( auto& a ) {
    a.balance += quantity;
  });
}

ACTION ebonhavencom::tokenissue( name to, asset quantity )
{
  require_auth( get_self() );
  auto sym = quantity.symbol;
  check( sym.is_valid(), "invalid symbol name" );

  globals_index globals_table( get_self(), get_self().value );
  auto global_singleton = globals_table.get();
  
  check( quantity.is_valid(), "invalid quantity" );
  check( quantity.amount > 0, "must issue positive quantity" );

  check( quantity.symbol == global_singleton.max_supply.symbol, "symbol precision mismatch" );
  check( quantity.amount <= global_singleton.max_supply.amount - global_singleton.supply.amount, "quantity exceeds available supply");
  
  global_singleton.supply += quantity;
  globals_table.set(global_singleton, get_self());

  add_token_balance( get_self(), quantity, get_self() );

  if( to != get_self() ) {
    action(
      permission_level{ get_self(), name("active") },
      name("ebonhavencom"),
      name("tokenxfer"),
      make_tuple( get_self(), to, quantity )
    ).send();
  }
}

ACTION ebonhavencom::tokenxfer( name from, name to, asset quantity )
{
    require_auth( from );
    check( from != to, "cannot transfer to self" );
    check( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();
    globals_index globals_table( get_self(), get_self().value );
    auto global_singleton = globals_table.get();

    require_recipient( from );
    require_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == global_singleton.max_supply.symbol, "symbol precision mismatch" );

    auto payer = has_auth( to ) ? to : from;

    sub_token_balance( from, quantity );
    add_token_balance( to, quantity, payer );
}

ACTION ebonhavencom::tokenretire( asset quantity, string memo )
{
  require_auth( get_self() );
  auto sym = quantity.symbol;
  check( sym.is_valid(), "invalid symbol name" );
  check( memo.size() <= 256, "memo has more than 256 bytes" );

  globals_index globals_table( get_self(), get_self().value );
  auto global_singleton = globals_table.get();
  
  check( quantity.is_valid(), "invalid quantity" );
  check( quantity.amount > 0, "must retire positive quantity" );

  check( quantity.symbol == global_singleton.supply.symbol, "symbol precision mismatch" );
  
  global_singleton.supply -= quantity;
  globals_table.set(global_singleton, get_self());

  sub_token_balance( get_self(), quantity );
}

void ebonhavencom::add_token_balance( name owner, asset value, name ram_payer )
{
    accounts_index accounts_table(get_self(), owner.value);
    auto to = accounts_table.find( owner.value );
    check( to != accounts_table.end(), "no valid account to send" );
    accounts_table.modify( to, ram_payer, [&]( auto& a ) {
      a.balance += value;
    });
}

void ebonhavencom::sub_token_balance( name owner, asset value )
{
    accounts_index accounts_table(get_self(), owner.value);
    auto from = accounts_table.get( owner.value );
    check( from.balance.amount >= value.amount, "overdrawn balance" );
    auto itr = accounts_table.find( owner.value );

    accounts_table.modify( itr, owner, [&]( auto& a ) {
      a.balance -= value;
    });
}