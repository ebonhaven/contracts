#pragma once

using namespace eosio;
using namespace std;

namespace types {
  
  enum class gender: uint8_t {
    MALE = 0,
    FEMALE = 1
  };
  
  enum class race: uint8_t {
    NONE = 0,
    HUMAN = 1,
    EARTHEN = 2,
    AMAZON = 3,
    HIGHBORNE = 4,
    GOBLIN = 5,
    MECHA = 6
  };
  
  enum class profession: uint8_t {
    NONE = 0,
    ROCKHOUND = 1,
    SURVIVALIST = 2,
    ALCHEMIST = 3,
    CONJUROR = 4,
    METALSMITH = 5
  };
  
  enum class outcomes: uint8_t {
    NONE = 0,
    DISOVERY = 1,
    RESOURCE = 2,
    TRAP = 3,
    COMBAT = 4,
    TREASURE = 5,
    LOOT = 6
  };
  
  enum class encounter_status: uint8_t {
    IN_PROGRESS = 0,
    WIN = 1,
    LOSE = 2
  };
  
  enum class combat_decision: uint8_t {
    NONE = 0,
    MELEE = 1,
    RANGED = 2,
    ABILITY1 = 3,
    ABILITY2 = 4,
    ABILITY3 = 5,
    RACEABILITY = 6,
    FLEE = 7,
    BRIBE = 8
  };
  
  enum class combat_target: uint8_t {
    ENEMY = 0,
    ALL = 1,
    SELF = 2,
    ALLY = 3
  };
  
  enum class effect_type: uint8_t {
    APPLYEFFECT = 0,
    APPLYAURA = 1,
    ITEMEFFECT = 2,
    LEARNABILITY = 3,
    LEARNRECIPE = 4,
    DROPREWARD = 5
  };
  
  enum class mob_type: uint8_t {
    BASIC = 0,
    FLYING = 1,
    ARMORED = 2,
    ETHEREAL = 3,
    ELITE = 4,
    BOSS = 5
  };

  enum class objective_type: uint8_t {
    KILL = 0,
    COLLECTION = 1,
    COURIER = 2,
    PROFESSION = 3,
    ESCORT = 4,
    REPUTATION = 5,
    EXPLORATION = 6
  };
  
  struct attack {
    uint32_t physical = 0;
    uint32_t spell = 0;
  };
  
  struct defense {
    uint32_t physical = 0;
    uint32_t spell = 0;
  };
  
  struct skill {
    uint32_t gather = 1;
    uint32_t craft = 1;
  };
  
  struct position {
    uint64_t world_zone_id = 1;
    uint8_t  x = 0;
    uint8_t  y = 0;
    uint8_t  orientation = 0;
  };
  
  struct stats {
    // Increase max health
    uint32_t stamina = 0;
    // Increase health regeneration
    uint32_t regen = 0;
    // Improved chance of resource encounter
    uint32_t perception = 0;
    // Improved chance of crafting additional items
    uint32_t skill = 0;
    // Improved chance of finding treasure
    uint32_t luck = 0;
  };

  struct rate_mod {
    float_t        combat = 1.00;
    float_t        resource = 1.00;
    float_t        secret = 1.00;
    float_t        trap = 1.00;
    float_t        treasure = 1.00;
    float_t        loot = 1.00;
  };
  
  struct rates {
    float_t        combat = 0.15;
    float_t        resource = 0.09;
    float_t        secret = 0.025;
    float_t        trap = 0.017;
    float_t        treasure = 0.005;
    float_t        loot = 0.0015;
    time_point_sec updated_at;
  };
  
  struct mod_mob {
    uint32_t mob_id;
    string   mob_name;
    int32_t  level = 1;
    uint8_t  mob_type = 0;
    uint8_t  last_decision = 0;
    uint8_t  last_decision_hit = 1;
    attack   attack;
    defense  defense;
    string   mob_data;
    uint32_t hp = 0;
    uint32_t max_hp = 0;
    uint32_t experience = 0;
    asset    worth = asset(0, symbol(symbol_code("EBON"),2));
    uint64_t drop_id;
  };
  
  struct item_drop {
    float_t percentage = 0.00;
    name    token_name;
  };
  
  struct resource_drop {
    float_t percentage = 0.00;
    name    token_name;
    uint8_t min_items;
    uint8_t max_items;
  };

  struct zone_drop {
    uint8_t profession_id;
    name    resource_name;
  };
  
  struct combatslots {
    uint64_t ability1 = 0;
    uint64_t ability2 = 0;
    uint64_t ability3 = 0;
    uint64_t racial = 0;
  };
  
  struct equipment {
    uint64_t head;
    uint64_t neck;
    uint64_t shoulders;
    uint64_t chest;
    uint64_t back;
    uint64_t bracers;
    uint64_t hands;
    uint64_t waist;
    uint64_t legs;
    uint64_t feet;
    uint64_t weapon;
    uint64_t ranged;
    uint64_t ring1;
    uint64_t ring2;
    uint64_t trinket1;
    uint64_t trinket2;
  };

  struct coords {
    uint64_t x;
    uint64_t y;
  };
  
  struct tiledata {
    coords coordinates;
    string attributes;
  };
  
  struct trigger {
    coords coordinates;
    float_t radius;
    string attributes;
  };
  
  struct mobdata {
    coords coordinates;
    uint64_t mob_id;
    uint8_t status;
    float_t radius;
    string attributes;
  };
  
  struct npcdata {
    coords coordinates;
    uint64_t npc_id;
    float_t radius;
    string attributes;
  };

  struct requirement {
    name token_name;
    uint64_t quantity;
  };

  struct objective {
    uint64_t objective_id;
    uint8_t  objective_type = 0;
    uint8_t  completed = 0;
    string   objective_data;
  };
  
}