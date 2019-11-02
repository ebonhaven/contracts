#pragma once

#include <eosio/eosio.hpp>
#include <string>

using namespace std;
using namespace eosio;

namespace item_attributes {
  enum class item_type: uint8_t {
    NONE = 0,
    WEAPON = 1,
    RANGED = 2,
    LIGHT = 3,
    MEDIUM = 4,
    HEAVY = 5,
    RING = 6,
    TRINKET = 7,
    CONSUMABLE = 8,
    RESOURCE = 9,
    CHEST = 10,
    KEY = 11
  };
  
  enum class equipment_slot: uint8_t {
    NONE = 0,
    HEAD = 1,
    NECK = 2,
    SHOULDERS = 3,
    CHEST = 4,
    BACK = 5,
    BRACERS = 6,
    HANDS = 7,
    WAIST = 8,
    LEGS = 9,
    FEET = 10,
    WEAPON = 11,
    RING1 = 12,
    RING2 = 13,
    TRINKET1 = 14,
    TRINKET2 = 15
  };

  enum class item_quality: uint8_t {
      NONE = 0,
      JUNK = 1,
      COMMON = 2,
      RARE = 3,
      EPIC = 4,
      LEGENDARY = 5,
  };

  enum class item_status: uint8_t {
      NORMAL = 0,
      BROKEN = 1,
      LOCKED = 2,
      UNLOCKED = 3
  };

  enum class item_binding: uint8_t {
      NONE = 0,
      EQUIP = 1,
      BOUND = 2
  };
  
  struct dgood_attributes {
    uint8_t bind_status = 0;
    uint8_t item_status = 0;
    uint8_t durability = 0;
    string  creator_name;
    time_point_sec created_at;
  };
  
  struct stat_attributes {
    string item_name;
    string item_description;
    uint8_t item_binding = 0;
    uint8_t level_requirement = 0;
    uint8_t quality = 0;
    uint8_t slot = 0;
    uint8_t item_type = 0;
    uint8_t profession_lock = 0;
    uint8_t max_durability = 0;
    asset repair_cost = asset(0, symbol(symbol_code("EBON"), 2));
    vector<uint64_t> auras;
    vector<uint64_t> effects;
    
    const bool is_key() {
      return (item_type == 11 ? true : false);
    }

    const bool is_chest() {
      return (item_type == 10 ? true : false);
    }
    
    const bool is_consumable() {
      return (item_type == 8 || item_type == 11 ? true : false);
    }
  
    EOSLIB_SERIALIZE( stat_attributes, 
      (item_name)(item_description)(item_binding)(level_requirement)
      (quality)(slot)(item_type)(profession_lock)(max_durability)(repair_cost)(auras)(effects) )
  };
}