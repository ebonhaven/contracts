#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/print.hpp>
#include <string.h>
#include "json.hpp"
#include "types.hpp"

using namespace eosio;
using namespace std;
using namespace types;

CONTRACT admin : public contract {
  public:
    admin( name receiver, name code, datastream<const char*> ds):contract(receiver, code, ds) {}
    const name GAME_ACCOUNT = name("ebonhavencom");

    ACTION upsaura( uint64_t aura_id, string aura_name, string aura_description, uint8_t aura_type, uint8_t is_hidden, uint8_t cooldown, string& aura_data );
    ACTION upsability( uint64_t ability_id, string ability_name, string ability_description, asset ability_cost, uint32_t level, uint8_t profession_lock, uint8_t race_lock, string ability_data );
    ACTION upsdrop( uint64_t drop_id, asset min_worth, asset max_worth, vector<item_drop> item_drops);
    ACTION upseffect(uint64_t effect_id, uint8_t effect_type, float_t chance, uint8_t cooldown, string& effect_data );
    ACTION upsmapdata( uint64_t mapdata_id, uint64_t world_zone_id, position respawn, vector<tiledata> tiles, vector<trigger> triggers, vector<mobdata> mobs, vector<npcdata> npcs, vector<zone_drop> resources, rate_mod rate_modifier );
    ACTION upsmob( uint32_t mob_id, string mob_name, uint8_t level, uint8_t mob_type, attack attack, defense defense, string mob_data, uint32_t hp, uint32_t experience, asset worth, uint64_t drop_id, bribe_drop bribe );
    ACTION upsnpc( uint64_t npc_id, vector<name> quests, vector<uint64_t> triggers );
    ACTION upsprogress( uint8_t level, uint64_t experience );
    ACTION upsrecipe( uint64_t recipe_id, name category, name token_name, uint8_t profession_lock, uint32_t min_skill, uint32_t max_skill, vector<requirement> requirements );
    ACTION upsresource( name resource_name, uint8_t profession_id, uint32_t experience, uint32_t min_skill, uint32_t max_skill, vector<resource_drop> drops );
    ACTION upsstats( uint8_t profession, uint8_t base_hp, uint8_t hp_increase, attack  base_attack, defense base_defense, attack attack_increase, defense defense_increase, stats base_stats, stats stats_increase );
    ACTION upstreasure( uint64_t world_zone_id, vector<item_drop> drops );
    ACTION upsquest( name user, uint64_t character_id, name quest_name, uint64_t begin_npc_id, uint8_t min_level, uint64_t complete_npc_id, asset worth, uint32_t experience, vector<name> rewards, uint8_t repeatable, vector<uint64_t> prerequisites, uint8_t profession_lock, uint8_t race_lock, vector<objective> objectives );
  
  private:

    TABLE ability {
      uint64_t ability_id;
      string   ability_name;
      string   ability_description;
      asset    ability_cost = asset(0, symbol(symbol_code("EBON"), 2));
      uint32_t level = 1;
      uint8_t  profession_lock = 0;
      uint8_t  race_lock = 0;
      string   ability_data;

      uint64_t primary_key() const { return ability_id; }
    };

    TABLE aura {
      uint64_t aura_id;
      string   aura_name;
      string   aura_description;
      uint16_t aura_type;
      uint8_t  is_hidden = 0;
      uint8_t  cooldown = 1;
      string   aura_data;

      uint64_t primary_key() const { return aura_id; }
    };

    TABLE basestat {
      uint8_t profession_id;
      uint8_t base_hp;
      uint8_t hp_increase;
      attack  base_attack;
      defense base_defense;
      attack  attack_increase;
      defense defense_increase;
      stats   base_stats;
      stats   stats_increase;
      
      auto primary_key() const { return profession_id; }
    };

    TABLE drop {
      uint64_t drop_id;
      asset    min_worth = asset(0, symbol(symbol_code("EBON"), 2));
      asset    max_worth = asset(0, symbol(symbol_code("EBON"), 2));
      vector<item_drop> drops;

      uint64_t primary_key() const { return drop_id; }
    };

    TABLE effect {
      uint64_t effect_id;
      uint8_t  effect_type;
      float_t  chance = 1.00;
      uint8_t  cooldown = 1;
      string   effect_data;
      
      uint64_t primary_key() const { return effect_id; }

      const uint64_t get_recipe_id() {
        auto j = nlohmann::json::parse(effect_data);
        if (j.count("learn") > 0) {
          return j["learn"].get<uint64_t>();
        } else {
          return 0;
        }
      }
      
      const bool can_resurrect() {
        auto j = nlohmann::json::parse(effect_data);
        return (j.count("resurrect") > 0 ? true : false);
      }
      
      const bool can_unlock() {
        auto j = nlohmann::json::parse(effect_data);
        return (j.count("unlocks") > 0 ? true : false);
      }
    };

    TABLE mapdata {
      uint64_t          mapdata_id;
      uint64_t          world_zone_id; // primary
      uint64_t          character_id = 0;
      position          respawn;
      vector<tiledata>  tiles;
      vector<trigger>   triggers;
      vector<mobdata>   mobs;
      vector<npcdata>   npcs;
      vector<zone_drop> resources;
      rate_mod          rate_modifier;
      
      uint64_t primary_key() const { return mapdata_id; }
      uint64_t by_character_id() const { return character_id; }
    };

    TABLE mob {
      uint32_t   mob_id;
      string     mob_name;
      int32_t    level = 1;
      uint8_t    mob_type = 0;
      uint8_t    last_decision = 0;
      uint8_t    last_decision_hit = 1;
      attack     attack;
      defense    defense;
      string     mob_data;
      uint32_t   hp = 0;
      uint32_t   max_hp = 0;
      uint32_t   experience = 0;
      asset      worth = asset(0, symbol(symbol_code("EBON"),2));
      uint64_t   drop_id;
      uint8_t    bribed = 0;
      bribe_drop bribe;

      uint64_t primary_key() const { return mob_id; }

      const bool can_bribe() const {
        return (bribe.drop_id > 0 ? true : false);
      }
    };

    TABLE npc {
      uint64_t npc_id;
      vector<name> quests;
      vector<uint64_t> triggers;

      uint64_t primary_key() const { return npc_id; }
    };

    TABLE progress {
      uint8_t  level;
      uint64_t experience;

      uint8_t primary_key() const { return level; }
    };

    TABLE quest {
      uint64_t quest_id;
      name     quest_name;
      uint64_t character_id;
      uint64_t begin_npc_id;
      uint8_t  min_level = 1;
      uint64_t complete_npc_id;
      asset    worth;
      uint32_t experience;
      vector<name> rewards;
      uint8_t  repeatable;
      vector<uint64_t> prerequisites;
      uint8_t  profession_lock = 0;
      uint8_t  race_lock = 0;
      vector<objective> objectives;

      uint64_t primary_key() const { return quest_name.value; }
      uint64_t by_quest_id() const { return quest_id; }
    };

    TABLE recipe {
      uint64_t            recipe_id;
      name                category;
      name                token_name;
      uint8_t             profession_lock;
      uint32_t            min_skill;
      uint32_t            max_skill;
      vector<requirement> requirements;

      uint64_t primary_key() const { return recipe_id; }
    };

    TABLE resource {
      name     resource_name;
      uint8_t  profession_id;
      uint32_t experience = 0;
      uint32_t min_skill;
      uint32_t max_skill;
      vector<resource_drop> drops;
      
      auto primary_key() const { return resource_name.value; }
    };

    TABLE treasure {
      uint64_t world_zone_id;
      vector<item_drop> drops;
      
      uint64_t primary_key() const { return world_zone_id; }
    };
    
    using abilities_index = multi_index< "abilities"_n, ability >;
    using auras_index = multi_index< "auras"_n, aura >;
    using basestats_index = multi_index< "basestats"_n, basestat >;
    using drops_index = multi_index< "drops"_n, drop >;
    using effects_index = multi_index< "effects"_n, effect >;
    using mapdata_index = multi_index< "mapdata"_n, mapdata,
            indexed_by< "bycharacterid"_n, const_mem_fun< mapdata, uint64_t, &mapdata::by_character_id> > >;
    using mobs_index = multi_index< "mobs"_n, mob >;
    using npcs_index = multi_index< "npcs"_n, npc>;
    using progress_index = multi_index< "progress"_n, progress>;
    using quests_index = multi_index< "quests"_n, quest,
            indexed_by< "byquestid"_n, const_mem_fun< quest, uint64_t, &quest::by_quest_id> > >;
    using recipes_index = multi_index< "recipes"_n, recipe>;
    using resources_index = multi_index< "resources"_n, resource >;
    using treasures_index = multi_index< "treasures"_n, treasure >;
    
};

EOSIO_DISPATCH(admin, 
  (upsaura)
  (upsability)
  (upsdrop)
  (upseffect)
  (upsmapdata)
  (upsmob)
  (upsnpc)
  (upsrecipe)
  (upsresource)
  (upsprogress)
  (upsquest)
  (upsstats)
  (upstreasure)
)


