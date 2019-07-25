#define RANDOM_FACTOR 0.2
#define ARMORED_FACTOR 0.15
#define MAX_LEVEL 60

#include <ebonhaven.hpp>

using namespace std;

ACTION ebonhaven::move( name user, uint64_t character_id, position new_position) {
  require_auth( user );
  
  characters_index characters(get_self(), user.value);
  auto character = characters.get(character_id, "cannot find character");
  check(character.owner == user, "does not belong to user");
  check(character.status == 0, "status does not allow movement");
  check(character.hp > 0, "cannot move");
  
  // Check if in same position
  bool same_position = false;
  if (new_position.world_zone_id == character.position.world_zone_id && 
      new_position.x == character.position.x &&
      new_position.y == character.position.y ) {
    check(false, "cannot move to same position");
  }

  // Check if within movement radius
  auto current = make_pair(character.position.x, character.position.y);
  auto target = make_pair(new_position.x, new_position.y);
  if (!is_position_within_target_radius(current, target, character.movement_radius)) {
    check(false, "target outside movement radius");
  }

  // Check if tile exists and walkable
  mapdata_index mapdata(get_self(), user.value);
  auto md = find_if(mapdata.begin(), mapdata.end(),
    [&character](const struct mapdata& el){ return el.world_zone_id == character.position.world_zone_id &&
                                            el.character_id == character.character_id; }
                  );
  check(md != mapdata.end(), "mapdata not found");
  bool is_walkable = is_coordinate_walkable(md->tiles, new_position.x, new_position.y);
  check(is_walkable, "cannot move to position");

  character.position = new_position;
  
  int range = 1000;
  auto num = random(range);
  
  globals_index globals_table( get_self(), get_self().value );
  auto global_singleton = globals_table.get();
  
  int LOOT_MAX = rate_to_floor(global_singleton.rates.loot, range);
  int TREASURE_MAX = rate_to_floor(global_singleton.rates.treasure, range) + LOOT_MAX;
  int TRAP_MAX = rate_to_floor(global_singleton.rates.trap, range) + TREASURE_MAX;
  int DISCOVERY_MAX = rate_to_floor(global_singleton.rates.discovery, range) + TRAP_MAX;
  int RESOURCE_MAX = rate_to_floor(global_singleton.rates.resource, range) + DISCOVERY_MAX;
  int COMBAT_MAX = rate_to_floor(global_singleton.rates.combat, range) + RESOURCE_MAX;
  
  if (num <= LOOT_MAX) {
    print("Outcome is: LOOT");
    character.status = 6;
  } else if (num > LOOT_MAX && num <= TREASURE_MAX) {
    print("Outcome is: TREASURE");
    // generate_treasure(user, character);
    character.status = 5;
  } else if (num > TREASURE_MAX && num <= TRAP_MAX) {
    print("Outcome is: TRAP");
    character.status = 3;
  } else if (num > TRAP_MAX && num <= DISCOVERY_MAX) {
    print("Outcome is: DISCOVERY");
    character.status = 1;
  } else if (num > DISCOVERY_MAX && num <= RESOURCE_MAX) {
    print("Outcome is: RESOURCE");
    character.status = 2;
  } else if (num > RESOURCE_MAX && num <= COMBAT_MAX) {
    // Generate encounter
    character.status = 4;
    vector<uint64_t> v = { 101 };
    generate_encounter(user, user, character, v);
    print("Outcome is: COMBAT");
  } else {
    print("Outcome is: NONE");
  }
  
  auto c_itr = characters.find(character_id);
  characters.modify( c_itr, user, [&](auto& c) {
    c = character;
  });
}

