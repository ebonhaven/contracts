#pragma once

using namespace eosio;

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
  
  enum class combat_target: uint8_t {
    ENEMY = 0,
    ALL = 1,
    SELF = 2,
    ALLY = 3
  };
  
  enum class effect_type: uint8_t {
    APPLYEFFECT = 0,
    ADDAURA = 1,
    ITEMEFFECT = 2
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
    uint8_t world = 0;
    uint8_t zone = 0;
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t orientation = 0;
  };
  
  struct stats {
    // Increase max health
    uint32_t stamina = 0;
    // Increase health regeneration out of combat
    uint32_t regen = 0;
    // Improved chance of resource encounter
    uint32_t perception = 0;
    // Improved chance of crafting additional items
    uint32_t skill = 0;
    // Improved chance of finding treasure
    uint32_t luck = 0;
  };
  
  struct rates {
    float_t        combat = 0.15;
    float_t        resource = 0.09;
    float_t        discovery = 0.025;
    float_t        trap = 0.017;
    float_t        treasure = 0.005;
    float_t        loot = 0.0015;
    time_point_sec updated_at;
  };
  
  struct item_drop {
    float_t percentage = 0.00;
    name    token_name;
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
  
}