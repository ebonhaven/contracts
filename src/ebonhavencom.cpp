#include <ebonhavencom.hpp>

#include "account.cpp"
#include "admin.cpp"
#include "character.cpp"
#include "game.cpp"
#include "items.cpp"


#include "token.cpp"

// Private
ACTION ebonhavencom::gentreasure( name user, uint64_t character_id ) {
  require_auth( get_self() );
  
  rewards_index rewards(get_self(), user.value);
  // uint64_t treasure_item = roll_treasure();
  
}

ACTION ebonhavencom::printval( name user )
{
  print("Total items: ");
  print(inventory_count(user));
}

// Admin
ACTION ebonhavencom::setconfig(string version)
{
  require_auth( get_self() );

  globals_index globals_table( get_self(), get_self().value );
  asset supply = asset(0.00, symbol(symbol_code("EBON"), 2));
  asset max_supply = asset(1000000000000.00, symbol(symbol_code("EBON"),2));
  auto base_rates = rates{};
  base_rates.updated_at = time_point_sec(current_time_point());
  auto global_singleton = globals_table.get_or_create( get_self(),
    globals{ 1, version, symbol_code("EBON"), 1, 1, supply, max_supply, base_rates } );
  
  // Setup with defaults and version number      
  global_singleton.version = version;
  globals_table.set(global_singleton, get_self());
}

EOSIO_DISPATCH(ebonhavencom, 
  (newaccount)
  (newcharacter)
  (delcharacter)
  (move)
  (useitem)
  (equipitem)
  (buyability)
  (equipability)
  (printval)
  (spawnitem)
  (spawnability)
  (create)
  (issue)
  (transfernft)
  (burnnft)
  (upsaura)
  (upsability)
  (upstreasure)
  (upseffect)
  (upsstats)
  (upsdrop)
  (setconfig)
  (tokenreward)
  (tokenissue)
  (tokenxfer)
  (tokenretire)
)
