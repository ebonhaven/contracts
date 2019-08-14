#include <admin.hpp>

ACTION admin::upsaura( uint64_t aura_id,
                       string aura_name, 
                       string aura_description,
                       uint8_t aura_type,
                       uint8_t is_hidden,
                       uint8_t cooldown, 
                       string& aura_data )
{
  require_auth( get_self() );
  auras_index auras(get_self(), get_self().value);
  auto itr = auras.find(aura_id);
  if ( itr == auras.end() ) {
    auras.emplace( get_self(), [&](auto& a) {
      a.aura_id = aura_id;
      a.aura_name = aura_name;
      a.aura_description = aura_description;
      a.is_hidden = is_hidden;
      a.aura_type = aura_type;
      a.cooldown = cooldown;
      a.aura_data = aura_data;
    });
  } else {
    auras.modify( itr, get_self(), [&](auto& a) {
      a.aura_name = aura_name;
      a.aura_description = aura_description;
      a.is_hidden = is_hidden;
      a.aura_type = aura_type;
      a.cooldown = cooldown;
      a.aura_data = aura_data;
    });
  }
}

// Admin
ACTION admin::upsability( uint64_t ability_id,
                              string ability_name,
                              string ability_description,
                              asset ability_cost,
                              uint32_t level,
                              uint8_t profession_lock,
                              uint8_t race_lock,
                              string ability_data )
{
  require_auth ( get_self() );
  abilities_index abilities(get_self(), get_self().value);
  auto itr = abilities.find(ability_id);
  if ( itr == abilities.end() ) {
    abilities.emplace( get_self(), [&](auto& a) {
      a.ability_id = ability_id;
      a.ability_name = ability_name;
      a.ability_description = ability_description;
      a.ability_cost = ability_cost;
      a.level = level;
      a.profession_lock = profession_lock;
      a.race_lock = race_lock;
      a.ability_data = ability_data;
    });
  } else {
    abilities.modify( itr, get_self(), [&](auto& a) {
      a.ability_name = ability_name;
      a.ability_description = ability_description;
      a.ability_cost = ability_cost;
      a.level = level;
      a.profession_lock = profession_lock;
      a.race_lock = race_lock;
      a.ability_data = ability_data;
    });
  }
}

// Admin
ACTION admin::upseffect(uint64_t effect_id,
                        uint8_t effect_type,
                        float_t chance,
                        uint8_t cooldown,
                        string& effect_data )
{
  require_auth( get_self() );
  effects_index effects( get_self(), get_self().value );

  auto itr = effects.find(effect_id);
  if ( itr == effects.end() ) {
    effects.emplace( get_self(), [&](auto& e) {
      e.effect_id = effect_id;
      e.effect_type = effect_type;
      e.chance = chance;
      e.cooldown = cooldown;
      e.effect_data = effect_data;
    });
  } else {
    effects.modify( itr, get_self(), [&](auto& e) {
      e.effect_type = effect_type;
      e.chance = chance;
      e.cooldown = cooldown;
      e.effect_data = effect_data;
    });
  }
}

ACTION admin::upsstats(uint8_t profession,
                       uint8_t base_hp,
                       uint8_t hp_increase,
                       attack  base_attack,
                       defense base_defense,
                       attack  attack_increase,
                       defense defense_increase,
                       stats   base_stats,
                       stats   stats_increase)
{
  require_auth( get_self() );
  
  basestats_index basestats(get_self(), get_self().value);
  auto itr = basestats.find(profession);
  
  if (itr == basestats.end() ) {
    basestats.emplace( get_self(), [&](auto& b) {
      b.profession_id = profession;
      b.base_hp = base_hp;
      b.hp_increase = hp_increase;
      b.base_attack = base_attack;
      b.attack_increase = attack_increase;
      b.base_defense = base_defense;
      b.defense_increase = defense_increase;
      b.base_stats = base_stats;
      b.stats_increase = stats_increase;
    });
  } else {
    basestats.modify( itr, get_self(), [&](auto& b) {
      b.base_hp = base_hp;
      b.hp_increase = hp_increase;
      b.base_attack = base_attack;
      b.attack_increase = attack_increase;
      b.base_defense = base_defense;
      b.defense_increase = defense_increase;
      b.base_stats = base_stats;
      b.stats_increase = stats_increase;
    });
  }
}

ACTION admin::upsdrop( uint64_t drop_id, asset min_worth, asset max_worth, vector<item_drop> item_drops)
{
  require_auth( get_self() );
  drops_index drops(get_self(), get_self().value);
  
  for ( auto& drop: item_drops ) {
    drop.percentage = floor( drop.percentage * 150 ) / 150;
  }

  auto itr = drops.find( drop_id );
  if ( itr == drops.end() ) {
    drops.emplace( get_self(), [&](auto& d) {
      d.drop_id = drop_id;
      d.min_worth = min_worth;
      d.max_worth = max_worth;
      d.drops = item_drops;
    });
  } else {
    drops.modify( itr, get_self(), [&](auto& d) {
      d.min_worth = min_worth;
      d.max_worth = max_worth;
      d.drops = item_drops;
    });
  }
};

