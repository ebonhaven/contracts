#include <ebonhaven.hpp>

// Admin
ACTION ebonhaven::modstatus( name user, uint64_t character_id, uint8_t status )
{
  require_auth(get_self());
  characters_index characters(get_self(), user.value);
  auto itr = characters.find(character_id);
  characters.modify( itr, get_self(), [&](auto& c) {
    c.status = status;
  });
}

// Admin
ACTION ebonhaven::modhp( name user, uint64_t character_id, uint64_t health )
{
  require_auth(get_self());
  characters_index characters(get_self(), user.value);
  auto character = characters.get(character_id, "cannot find character");
  check(health <= character.max_hp, "value cannot exceed max hp");
  auto itr = characters.find(character_id);
  characters.modify( itr, get_self(), [&](auto& c) {
    c.hp = health;
  });
}

ACTION ebonhaven::upsrates( float_t combat_rate,
                            float_t resource_rate,
                            float_t secret_rate,
                            float_t trap_rate, 
                            float_t treasure_rate,
                            float_t loot_rate )
{
  require_auth( get_self() );

  globals_index globals_table( get_self(), get_self().value );
  auto global_singleton = globals_table.get();
  
  auto new_rates = rates{};
  new_rates.combat = combat_rate;
  new_rates.resource = resource_rate;
  new_rates.secret = secret_rate;
  new_rates.trap = trap_rate;
  new_rates.treasure = treasure_rate;
  new_rates.loot = loot_rate;
  new_rates.updated_at = time_point_sec(current_time_point());
  
  global_singleton.rates = new_rates;
  globals_table.set(global_singleton, get_self());
}

// Admin
ACTION ebonhaven::spawnitem( name to, name token_name )
{
  require_auth( get_self() );
  
  accounts_index accounts(get_self(), to.value);
  auto acct = accounts.get(to.value, "account not found");
  
  print(inventory_count(to));
  check(inventory_count(to) <= acct.max_inventory - 1, "not enough inventory space");
  auto quantity = asset(1, symbol(symbol_code("EBON"), 0));
  require_recipient( to );
  action(
    permission_level{ get_self(), name("active") },
    name("ebonhavencom"),
    name("issue"),
    make_tuple( to, name("ebonhavencom"), token_name, quantity, string("1"), string(""), string("issued from ebonhavencom"))
  ).send();
}

// Admin
ACTION ebonhaven::spawnability( name user, uint64_t character_id, uint64_t ability_id )
{
  require_auth( get_self() );

  characters_index characters(get_self(), user.value);
  abilities_index abilities(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  charhistory_index history(get_self(), user.value);

  auto c = characters.get(character_id, "couldn't find character");
  auto ability = abilities.get(ability_id, "couldn't find ability");
  auto info = history.get(character_id, "couldn't find character info");
  check(find(info.learned_abilities.begin(), info.learned_abilities.end(), ability_id) == info.learned_abilities.end(), "character ability already learned");
  if (ability.profession_lock > 0) {
    check(c.profession == ability.profession_lock, "character ability cannot be learned by this profession");  
  }
  if (ability.race_lock > 0) {
    check(c.race == ability.race_lock, "character ability cannot be learned by this race");
  }
  
  info.learned_abilities.push_back(ability_id);

  auto itr = history.find(character_id);
  history.modify( itr, get_self(), [&](auto& row) {
    row.learned_abilities = info.learned_abilities;
  });
}

// Admin
ACTION ebonhaven::newencounter( name user, uint64_t character_id, vector<uint64_t> mob_ids )
{
  require_auth( get_self() );

  characters_index characters(get_self(), user.value);
  auto character = characters.get(character_id, "couldn't find character");

  character.status = as_integer(outcomes::COMBAT);
  
  generate_encounter( user, get_self(), character, mob_ids );

  auto c_itr = characters.find(character.character_id);
  characters.modify(c_itr, get_self(), [&](auto& c) {
    c = character;
  });
}

ACTION ebonhaven::modencounter( name user,
                                uint64_t encounter_id,
                                uint8_t encounter_status, 
                                vector<mod_mob> mobs )
{
  require_auth( get_self() );

  encounters_index encounters(get_self(), user.value);
  auto encounter = encounters.get(encounter_id, "couldn't find encounter");
  
  vector<mob> updated_mobs;
  for (auto& m: mobs) {
    mob new_mob {
      m.mob_id,
      m.mob_name,
      m.level,
      m.mob_type,
      m.last_decision,
      m.last_decision_hit,
      m.attack,
      m.defense,
      m.mob_data,
      m.hp,
      m.max_hp,
      m.experience,
      m.worth,
      m.drop_id
    };
    updated_mobs.push_back(new_mob);
  }
  
  auto itr = encounters.find(encounter_id);
  if (itr != encounters.end()) {
    encounters.modify( itr, get_self(), [&](auto& e) {
      e.encounter_status = encounter_status;
      e.mobs = updated_mobs;
    });
  }
}

ACTION ebonhaven::delencounter( name user, uint64_t encounter_id ) {
  require_auth( get_self() );
  encounters_index encounters(get_self(), user.value);
  characters_index characters(get_self(), user.value);
  auto encounter = encounters.get(encounter_id, "couldn't find encounter");
  auto character = characters.get(encounter.character_id, "couldn't find character");

  if (character.status == 4) {
    auto itr = characters.find(character.character_id);
    characters.modify( itr, get_self(), [&](auto& c) {
        c.status = 0;
    });
  }

  check(character.owner == user, "account does not own character");
  auto e_itr = encounters.find(encounter_id);
  encounters.erase(e_itr);
}

ACTION ebonhaven::gentreasure( name user, uint64_t character_id )
{
  require_auth( get_self() );
  characters_index characters( get_self(), user.value );
  auto character = characters.get(character_id, "couldn't find character");
  generate_treasure( user, get_self(), character );
}

void ebonhaven::buyslot( name from, asset quantity )
{
  accounts_index accounts(get_self(), from.value);
  auto acct = accounts.get(from.value, "cannot find account");
  map<uint8_t, asset> costs{
    { 1, asset(10000, symbol(symbol_code("EOS"), 4)) },
    { 2, asset(50000, symbol(symbol_code("EOS"), 4)) },
    { 3, asset(120000, symbol(symbol_code("EOS"), 4)) },
    { 4, asset(250000, symbol(symbol_code("EOS"), 4)) }
  };
  auto c_itr = costs.find(acct.max_characters);
  if ( c_itr != costs.end() ) {
    asset cost = c_itr->second;
    check ( quantity.amount == cost.amount, "send the correct amount");
  } else {
    check( false, "cannot find cost to add slot");
  }

  if ( from != get_self() ) {
      // send EOS to account owed
      action( permission_level{ get_self(), name("active") },
              name("eosio.token"), name("transfer"),
              make_tuple( get_self(), DAC_CONTRACT, quantity, string("purchase character slot") ) ).send();
  }

  require_recipient( from );
  auto a_itr = accounts.find(from.value);
  accounts.modify( a_itr, same_payer, [&](auto& a) {
    a.max_characters = a.max_characters + 1;
  });
}