ACTION ebonhaven::combat( name user, uint64_t encounter_id, uint8_t combat_decision, uint8_t mob_idx) {
  require_auth( user );
  encounters_index encounters(get_self(), user.value);
  auto encounter = encounters.get(encounter_id, "encounter not found");

  characters_index characters(get_self(), user.value);
  abilities_index abilities(get_self(), get_self().value);
  
  auto character = characters.get(encounter.character_id, "character doesn't exist");
  check(character.owner == user, "character doesn't belong to user");
  check(character.status == 4, "character is not in combat");
  check(combat_decision > 0 && combat_decision <= 8, "combat decision not valid");
  check(mob_idx <= encounter.mobs.size(), "mob index out of bounds");
  check(encounter.encounter_status == 0, "encounter no longer in progress");
  check(encounter.mobs[mob_idx].hp != 0, "mob is already dead");

  encounter.last_decision = combat_decision;
  encounter.turn_counter++;

  bool mob_can_hit = true;
  bool player_can_hit = true;

  // TODO: Check if bribe possible


  // Player Turn
  auto roll = random(100);
  print("\nRoll is: ", roll);
  
  // TODO: Check ability cooldowns
  if (combat_decision > 0 && combat_decision <= 6) {
    print("\nCombat Decision: FIGHT");
    ebonhaven::ability ability = {};
    switch (combat_decision) {
      case 1:
        print("\nBasic Attack Chosen");
        break;
      case 2:
        print("\nRanged Attack Chosen");
        check(character.equipped.ranged != 0, "No ranged weapon equipped");
        break;
      case 3:
        print("\nAbility 1 Chosen\n");
        check(character.abilities.ability1 != 0, "No ability in ability slot 1");
        ability = abilities.get(character.abilities.ability1, "Couldn't find ability");
        print("\nAbility: ", ability.ability_name);
        break;
      case 4:
        print("\nAbility 2 Chosen\n");
        check(character.abilities.ability2 != 0, "No ability in ability slot 2");
        ability = abilities.get(character.abilities.ability2, "Couldn't find ability");
        print("\nAbility: ", ability.ability_name);
        break;
      case 5:
        print("\nAbility 3 Chosen\n");
        check(character.abilities.ability3 != 0, "No ability in ability slot 3");
        ability = abilities.get(character.abilities.ability2, "Couldn't find ability");
        print("\nAbility: ", ability.ability_name);
        break;
      case 6:
        print("\nRace Ability Chosen\n");
        check(character.abilities.racial != 0, "No ability in ability slot 3");
        ability = abilities.get(character.abilities.racial, "Couldn't find ability");
        print("\nAbility: ", ability.ability_name);
    }

    // TODO: Calculate hit chance
    int HIT_MIN = 24;

    if (roll >= HIT_MIN && player_can_hit == true) {
      print("\nPLAYER TURN: Hit");
      bool is_ranged = false;
      if (combat_decision == 2) {
        is_ranged = true;
      }
      auto damage = calculate_player_attack_damage(user, character, encounter.mobs[mob_idx], is_ranged);

      auto itr = encounters.find(encounter_id);
      
      if (encounter.mobs[mob_idx].hp > damage) {
        encounter.mobs[mob_idx].hp -= damage;
      } else {
        mob_can_hit = false;
        encounter.mobs[mob_idx].hp = 0;
        
        if (is_encounter_over(character, encounter)) {
          encounter.encounter_status = 1;
          character.status = 0;
          generate_reward( user, user, character.level, encounter );  
        }
        
      }
    } else {
      print("\nPLAYER TURN: Missed");
      encounter.last_decision_hit = 0;
    }

    // TODO: Make mob decision
    // TODO: Calculate attack damage
    int MOB_HIT_MIN = 0;
    int mob_damage = 5;
    if (mob_can_hit) {
      encounter.mobs[mob_idx].last_decision = 1;
      if (roll >= MOB_HIT_MIN) {
        print("\n MOB TURN: Hit");

        if (mob_damage < character.hp) {
            character.hp -= mob_damage;
        } else {
            character.hp = 0;
            character.status = 0;
            encounter.encounter_status = 2;
        }
      } else {
        encounter.mobs[mob_idx].last_decision_hit = 0;
        print("\nPLAYER TURN: Miseed");
      }
    }
      
  } else if (combat_decision == as_integer(combat_decision::FLEE)) {
    print("\nCombat Decision: FLEE");
    
    int FLEE_MIN = 35;

    if (roll >= FLEE_MIN) {

      print(" Failed. Lose health x2");
      // TODO: Calc mob damage
      int damage = 75;
      if (damage < character.hp) {
        character.hp -= damage;
      } else {
        character.hp = 0;
        character.status = 0;
      }
    } else if (roll < FLEE_MIN) {
      print(" Success. No combat.");
      encounter.encounter_status = 1;
      //flee_encounter( user, encounter );
    }
  } else if (combat_decision == as_integer(combat_decision::BRIBE)) {
    print("\nCombat Decision: BRIBE");

    int BRIBE_MIN = 25;

    if (roll >= BRIBE_MIN) {
      encounter.encounter_status = 1;
      print(" Success. Deduct balance");
    } else if (roll < BRIBE_MIN) {
      print(" Fail. Deduct balance and lose health");
    }
    // Check if user has required balance
  }
  
  auto c_itr = characters.find(encounter.character_id);
  characters.modify(c_itr, user, [&](auto& c) {
    c = character;
  });
  
  auto e_itr = encounters.find(encounter_id);
  encounters.modify(e_itr, user, [&](auto& e) {
    e = encounter;
  });
};