ACTION admin::upsresource( name resource_name,
                           uint8_t  profession_id,
                           uint32_t experience,
                           uint32_t min_skill,
                           vector<resource_drop> drops )
{
  require_auth( get_self() );
  
  resources_index resources( get_self(), get_self().value );
  auto itr = resources.find(resource_name.value);
  if ( itr == resources.end() ) {
    resources.emplace( get_self(), [&](auto& r) {
      r.resource_name = resource_name;
      r.profession_id = profession_id;
      r.experience = experience;
      r.min_skill = min_skill;
      r.drops = drops;
    });
  } else {
    resources.modify( itr, get_self(), [&](auto& r) {
      r.profession_id = profession_id;
      r.experience = experience;
      r.min_skill = min_skill;
      r.drops = drops;
    });
  }
}

// Admin
ACTION admin::upstreasure( uint64_t world_zone_id, vector<item_drop> drops ) {
  require_auth( get_self() );
  
  treasures_index treasures( get_self(), get_self().value );
  auto itr = treasures.find( world_zone_id );
  if (itr == treasures.end() ) {
    treasures.emplace( get_self(), [&](auto& t) {
      t.world_zone_id = world_zone_id;
      t.drops = drops;
    });
  } else {
    treasures.modify( itr, get_self(), [&](auto& t) {
      t.drops = drops;
    });
  }
}

ACTION admin::upsmapdata( uint64_t world_zone_id,
                          position respawn,
                          vector<tiledata> tiles,
                          vector<trigger> triggers,
                          vector<mobdata> mobs,
                          vector<npcdata> npcs,
                          vector<zone_drop> resources,
                          rate_mod rate_modifier )
{
  require_auth( get_self() );
  
  vector<tiledata> updated_tiles;
  for (auto& ti: tiles) {
    tiledata new_tile {
      ti.coordinates,
      ti.attributes
    };
    updated_tiles.push_back(new_tile);
  }
  
  vector<trigger> updated_triggers;
  for (auto& tr: triggers) {
    trigger new_trigger {
      tr.coordinates,
      tr.radius,
      tr.attributes
    };
    updated_triggers.push_back(new_trigger);
  }
  
  vector<mobdata> updated_mobs;
  for (auto& m: mobs) {
    mobdata new_mob {
      m.coordinates,
      m.mob_id,
      m.status,
      m.radius,
      m.attributes
    };
    updated_mobs.push_back(new_mob);
  }
  
  vector<npcdata> updated_npcs;
  for (auto& n: npcs) {
    npcdata new_npc {
      n.coordinates,
      n.npc_id,
      n.radius,
      n.attributes
    };
    updated_npcs.push_back(new_npc);
  }

  vector<zone_drop> updated_resources;
  for (auto& r: resources) {
    zone_drop new_drop {
      r.profession_id,
      r.resource_name
    };
    updated_resources.push_back(new_drop);
  }

  rate_mod updated_rates {
    rate_modifier.combat,
    rate_modifier.resource,
    rate_modifier.secret,
    rate_modifier.trap,
    rate_modifier.treasure,
    rate_modifier.loot
  };

  mapdata_index mapdata(get_self(), get_self().value);
  auto itr = mapdata.find(world_zone_id);
  if ( itr == mapdata.end() ) {
    mapdata.emplace( get_self(), [&](auto& m) {
      m.world_zone_id = world_zone_id;
      m.respawn = respawn;
      m.tiles = updated_tiles;
      m.triggers = updated_triggers;
      m.mobs = updated_mobs;
      m.npcs = updated_npcs;
      m.resources = updated_resources;
      m.rate_modifier = updated_rates;
    });
  } else {
    mapdata.modify( itr, get_self(), [&](auto& m) {
      m.world_zone_id = world_zone_id;
      m.respawn = respawn;
      m.tiles = updated_tiles;
      m.triggers = updated_triggers;
      m.mobs = updated_mobs;
      m.npcs = updated_npcs;
      m.resources = updated_resources;
      m.rate_modifier = updated_rates;
    });
  }
}

