#include <ebonhavencom.hpp>

ACTION ebonhavencom::move( name user, uint64_t character_id, position new_position) {
  require_auth( user );
  
  characters_index characters(get_self(), user.value);
  auto character = characters.get(character_id, "cannot find character");
  check(character.owner == user, "does not belong to user");
  check(character.status == 0, "status does not allow movement");
  check(character.hp > 0, "cannot move");
  
  bool same_position = false;
  if (new_position.world == character.position.world && 
      new_position.zone == character.position.zone &&
      new_position.x == character.position.x &&
      new_position.y == character.position.y ) {
    check(false, "cannot move to same position");
  }
  
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
    //generate_treasure(user, character);
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
    //generate_encounter(user, user, character, v);
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
  encounter.turn_counter += 1;

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
        eosio_assert(character.abilities.raceability != 0, "No ability in ability slot 3");
        ability = abilities.get(character.abilities.raceability, "Couldn't find ability");
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
        encounter.encounter_status = 1;
        character.status = 0;
        finalize_encounter( user, character.level, encounter );
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
            update_encounter(user, user, encounter);
        } else {
            character.hp = 0;
            character.status = 0;
            encounter.encounter_status = 2;
            update_encounter(user, user, encounter);
        }
      } else {
        encounter.mobs[mob_idx].last_decision_hit = 0;
        print("\nPLAYER TURN: Miseed");
      }
    }
    
  
    update_character(user, user, character);
      
  } else if (combat_decision == as_integer(combat_decision::FLEE)) {
    print("\nCombat Decision: FLEE");
    
    int FLEE_MIN = 35;

    if (roll >= FLEE_MIN) {
      //string new_log = update_log(encounter.encounter_id, "Flee failed. Lose health.");
      print(" Failed. Lose health x2");
      // TODO: Calc mob damage
      int damage = 75;
      if (damage < character.hp) {
        character.hp -= damage;
        update_character(user, user, character);
      } else {
        character.hp = 0;
        character.status = 0;
        update_character(user, user, character);
      }
    } else if (roll < FLEE_MIN) {
      print(" Success. No combat.");
      encounter.encounter_status = 1;
      flee_encounter( user, encounter );
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
};

// TODO: Add crits = double damage
uint64_t ebonhavencom::calculate_player_attack_damage( name user,
                                                       ebonhaven::character& c,
                                                       ebonhaven::mob& m, bool is_ranged = false)
{
  uint64_t damage = 0;
  uint64_t aura_damage = 0;
  dgood_index items(get_self(), get_self().value);
  auras_table auras(get_self(), get_self().value);

  ebonhaven::item item;

  if (!is_ranged) {
    if (m.mob_type == as_integer(mob_type::FLYING)) {
      return damage;
    }
    if (c.equipped.weapon > 0) {
      item = items.get(c.equipped.weapon, "Couldn't find weapon");
      for ( auto a: item.auras ) {
        auto aura = auras.get(a, "Couldn't find aura");
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
      item = items.get(c.equipped.ranged, "Couldn't find ranged weapon");
      for ( auto a: item.auras ) {
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

ebonhavencom::reward ebonhaven::generate_reward( name user, name payer, uint32_t character_level, encounter s_encounter ) {
  reward r = {};
  asset total_reward = asset(0, symbol(symbol_code("EBON"),2));
  r.experience = calculate_total_experience(character_level, s_encounter);
  vector<uint64_t> rewardItems;
  for (auto& mob: s_encounter.mobs) {
    print(" Mob worth: ", mob.worth.amount);
    total_reward.amount += mob.worth.amount;
    auto roll = random(100);
    print(" Comp roll: ", roll);
    for (auto& drop: mob.drop.drops) {
      uint8_t perc = abs(floor((100 * drop.percentage) / 100));
      // print(" Rolled on drop: ", perc);
      if ( roll <= perc ) {
        print(" Item won: ", drop.item_id);
        rewardItems.push_back(drop.item_id);
      }
    }
  }

  dgood_index items(get_self(), get_self().value);
  for (auto& id: rewardItems) {
    auto itr = items.find(id);
    if (itr != items.end()) {
      item newItem = items.get(id);
      newItem.parent_id = newItem.item_id;
      auto str = to_string(newItem.item_id) + newItem.item_name + to_string(current_time());
      auto hash = create_hash(str);
      newItem.item_id = hash;
      upsert_item( user, newItem, payer );
      r.items.push_back( newItem );
    }
  }

  r.worth = total_reward;

  auto str = "reward" + to_string(current_time());
  auto hash = create_hash(str);
  r.reward_id = hash;
  r.encounter_id = s_encounter.encounter_id;
  r.character_id = s_encounter.character_id;

  print(" Total mob worth: ", total_reward);
  return r;
}

// TODO: No experience if mob is more than 4 levels below character
uint32_t ebonhavencom::calculate_total_experience(uint32_t character_level, encounter e) {
  uint32_t total_exp = 0;
  for (auto& mob: e.mobs) {
      total_exp += mob.experience;
  }
  return total_exp;
}