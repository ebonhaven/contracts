#include <ebonhaven.hpp>

#include "account.cpp"
#include "admin.cpp"
#include "character.cpp"
#include "game.cpp"
#include "items.cpp"
#include "quests.cpp"
#include "skills.cpp"
#include "token.cpp"

ACTION ebonhaven::printval( name user, uint64_t x, uint64_t y )
{
  mapdata_index mapdata(get_self(), get_self().value);
  auto m = mapdata.get(1, "couldn't find map");
  bool is_walkable = is_coordinate_walkable(m.tiles, x, y);
  print("Is Walkable: ");
  print(is_walkable);
}

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

EOSIO_DISPATCH(ebonhaven, 
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
  (printval)
  (spawnitem)
  (spawnability)
  (newencounter)
  (modencounter)
  (delencounter)
  (create)
  (issue)
  (transfernft)
  (burnnft)
  (modstatus)
  (modhp)
  (upsaura)
  (upsability)
  (upseffect)
  (upsstats)
  (upsrates)
  (upsmob)
  (upsdrop)
  (upsresource)
  (upstreasure)
  (upsmapdata)
  (upsrecipe)
  (gentreasure)
  (upsquest)
  (upsnpc)
  (setconfig)
  (tokenreward)
  (tokenissue)
  (tokenxfer)
  (tokenretire)
)