ACTION admin::upsmob( uint32_t mob_id,
                      string   mob_name,
                      uint8_t  level,
                      uint8_t  mob_type,
                      attack   attack,
                      defense  defense,
                      string   mob_data,
                      uint32_t hp,
                      uint32_t experience,
                      asset    worth,
                      uint64_t drop_id )
{
  require_auth( get_self() );

  mobs_index mobs(get_self(), get_self().value);
  auto itr = mobs.find(mob_id); 

  if ( itr == mobs.end() ) {
    mobs.emplace( get_self(), [&]( auto& m ) {
      m.mob_id = mob_id;
      m.mob_name = mob_name;
      m.level = level;
      m.mob_type = mob_type;
      m.attack = attack;
      m.defense = defense;
      m.mob_data = mob_data;
      m.hp = hp;
      m.max_hp = hp;
      m.experience = experience;
      m.worth = worth;
      m.drop_id = drop_id;
    });
  } else {
    mobs.modify( itr, get_self(), [&]( auto& m ) {
      m.mob_id = mob_id;
      m.mob_name = mob_name;
      m.level = level;
      m.mob_type = mob_type;
      m.attack = attack;
      m.defense = defense;
      m.mob_data = mob_data;
      m.hp = hp;
      m.max_hp = hp;
      m.experience = experience;
      m.worth = worth;
      m.drop_id = drop_id;
    });
  }
};

ACTION admin::upsnpc( uint64_t npc_id,
                      vector<name> quests,
                      vector<uint64_t> triggers )
{
  require_auth(get_self());

  npcs_index npcs(get_self(), get_self().value);
  auto itr = npcs.find(npc_id);
  if (itr != npcs.end()) {
    npcs.modify( itr, get_self(), [&](auto& n) {
      n.quests = quests;
      n.triggers = triggers;
    });
  } else {
    npcs.emplace( get_self(), [&](auto& n) {
      n.npc_id = npc_id;
      n.quests = quests;
      n.triggers = triggers;
    });
  }
}

ACTION admin::upsprogress( uint8_t level, uint64_t experience )
{
  require_auth( get_self() );

  progress_index progress(get_self(), get_self().value);
  auto itr = progress.find(level);
  if (itr != progress.end()) {
    progress.modify( itr, get_self(), [&](auto& p) {
      p.experience = experience;
    });
  } else {
    progress.emplace( get_self(), [&](auto& p) {
      p.level = level;
      p.experience = experience;
    });
  }
}

ACTION admin::upsrecipe( uint64_t recipe_id,
                         name category,
                         name token_name,
                         uint8_t profession_lock,
                         uint32_t min_skill,
                         vector<requirement> requirements )
{
  require_auth(get_self());

  vector<requirement> updated_requirements;
  for (auto& r: requirements) {
    requirement new_req {
      r.token_name,
      r.quantity
    };
    updated_requirements.push_back(new_req);
  }

  recipes_index recipes( get_self(), get_self().value );
  auto itr = recipes.find(recipe_id);
  if (itr != recipes.end()) {
    recipes.modify( itr, get_self(), [&](auto& r) {
      r.category = category;
      r.token_name = token_name;
      r.profession_lock = profession_lock;
      r.min_skill = min_skill;
      r.requirements = updated_requirements;
    });
  } else {
    recipes.emplace( get_self(), [&](auto& r) {
      r.recipe_id = recipe_id;
      r.category = category;
      r.token_name = token_name;
      r.profession_lock = profession_lock;
      r.min_skill = min_skill;
      r.requirements = updated_requirements;
    });
  }
}

ACTION admin::upsquest( name user,
                        uint64_t character_id,
                        name quest_name,
                        uint64_t begin_npc_id,
                        uint8_t min_level,
                        uint64_t complete_npc_id,
                        asset worth,
                        uint32_t experience,
                        vector<name> rewards,
                        uint8_t repeatable,
                        vector<uint64_t> prerequisites,
                        uint8_t profession_lock,
                        uint8_t race_lock,
                        vector<objective> objectives )
{
  require_auth( get_self() );

  vector<objective> updated_objectives;
  for (auto& o: objectives) {
    objective new_obj {
      o.objective_id,
      o.objective_type,
      o.completed,
      o.objective_data
    };
    updated_objectives.push_back(new_obj);
  }

  quests_index quests(get_self(), get_self().value);
  auto itr = quests.find(quest_name.value);
  if (itr != quests.end()) {
    quests.modify(itr, get_self(), [&](auto& q) {
      q.quest_name = quest_name;
      q.character_id = character_id;
      q.begin_npc_id = begin_npc_id;
      q.min_level = min_level;
      q.complete_npc_id = complete_npc_id;
      q.worth = worth;
      q.experience = experience;
      q.rewards = rewards;
      q.repeatable = repeatable;
      q.prerequisites = prerequisites;
      q.profession_lock = profession_lock;
      q.race_lock = race_lock;
      q.objectives = updated_objectives;
    });
  } else {
    auto id = quests.available_primary_key();
    if (id == 0) { id++; }
    quests.emplace( get_self(), [&](auto& q) {
      q.quest_id = id;
      q.quest_name = quest_name;
      q.character_id = character_id;
      q.begin_npc_id = begin_npc_id;
      q.min_level = min_level;
      q.complete_npc_id = complete_npc_id;
      q.worth = worth;
      q.experience = experience;
      q.rewards = rewards;
      q.repeatable = repeatable;
      q.prerequisites = prerequisites;
      q.profession_lock = profession_lock;
      q.race_lock = race_lock;
      q.objectives = updated_objectives;
    });
  }
}