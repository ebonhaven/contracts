#include <ebonhaven.hpp>

#include "account.cpp"
#include "admin.cpp"
#include "character.cpp"
#include "game.cpp"
#include "items.cpp"
#include "quests.cpp"
#include "skills.cpp"
#include "token.cpp"

// Admin
ACTION ebonhaven::setconfig(string version)
{
  require_auth( get_self() );

  globals_index globals_table( get_self(), get_self().value );
  asset supply = asset(0.00, symbol(symbol_code("EBON"), 2));
  asset max_supply = asset(1000000000000.00, symbol(symbol_code("EBON"),2));
  rates base_rates;
  base_rates.updated_at = time_point_sec(current_time_point());
  auto global_singleton = globals_table.get_or_create( get_self(),
    globals{ 1, version, symbol_code("EBON"), 1, 1, supply, max_supply, base_rates } );
  
  // Setup with defaults and version number      
  global_singleton.version = version;
  globals_table.set(global_singleton, get_self());
}

extern "C" {
  void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
    auto self = receiver;
    if ( code == self ) {
      switch( action ) {
        EOSIO_DISPATCH_HELPER(ebonhaven, 
          (newaccount)
          (newcharacter)
          (delcharacter)
          (move)
          (combat)
          (revive)
          (claimrewards)
          (useitem)
          (equipitem)
          (buyability)
          (equipability)
          (unlock)
          (craft)
          (gather)
          (acceptquest)
          (endquest)
          (spawnitem)
          (spawnability)
          (newencounter)
          (modencounter)
          (delencounter)
          (create)
          (issue)
          (transfernft)
          (burnnft)
          (listsalenft)
          (closesalenft)
          (modstatus)
          (modhp)
          (upsrates)
          (gentreasure)
          (setconfig)
          (tokenreward)
          (tokenissue)
          (tokenxfer)
          (tokenretire)
        )
        break;
      }
    } 
    else if ( code == name("eosio.token").value && action == name("transfer").value ) {
      execute_action( name(receiver), name(code), &ebonhaven::buynft );
    }
  }
}


