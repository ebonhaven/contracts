#include <ebonhaven.hpp>

using namespace nlohmann;

ACTION ebonhaven::newcharacter( name     user,
                                   string   character_name,
                                   uint8_t  gender,
                                   uint8_t  profession,
                                   uint8_t  race )
{
  require_auth( user );
  
  check( gender >= 0 && gender <= 1, "must be valid gender");
  check( profession > 0 && profession <= 5, "must be valid profession");
  check( race > 0 && race <= 6, "must be valid race");
  
  // Create account if does not exist
  accounts_index accounts(get_self(), user.value);
  auto ac_itr = accounts.find(user.value);
  if ( ac_itr == accounts.end() ) {
    ac_itr = accounts.emplace( user, [&](auto& a) {
      a.user = user;
    });
  }
  
  auto acct = accounts.get(user.value);
  
  basestats_index basestats(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  auto& base = basestats.get(profession, "profession stats not found");
  
  characters_index characters(get_self(), user.value);
  check(distance(characters.begin(), characters.end()) + 1 <= acct.max_characters, "no available characters");
  
  position default_position = position{1, 0, 15, 0};

  auto character_id = characters.available_primary_key();
  characters.emplace( user, [&](auto& c) {
    c.character_id = character_id;
    c.owner = user;
    c.character_name = character_name;
    c.gender = gender;
    c.profession = profession;
    c.race = race;
    c.movement_radius = 3.5;
    c.hp = base.base_hp;
    c.max_hp = base.base_hp;
    c.attack = base.base_attack;
    c.defense = base.base_defense;
    c.position = default_position;
    c.stats = base.base_stats;
  });

  mapdata_index mapdata(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  // Find starting map for character
  auto map = mapdata.get(0, "could't find map");
  mapdata_index u_mapdata(get_self(), user.value);
  map.mapdata_id = u_mapdata.available_primary_key();
  map.character_id = character_id;
  u_mapdata.emplace( user, [&](auto &m) {
    m = map;
  });
  
  charhistory_index charhistory(get_self(), user.value);
  charhistory.emplace(user, [&](auto& h) {
    h.character_id = character_id;
  });
}

ACTION ebonhaven::delcharacter( name user, uint64_t character_id ) {
  require_auth( user );

  characters_index characters(get_self(), user.value);
  charhistory_index history(get_self(), user.value);
  accounts_index accounts(get_self(), user.value);
  encounters_index encounters(get_self(), user.value);
  rewards_index rewards(get_self(), user.value);
  auto character = characters.get(character_id, "couldn't find character");
  auto acct = accounts.get(user.value, "couldn't find account");

  vector<uint64_t> to_delete;
  if (character.equipped.head != 0) {
      to_delete.push_back(character.equipped.head);
  }
  if (character.equipped.neck != 0) {
      to_delete.push_back(character.equipped.neck);
  }
  if (character.equipped.shoulders != 0) {
      to_delete.push_back(character.equipped.shoulders);
  }
  if (character.equipped.chest != 0) {
      to_delete.push_back(character.equipped.chest);
  }
  if (character.equipped.back != 0) {
      to_delete.push_back(character.equipped.chest);
  } 
  if (character.equipped.bracers != 0) {
      to_delete.push_back(character.equipped.bracers);
  } 
  if (character.equipped.hands != 0) {
      to_delete.push_back(character.equipped.hands);
  }
  if (character.equipped.waist != 0) {
      to_delete.push_back(character.equipped.waist);
  }
  if (character.equipped.legs != 0) {
      to_delete.push_back(character.equipped.legs);
  }
  if (character.equipped.feet != 0) {
      to_delete.push_back(character.equipped.feet);
  }
  if (character.equipped.weapon != 0) {
      to_delete.push_back(character.equipped.weapon);
  }
  if (character.equipped.ring1 != 0) {
      to_delete.push_back(character.equipped.ring1);
  }
  if (character.equipped.ring2 != 0) {
      to_delete.push_back(character.equipped.ring2);
  }
  if (character.equipped.trinket1 != 0) {
      to_delete.push_back(character.equipped.trinket1);
  }
  if (character.equipped.trinket2 != 0) {
      to_delete.push_back(character.equipped.trinket2);
  }

  if (to_delete.size() > 0) {
    action(
      permission_level{ user, name("active") },
      name("ebonhavencom"),
      name("burnnft"),
      make_tuple( user, to_delete )
    ).send();
  }

  vector<uint64_t> enc_to_delete;
  for (auto& enc: encounters) {
    if (enc.character_id == character_id) {
      enc_to_delete.push_back(enc.encounter_id);
    }
  }

  for (uint64_t del: enc_to_delete) {
    auto itr = encounters.find(del);
    if (itr != encounters.end()) {
      encounters.erase(itr);
    }
  }

  vector<uint64_t> rew_to_delete;
  for (auto& rew: rewards) {
    if (rew.character_id == character_id) {
      rew_to_delete.push_back(rew.reward_id);
    }
  }

  for (uint64_t del: rew_to_delete) {
    auto itr = rewards.find(del);
    if (itr != rewards.end()) {
      rewards.erase(itr);
    }
  }

  mapdata_index mapdata(get_self(), user.value);
  vector<uint64_t> mapdata_to_delete;
  for (auto& m: mapdata) {
    if (m.character_id == character_id) {
      mapdata_to_delete.push_back(m.mapdata_id);
    }
  }

  for (uint64_t m_del: mapdata_to_delete) {
    auto itr = mapdata.find(m_del);
    if (itr != mapdata.end()) {
      mapdata.erase(itr);
    }
  }
  
  auto itr = characters.find(character_id);
  characters.erase(itr);

  auto history_itr = history.find(character_id);
  history.erase(history_itr);
}

// TODO: Skip player turn if used in combat
ACTION ebonhaven::useitem( name user, uint64_t character_id, uint64_t dgood_id, uint8_t effect_idx = 0 ) {
    require_auth( user );

    characters_index characters(get_self(), user.value);
    accounts_index accounts(get_self(), user.value);
    effects_index effects(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
    
    dgoods_index items(get_self(), get_self().value);

    auto acct = accounts.get(user.value, "couldn't find account");
    auto c = characters.get(character_id, "couldn't find character");
    auto item = items.get(dgood_id, "couldn't find item");
    check(c.owner == user, "character doesn't belong to user");
    
    stats_index stats_table(get_self(), item.category.value);
    auto stat = stats_table.get(item.token_name.value, "couldn't find parent");
    check(stat.attributes.level_requirement <= c.level, "character level requirement not met");
    check(stat.attributes.is_key() != true, "not allowed. use unlock action");
    check(stat.attributes.is_chest() != true, "not allowed. use unlock action");
    check(stat.attributes.is_consumable() || stat.attributes.effects.size() > 0, "item cannot be used");
    
    lock_index lock_table( get_self(), get_self().value );
    auto locked_nft = lock_table.find( dgood_id );
    check( locked_nft == lock_table.end(), "token locked, cannot use item");

    auto effect = effects.get(stat.attributes.effects[effect_idx], "couldn't find effect");
    check(!effect.can_resurrect(), "cannot use items that ressurect. must use revive action");
    check(c.hp > 0, "cannot use items while dead");
    
    if( effect.effect_type == 0) {
      apply_effect_to_character( effect, c );
      auto itr = characters.find(character_id);
      characters.modify( itr, user, [&](auto& mod) {
          mod = c;
      });
      // LEARNRECIPE
    } else if (effect.effect_type == 4 ) {
      charhistory_index charhistory(get_self(), user.value);
      recipes_index recipes(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
      auto history = charhistory.get(character_id, "couldn't find history");
      auto recipe = recipes.get(effect.get_recipe_id(), "couldn't find recipe");
      if (recipe.profession_lock > 0) {
        check(c.profession == recipe.profession_lock, "character profession cannot learn this recipe");
      }
      check(history.profession_skill.craft >= recipe.min_skill, "crafting skill not high enough");
      auto h_itr = charhistory.find(character_id);
      bool found = (find(history.learned_recipes.begin(), history.learned_recipes.end(), recipe.recipe_id) != history.learned_recipes.end());
      check(found == false, "recipe already learned");
      history.learned_recipes.push_back(recipe.recipe_id);
      if ( h_itr != charhistory.end() ) {
        charhistory.modify( h_itr, user, [&](auto& h) {
          h.learned_recipes = history.learned_recipes;
        });
      }
      // EXPANDINVENTORY
    } else if (effect.effect_type == 8) {
      json j = json::parse(effect.effect_data);
      check(j.count("expand_inventory") > 0 && j.count("max_inventory") > 0, "effect not valid");
      check(j["max_inventory"] == acct.max_inventory, "account inventory size invalid");
      auto a_itr = accounts.find(user.value);
      if (a_itr != accounts.end()) {
        accounts.modify( a_itr, user, [&](auto& a) {
          a.max_inventory = j["expand_inventory"].get<uint8_t>();
        });
      }
    }

    // Burn NFT if consumable
    if (stat.attributes.is_consumable()) {
      vector<uint64_t> v = { item.id };
      action(
        permission_level{ user, name("active") },
        name("ebonhavencom"),
        name("burnnft"),
        make_tuple( user, v )
      ).send();
    }   
};

ACTION ebonhaven::equipitem( name user, uint64_t character_id, uint64_t dgood_id, uint8_t equip_slot ) {
  require_auth( user );

  characters_index characters_table(get_self(), user.value);
  accounts_index accounts_table(get_self(), user.value);
  dgoods_index items(get_self(), get_self().value);
  lock_index lock_table( get_self(), get_self().value );
  auto locked_nft = lock_table.find( dgood_id );
  check( locked_nft == lock_table.end(), "cannot equip listed items" );

  auto acct = accounts_table.get(user.value, "couldn't find account");
  auto c = characters_table.get(character_id, "couldn't find character");
  check(c.status != 4, "character cannot equip items in combat");
  check(c.hp > 0, "cannot equip items while dead");
  auto item = items.get(dgood_id, "couldn't find item");
  
  reset_stats( c );

  // Unequip
  if (equip_slot == 0) {
    // Check if inventory space available
    check(inventory_count(user) < acct.max_inventory, "not enough inventory space");

    item.equipped = false;

    if (c.equipped.head == item.id) { c.equipped.head = 0; }
    else if (c.equipped.neck == item.id) { c.equipped.neck = 0; }
    else if (c.equipped.shoulders == item.id) { c.equipped.shoulders = 0; }
    else if (c.equipped.chest == item.id) { c.equipped.chest = 0; }
    else if (c.equipped.back == item.id) { c.equipped.back = 0; }
    else if (c.equipped.bracers == item.id) { c.equipped.bracers = 0; }
    else if (c.equipped.hands == item.id) { c.equipped.hands = 0; }
    else if (c.equipped.waist == item.id) { c.equipped.waist = 0; }
    else if (c.equipped.legs == item.id) { c.equipped.legs = 0; }
    else if (c.equipped.feet == item.id) { c.equipped.feet = 0; }
    else if (c.equipped.weapon == item.id) { c.equipped.weapon = 0; }
    else if (c.equipped.ring1 == item.id) { c.equipped.ring1 = 0; }
    else if (c.equipped.ring2 == item.id) { c.equipped.ring2 = 0; }
    else if (c.equipped.trinket1 == item.id) { c.equipped.trinket1 = 0; }
    else if (c.equipped.trinket2 == item.id) { c.equipped.trinket2 = 0; }
    else {
      check(false, "couldn't find equipped item");
    }

  } else {
    
    stats_index stats_table(get_self(), item.category.value);
    auto stat = stats_table.get(item.token_name.value, "couldn't find parent");
    check(stat.attributes.slot == equip_slot, "item can't be equipped in this slot");
    
    dgood old_item;
    
    switch (equip_slot) {
      case 1:
        if (c.equipped.head != 0 ) { old_item = items.get(c.equipped.head); }
        c.equipped.head = item.id;
        break;
      case 2:
        if (c.equipped.neck != 0 ) { old_item = items.get(c.equipped.neck); }
        c.equipped.neck = item.id;
        break;
      case 3:
        if (c.equipped.shoulders != 0 ) { old_item = items.get(c.equipped.shoulders); }
        c.equipped.shoulders = item.id;
        break;
      case 4:
        if (c.equipped.chest != 0 ) { old_item = items.get(c.equipped.chest); }
        c.equipped.chest = item.id;
        break;
      case 5:
        if (c.equipped.back != 0 ) { old_item = items.get(c.equipped.back); }
        c.equipped.back = item.id;
        break;
      case 6:
        if (c.equipped.bracers != 0 ) { old_item = items.get(c.equipped.bracers); }
        c.equipped.bracers = item.id;
        break;
      case 7:
        if (c.equipped.hands != 0 ) { old_item = items.get(c.equipped.hands); }
        c.equipped.hands = item.id;
        break;
      case 8:
        if (c.equipped.waist != 0 ) { old_item = items.get(c.equipped.waist); }
        c.equipped.waist = item.id;
        break;
      case 9:
        if (c.equipped.legs != 0 ) { old_item = items.get(c.equipped.legs); }
        c.equipped.legs = item.id;
        break;
      case 10:
        if (c.equipped.feet != 0 ) { old_item = items.get(c.equipped.feet); }
        c.equipped.feet = item.id;
        break;
      case 11:
        if (c.equipped.weapon != 0 ) { old_item = items.get(c.equipped.weapon); }
        c.equipped.weapon = item.id;
        break;
      case 12:
        if (c.equipped.ring1 != 0 ) { old_item = items.get(c.equipped.ring1); }
        c.equipped.ring1 = item.id;
        break;
      case 14:
        if (c.equipped.ring2 != 0 ) { old_item = items.get(c.equipped.ring2); }
        c.equipped.ring2 = item.id;
        break;
      case 15:
        if (c.equipped.trinket1 != 0 ) { old_item = items.get(c.equipped.trinket1); }
        c.equipped.trinket1 = item.id;
        break;
      case 16:
        if (c.equipped.trinket2 != 0 ) { old_item = items.get(c.equipped.trinket2); }
        c.equipped.trinket2 = item.id;
        break;
      default:
        check(false, "equipment slot not found");
        break;
    }
    
    item.equipped = true;
    
    // Store old item as unequipped
    check(old_item.id != item.id, "cannot equip same item");
    auto oi_itr = items.find(old_item.id);
    if (oi_itr != items.end()) {
      items.modify( oi_itr, get_self(), [&](auto& oi) {
        oi.equipped = false;
      });
    }

    // Bind item if bind on equip
    if (stat.attributes.item_binding == 1 && item.attributes.bind_status == 1) {
      item.attributes.bind_status = 2;
    }
  }
  
  // Update item
  auto i_itr = items.find(item.id);
  items.modify( i_itr, get_self(), [&](auto& i) {
    i = item;
  });

  // Add static item auras
  add_item_stats( user, c );

  auto c_itr = characters_table.find(character_id);
  characters_table.modify( c_itr, user, [&](auto& row) {
    row.attack = c.attack;
    row.defense = c.defense;
    row.stats = c.stats;
    row.equipped = c.equipped;
  });
}

// TODO: Restrict to radius around allowed ability vendors coordinate 
ACTION ebonhaven::buyability( name user, uint64_t character_id, uint64_t ability_id )
{
  require_auth( user );

  characters_index characters(get_self(), user.value);
  accounts_index accounts(get_self(), user.value);
  abilities_index abilities(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  charhistory_index history(get_self(), user.value);

  auto c = characters.get(character_id, "couldn't find character");
  check(c.status == 0, "character status doesn't allow buying abilities");
  check(c.hp > 0, "cannot buy abilities while dead");
  check(user == c.owner, "character does not belong to user");
  auto ability = abilities.get(ability_id, "couldn't find ability");
  auto info = history.get(character_id, "couldn't find character info");
  check(find(info.learned_abilities.begin(), info.learned_abilities.end(), ability_id) == info.learned_abilities.end(), "character ability already learned");
  if (ability.profession_lock > 0) {
    check(c.profession == ability.profession_lock, "character ability cannot be learned by this profession");  
  }
  if (ability.race_lock > 0) {
    check(c.race == ability.race_lock, "character ability cannot be learned by this race");
  }
  
  action(
    permission_level{ user ,"active"_n},
    get_self(),
    "tokenxfer"_n,
    std::make_tuple( user, get_self(), ability.ability_cost, string("buy ability") )
  ).send();

  info.learned_abilities.push_back(ability_id);

  auto itr = history.find(character_id);
  history.modify( itr, user, [&](auto& row) {
    row.learned_abilities = info.learned_abilities;
  });
}

ACTION ebonhaven::equipability( name user, uint64_t character_id, uint64_t ability_id, uint8_t ability_idx )
{
  require_auth( user );

  characters_index characters(get_self(), user.value);
  accounts_index accounts(get_self(), user.value);
  abilities_index abilities(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  charhistory_index history(get_self(), user.value);

  auto c = characters.get(character_id, "couldn't find character");
  check(user == c.owner, "character does not belong to user");
  check(c.status == 0, "character status doesn't allow equipping abilities");
  check(c.hp > 0, "cannot equip abilities while dead");
  auto ability = abilities.get(ability_id, "couldn't find ability");
  auto info = history.get(character_id, "couldn't find character info");
  check(find(info.learned_abilities.begin(), info.learned_abilities.end(), ability_id) != info.learned_abilities.end(), "character has not learned this ability");
  check(c.level >= ability.level, "level requirement not met");
  check(c.profession == ability.profession_lock, "character ability cannot be equipped by this profession");
  if (ability_idx == 4) {
    check(ability.race_lock > 0, "ability index reserved for race ability");
  }
  if (ability.race_lock > 0) {
    check(c.race == ability.race_lock, "character ability cannot be equipped by this race");
    check(ability_idx == 4, "ability index reserved for race ability");
  }

  ebonhaven::charhistory empty = {};
  if (c.abilities.ability1 == ability_id) {
    c.abilities.ability1 = 0;
  } else if (c.abilities.ability2 == ability_id) {
    c.abilities.ability2 = 0;
  } else if (c.abilities.ability3 == ability_id) {
    c.abilities.ability3 = 0;
  } else if (c.abilities.racial == ability_id ) {
    c.abilities.racial = 0;
  }
  
  if ( ability_idx > 0 && ability_idx <= 4 ) {
    switch ( ability_idx ) {
      case 1:
        c.abilities.ability1 = ability_id;
        break;
      case 2:
        c.abilities.ability2 = ability_id;
        break;
      case 3:
        c.abilities.ability3 = ability_id;
        break;
      case 4:
        c.abilities.racial = ability_id;
        break;
    }
  } else if ( ability_idx != 0 ) {
    check(false, "ability index not found");
  }

  auto itr = characters.find(character_id);
  characters.modify( itr, user, [&](auto& row) {
      row.abilities = c.abilities;
  });
}

ACTION ebonhaven::unlock( name user, uint64_t key_id, uint64_t chest_id )
{
  require_auth( user );
  dgoods_index items(get_self(), get_self().value);
  stats_index stats_table(get_self(), get_self().value);
  lock_index lock_table( get_self(), get_self().value );
  auto key = items.get(key_id, "couldn't find key");
  auto key_stat = stats_table.get(key.token_name.value, "couldn't find key stats");
  check(key_stat.attributes.is_key(), "incorrect item type: key");
  check(key_stat.attributes.effects.size() > 0, "no key effect found");
  auto locked_nft_key = lock_table.find( key.id );
  check( locked_nft_key == lock_table.end(), "token locked, cannot unlock using this item");
  auto chest = items.get(chest_id, "couldn't find chest");
  auto chest_stat = stats_table.get(chest.token_name.value, "couldn't find chest stats");
  check(chest_stat.attributes.is_chest(), "incorrect item type: chest");
  auto locked_nft_chest = lock_table.find( chest.id );
  check( locked_nft_chest == lock_table.end(), "token locked, cannot unlock using this item");
  
  effects_index effects(ADMIN_CONTRACT, ADMIN_CONTRACT.value);

  auto key_effect = effects.get(key_stat.attributes.effects[0], "couldn't find key effect");
  json j = json::parse(key_effect.effect_data);
  vector<string> unlocks;
  if (j.count("unlocks") > 0) {
    for (auto& el: j["unlocks"]) {
      unlocks.push_back(el);
    }
  } else {
    check(false, "no unlocks key found in effect");
  }

  bool found = find(unlocks.begin(), unlocks.end(), chest.token_name.to_string()) != unlocks.end();
  check(found, "chest cannot be unlocked by this key");

  auto chest_effect = effects.get(chest_stat.attributes.effects[0], "couldn't find chest effect");
  j = json::parse(chest_effect.effect_data);
  uint64_t drop_id;
  if (j.count("drop") > 0) {
    drop_id = j["drop"].get<uint64_t>();
  } else {
    check(false, "no drop id found in chest effect");
  }

  drops_index drops(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  auto drop = drops.get(drop_id, "couldn't find drop");

  rewards_index rewards(get_self(), user.value);
  reward r = {};
  r.reward_id = rewards.available_primary_key();
  if (r.reward_id == 0) { r.reward_id = 1; }
  r.worth = asset(0, symbol(symbol_code("EBON"),2));
  auto roll = random(100);
  print("Drop roll: ", roll);
  if (drop.min_worth.amount > 0) {
    auto amt = random(drop.max_worth.amount - drop.min_worth.amount);
    r.worth.amount = drop.min_worth.amount + amt;
  }
  for (auto& drop: drop.drops) {
    uint8_t perc = abs(floor((100 * drop.percentage) / 100));
    // print(" Rolled on drop: ", perc);
    if ( roll <= perc ) {
      print(" Item won: ", drop.token_name);
      r.items.push_back(drop.token_name);
    }
  }

  vector<uint64_t> to_burn = {
    key.id, chest.id
  };

  action(
    permission_level{ user, name("active") },
    name("ebonhavencom"),
    name("burnnft"),
    make_tuple( user, to_burn )
  ).send();

  rewards.emplace( user, [&](auto& reward) {
    reward = r; 
  });

}

void ebonhaven::reset_stats( ebonhaven::character& c ) {
  basestats_index basestats(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  auto& base = basestats.get(c.profession, "Base stats for profession does not exist");

  c.attack.physical = base.base_attack.physical + (base.attack_increase.physical * c.level); 
  c.attack.spell = base.base_attack.spell + (base.attack_increase.spell * c.level);
  c.defense.physical = base.base_defense.physical + (base.defense_increase.physical * c.level);
  c.defense.spell = base.base_defense.physical + (base.defense_increase.spell * c.level);
  c.stats.stamina = base.base_stats.stamina + (base.stats_increase.stamina * c.level);
  c.stats.regen = base.base_stats.regen + (base.stats_increase.regen * c.level);
  c.stats.perception = base.base_stats.perception + (base.stats_increase.perception * c.level);
  c.stats.skill = base.base_stats.skill + (base.stats_increase.skill * c.level);
  c.stats.luck = base.base_stats.luck + (base.stats_increase.luck * c.level);
}

void ebonhaven::add_item_stats( name user, ebonhaven::character& c ) {
  if (c.equipped.head > 0) { apply_item_auras( user, c, c.equipped.head ); }
  if (c.equipped.neck > 0) { apply_item_auras( user, c, c.equipped.neck ); }
  if (c.equipped.shoulders > 0) { apply_item_auras( user, c, c.equipped.shoulders ); }
  if (c.equipped.chest > 0) { apply_item_auras( user, c, c.equipped.chest ); }
  if (c.equipped.bracers > 0) { apply_item_auras( user, c, c.equipped.bracers ); }
  if (c.equipped.hands > 0) { apply_item_auras( user, c, c.equipped.hands ); }
  if (c.equipped.waist > 0) { apply_item_auras( user, c, c.equipped.waist ); }
  if (c.equipped.legs > 0) { apply_item_auras( user, c, c.equipped.legs ); }
  if (c.equipped.feet > 0) { apply_item_auras( user, c, c.equipped.feet ); }
  if (c.equipped.weapon > 0) { apply_item_auras( user, c, c.equipped.weapon ); }
  if (c.equipped.ring1 > 0) { apply_item_auras( user, c, c.equipped.ring1 ); }
  if (c.equipped.ring2 > 0) { apply_item_auras( user, c, c.equipped.ring2 ); }
  if (c.equipped.trinket1 > 0) { apply_item_auras( user, c, c.equipped.trinket1 ); }
  if (c.equipped.trinket2 > 0) { apply_item_auras( user, c, c.equipped.trinket2 ); }
}

void ebonhaven::apply_item_auras( name user, ebonhaven::character& c, uint64_t item_id ) {
  dgoods_index items(get_self(), get_self().value);
  auto item = items.get( item_id, "couldn't find item" );
  auras_index auras(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  stats_index stats(get_self(), item.category.value);
  auto stat = stats.get(item.token_name.value, "couldn't find parent");

  for ( auto a: stat.attributes.auras ) {
    auto aura = auras.get(a, "Couldn't find aura");

    // Apply static item auras only
    if (aura.aura_type == 0) {
      auto j = json::parse(aura.aura_data);
      if (j.count("attack") > 0) {
        if (j["attack"].count("physical") > 0) {
          c.attack.physical += j["attack"]["physical"].get<uint32_t>();
        }
        if (j["attack"].count("spell") > 0) {
          c.attack.spell += j["attack"]["spell"].get<uint32_t>();
        }
    }
    if (j.count("defense") > 0) {
        if (j["defense"].count("physical") > 0) {
          c.attack.physical += j["attack"]["physical"].get<uint32_t>();
        }
        if (j["defense"].count("spell") > 0) {
          c.attack.spell += j["attack"]["spell"].get<uint32_t>();
        }
      }
      if (j.count("stamina") > 0) {
          c.stats.stamina += j["stamina"].get<uint32_t>();
      }
      if (j.count("regen") > 0) {
          c.stats.regen += j["regen"].get<uint32_t>();
      }
      if (j.count("perception") > 0) {
          c.stats.perception += j["perception"].get<uint32_t>();
      }
      if (j.count("skill") > 0) {
          c.stats.skill += j["skill"].get<uint32_t>();
      }
      if (j.count("luck") > 0) {
          c.stats.luck += j["luck"].get<uint32_t>();
      }
    }
  }
}

void ebonhaven::apply_effect_to_character( effect e, character& c ) {
    auto j = json::parse(e.effect_data);
    if (j.count("hp_delta") > 0) {
        auto result = c.hp += j["hp_delta"].get<uint64_t>();
        if (result >= c.max_hp) {
            c.hp = c.max_hp;
        } else {
            c.hp = result;
        }
    }
}