// TODO: Add crits = double damage
uint64_t ebonhaven::calculate_player_attack_damage( name user,
                                                       ebonhaven::character& c,
                                                       ebonhaven::mob& m, bool is_ranged = false)
{
  uint64_t damage = 0;
  uint64_t aura_damage = 0;
  dgoods_index items(get_self(), get_self().value);
  
  auras_index auras(get_self(), get_self().value);

  ebonhaven::dgood item;

  if (!is_ranged) {
    if (m.mob_type == as_integer(mob_type::FLYING)) {
      return damage;
    }
    if (c.equipped.weapon > 0) {
      item = items.get(c.equipped.weapon, "couldn't find weapon");
      stats_index stats(get_self(), item.category.value);
      auto stat = stats.get(item.token_name.value, "couldn't find parent item");
      for ( auto a: stat.attributes.auras ) {
        auto aura = auras.get(a, "couldn't find aura");
        json j = json::parse(aura.aura_data);
        if (j.count("damage") > 0) {
            aura_damage += j["damage"].get<uint64_t>();
        }
      }
    } else {
      damage += 1;
    }
    print("\nAura Damage: ", aura_damage);
    print("\nPhysical Attack: ", c.attack.physical);
    print("\nMob Defense: ", m.defense.physical);
    damage = (aura_damage + c.attack.physical) - m.defense.physical;      
  } else {
    if (c.equipped.ranged > 0) {
      item = items.get(c.equipped.ranged, "couldn't find ranged weapon");
      stats_index stats(get_self(), item.category.value);
      auto stat = stats.get(item.token_name.value, "couldn't find parent item");
      for ( auto a: stat.attributes.auras ) {
        auto aura = auras.get(a, "Couldn't find aura");
        json j = json::parse(aura.aura_data);
        if (j.count("damage") > 0) {
            aura_damage += j["damage"].get<uint64_t>();
        }
      }
    } else {
      check(false, "Ranged weapon must be equipped");
    }
    print("\nAura Damage: ", aura_damage);
    print("\nPhysical Attack: ", c.attack.physical);
    print("\nMob Defense: ", m.defense.physical);
    damage = (aura_damage + c.attack.physical) - m.defense.physical;
  }
  if (m.mob_type == as_integer(mob_type::ARMORED) || m.mob_type == as_integer(mob_type::ELITE) || m.mob_type == as_integer(mob_type::BOSS)) {
    damage -= floor(damage * ARMORED_FACTOR);
  }

  print("\nDamage Before Random: ", damage);
  // Adds random factor
  int random_factor = floor(damage * RANDOM_FACTOR);
  print("\nRandom Factor: ", random_factor);
  damage = (damage - random_factor) + random(random_factor);

  print("\nFinal Damage: ", damage);

  return damage;
}

void ebonhaven::generate_encounter( name user, name payer, ebonhaven::character& character, vector<uint64_t> mob_ids)
{
    encounters_index encounters(get_self(), user.value);
    check(character.status == 4, "character not in combat");
    for (auto& enc: encounters) {
        check(enc.character_id != character.character_id, "encounter already exists for character");
    }

    mobs_index mobs(get_self(), get_self().value);
    vector<mob> v_mobs = {};
    for (uint8_t i = 0; i < mob_ids.size(); i++) {
      auto mob = mobs.get(mob_ids[i], "can't find mob");
      v_mobs.push_back(mob);
    }
    
    ebonhaven::encounter enc;
    enc.encounter_id = encounters.available_primary_key();
    if (enc.encounter_id == 0) {
      enc.encounter_id = 1;
    }
    enc.character_id = character.character_id;
    enc.mobs = v_mobs;

    encounters.emplace( payer, [&]( auto& e ) {
      e = enc;
    });
};

