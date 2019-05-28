#include <ebonhavencom.hpp>

// Admin
ACTION ebonhavencom::upsaura( uint64_t aura_id,
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
ACTION ebonhavencom::upsability( uint64_t ability_id,
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
ACTION ebonhavencom::upstreasure( uint64_t world_zone_id, vector<item_drop> drops ) {
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

// Admin
ACTION ebonhavencom::upseffect(uint64_t effect_id,
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

// Admin
ACTION ebonhavencom::upsstats(uint8_t profession,
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

ACTION ebonhavencom::upsdrop( uint64_t drop_id, vector<item_drop> item_drops)
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
      d.drops = item_drops;
    });
  } else {
    drops.modify( itr, get_self(), [&](auto& d) {
      d.drops = item_drops;
    });
  }
};

// Admin
ACTION ebonhavencom::spawnitem( name to, name token_name )
{
  require_auth( get_self() );
  
  accounts_index accounts(get_self(), to.value);
  auto acct = accounts.get(to.value, "account not found");
  
  check(inventory_count(to) <= acct.max_inventory - 1, "not enough inventory space");
  action(
    permission_level{ get_self(), name("active") },
    name("ebonhavencom"),
    name("issue"),
    make_tuple( to, name("ebonhavencom"), token_name, string("1"), string("1"), string(""), string("issued from ebonhavencom"))
  ).send();
}

// Admin
ACTION ebonhavencom::spawnability( name user, uint64_t character_id, uint64_t ability_id )
{
  require_auth( get_self() );

  characters_index characters(get_self(), user.value);
  abilities_index abilities(get_self(), get_self().value);
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
