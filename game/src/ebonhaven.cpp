#include <ebonhaven.hpp>

#include "account.cpp"
#include "admin.cpp"
#include "character.cpp"
#include "game.cpp"
#include "items.cpp"
#include "quests.cpp"
#include "skills.cpp"
#include "token.cpp"

using namespace eosio;
using namespace utility;

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


void ebonhaven::transfer( uint64_t sender, uint64_t receiver )
{
  auto transfer_data = unpack_action_data<raw_transfer>();
  if ( transfer_data.memo == "deposit" ) return;
  if ( transfer_data.to != get_self() ) return;
  if ( transfer_data.from == name("eosio.stake") ) return;
  check( transfer_data.quantity.symbol == symbol( symbol_code("EOS"), 4), "can buy only with EOS" );
  check( transfer_data.quantity.amount > 0, "quantity must be positive");
  
  // action:id:recipient
  transfer_action ta = parsetransfer(transfer_data);
  accounts_index accounts( get_self(), name(ta.from).value );
  auto a_itr = accounts.find( name(ta.from).value );
  check( a_itr != accounts.end(), "account not found" );

  if (ta.action.size() == 0) {
    return;
  } else if (ta.action == "buynft") {
    buynft( ta.from, ta.recipient, ta.id, ta.quantity );
  } else if (ta.action == "buyslot") {
    buyslot( ta.from, ta.quantity );
  }

}

extern "C" {
  void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
    auto self = receiver;
    if ( code == self ) {
      switch( action ) {
        EOSIO_DISPATCH_HELPER(ebonhaven, 
          (newaccount)
          (newcontact)
          (delcontact)
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
      execute_action( name(receiver), name(code), &ebonhaven::transfer );
    }
  }
}