ACTION ebonhaven::claimrewards( name user, uint64_t reward_id, vector<name> selected_items)
{
  require_auth( user );

  accounts_index accounts(get_self(), user.value );
  characters_index characters(get_self(), user.value );
  rewards_index rewards(get_self(), user.value);
  encounters_index encounters(get_self(), user.value);
  auto reward = rewards.get( reward_id, "reward not found" );
  auto acct = accounts.get( user.value, "couldn't find account");
  auto character = characters.get( reward.character_id, "couldn't find character" );
  check(character.owner == user, "character does not belong to user");

  if (reward.experience > 0) {
    character.experience += reward.experience;
  }

  if (reward.worth.amount > 0) {
    action(
      permission_level{get_self(),"active"_n},
      get_self(),
      name("tokenreward"),
      std::make_tuple(user, reward.worth)
    ).send();
  }
  
  for (auto const& item: reward.items) {
    stats_index stats(get_self(), name("ebonhavencom").value);
    auto itr = stats.find(item.value);
    auto i_itr = find(selected_items.begin(), selected_items.end(), item);

    if (i_itr != selected_items.end()) {
      check(inventory_count(user) <= acct.max_inventory - 1, "not enough inventory space");
      action(
        permission_level{ get_self(), name("active") },
        name("ebonhavencom"),
        name("issue"),
        make_tuple( user, name("ebonhavencom"), item, string("1"), string("1"), string(""), string("issued from ebonhavencom"))
      ).send();
      selected_items.erase(i_itr);
    }
  }

  if (reward.encounter_id > 0) {
    auto e_itr = encounters.find(reward.encounter_id);
    encounters.erase(e_itr);
  }
  
  // Update character
  auto c_itr = characters.find(reward.character_id);
  characters.modify(c_itr, user, [&](auto& c) {
    c.experience = character.experience;
  });
  
  auto r_itr = rewards.find(reward_id);
  rewards.erase(r_itr);
}

void ebonhaven::generate_reward( name user,
                                 name payer,
                                 uint32_t character_level,
                                 encounter s_encounter )
{
  drops_index drops(get_self(), get_self().value);
  rewards_index rewards(get_self(), user.value);
  reward r = {};
  asset total_reward = asset(0, symbol(symbol_code("EBON"),2));
  r.experience = calculate_total_experience(character_level, s_encounter);
  vector<name> reward_items;
  for (auto& mob: s_encounter.mobs) {
    print(" Mob worth: ", mob.worth.amount);
    total_reward.amount += mob.worth.amount;
    auto roll = random(100);
    print(" Comp roll: ", roll);
    auto m_drop = drops.get(mob.drop_id, "couldn't find drop");
    for (auto& drop: m_drop.drops) {
      uint8_t perc = abs(floor((100 * drop.percentage) / 100));
      // print(" Rolled on drop: ", perc);
      if ( roll <= perc ) {
        print(" Item won: ", drop.token_name);
        reward_items.push_back(drop.token_name);
      }
    }
  }
  
  r.reward_id = rewards.available_primary_key();
  if (r.reward_id == 0) {
    r.reward_id = 1;
  }
  r.items = reward_items;
  r.worth = total_reward;
  r.encounter_id = s_encounter.encounter_id;
  r.character_id = s_encounter.character_id;

  print(" Total mob worth: ", total_reward);
  
  rewards.emplace(payer, [&](auto& reward) {
    reward = r;
  });
  
}

// TODO: No experience if mob is more than 4 levels below character
uint32_t ebonhaven::calculate_total_experience(uint32_t character_level, encounter e) {
  uint32_t total_exp = 0;
  for (auto& mob: e.mobs) {
    total_exp += mob.experience;
  }
  return total_exp;
}

bool ebonhaven::is_encounter_over(character c, encounter e) {
  if (c.hp == 0) {
    return true;
  }
  vector<mob> alive_mobs;
  copy_if(e.mobs.begin(), e.mobs.end(), back_inserter(alive_mobs), [](mob m) { return m.hp > 0; });
  if (alive_mobs.size() == 0) {
    return true;
  }
  return false;
}