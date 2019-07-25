#pragma once

#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/asset.hpp>
#include <eosio/print.hpp>
#include <string.h>
#include <math.h>
#include "json.hpp"
#include "types.hpp"
#include "dasset.hpp"
#include "attributes.hpp"

using namespace eosio;
using namespace std;
using namespace types;
using namespace dgoods_asset;
using namespace item_attributes;

CONTRACT ebonhaven : public contract {
  
  private:
  
    TABLE globals {
      uint8_t     key = 1;
      string      version;
      symbol_code symbol = symbol_code("EBON");
      uint64_t    category_name_id;
      uint32_t    seed = 1;
      asset       supply;
      asset       max_supply;
      rates       rates;
    };
    
    TABLE account {
      name     user;
      asset    balance = asset(0, symbol(symbol_code("EBON"), 2));
      uint8_t  max_characters = 1;
      uint8_t  max_inventory = 16;
      
      auto primary_key() const { return user.value; }
    };
    
    TABLE character {
      uint64_t    character_id;
      name        owner;
      string      character_name;
      uint8_t     gender;
      uint8_t     profession;
      uint8_t     race;
      uint8_t     level = 1;
      uint64_t    experience = 0;
      position    position;
      float_t     movement_radius;
      uint8_t     status = 0;
      uint64_t    hp = 0;
      uint64_t    max_hp = 0;
      stats       stats;
      attack      attack;
      defense     defense;
      equipment   equipped;
      combatslots abilities;
      
      uint64_t primary_key() const { return character_id; }
    };
    
    TABLE charhistory {
      uint64_t         character_id;
      skill            profession_skill;
      vector<uint64_t> learned_abilities;
      vector<uint64_t> learned_recipes;
      vector<uint64_t> current_quests;
      vector<uint64_t> completed_quests;
      vector<uint64_t> feats;
      
      uint64_t primary_key() const { return character_id; }
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
        return (j.count("ressurect") > 0 ? true : false);
      }
      
      const bool can_unlock() {
        auto j = nlohmann::json::parse(effect_data);
        return (j.count("unlocks") > 0 ? true : false);
      }
    };
    
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
    
    TABLE drop {
      uint64_t drop_id;
      vector<item_drop> drops;

      uint64_t primary_key() const { return drop_id; }
    };
    
    TABLE mob {
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

      uint64_t primary_key() const { return mob_id; }
    };
    
    TABLE encounter {
      uint64_t encounter_id;
      uint64_t character_id;
      uint8_t  encounter_status = 0;
      uint64_t turn_counter = 0;
      uint64_t last_decision = 0;
      uint64_t last_decision_hit = 1;
      vector<mob> mobs;

      uint64_t primary_key() const { return encounter_id; }

      const mob& get_mob_by_id(int id) const {
        for (int i = 0; i < mobs.size(); i++) {
            if (mobs[i].mob_id == id) {
                return mobs[i];
            }
        }

        check(false, "No mob with that id");
        return mobs[0];
      }
    };
    
    TABLE reward {
      uint64_t     reward_id;
      uint64_t     character_id;
      uint64_t     encounter_id;
      uint32_t     experience;
      asset        worth = asset(0, symbol(symbol_code("EBON"), 2));
      vector<name> items;
      
      auto primary_key() const { return reward_id; }
    };
    
    TABLE treasure {
      uint64_t world_zone_id;
      vector<item_drop> drops;
      
      uint64_t primary_key() const { return world_zone_id; }
    };
    
    // Scope is profession id
    TABLE resource {
      uint64_t world_zone_id;
      uint32_t min_skill;
      vector<resource_drop> drops;
      
      uint64_t primary_key() const { return world_zone_id; }
    };
    
    // dGoods
    TABLE categoryinfo {
      name category;

      uint64_t primary_key() const { return category.value; }
    };
    
    TABLE balances {
      uint64_t category_name_id;
      name category;
      name token_name;
      dasset amount;

      uint64_t primary_key() const { return category_name_id; }
    };
    
    // Scope is category name (ebonhavencom)
    TABLE dgoodstats {
      bool     fungible;
      bool     burnable;
      bool     transferable;
      name     issuer;
      name     token_name;
      uint64_t category_name_id;
      dasset   max_supply;
      uint64_t current_supply;
      uint64_t issued_supply;
      string   base_uri;
      stat_attributes attributes;

      auto primary_key() const { return token_name.value; }
    };
    
    TABLE dgood {
      uint64_t id;
      uint64_t serial_number;
      name     owner;
      name     category;
      name     token_name;
      std::optional<string> relative_uri;
      bool     equipped = false;
      dgood_attributes attributes;

      uint64_t primary_key() const { return id; }
      uint64_t get_owner() const { return owner.value; }
    };
    
    EOSLIB_SERIALIZE( dgood, (id)(serial_number)(owner)(category)(token_name)(relative_uri)(equipped)(attributes) )
    
    // Scope is user
    TABLE mapdata {
      uint64_t         world_zone_id; // primary
      uint64_t         character_id = 0;
      position         respawn;
      vector<tiledata> tiles;
      vector<trigger>  triggers;
      vector<mobdata>  mobs;
      vector<npcdata>  npcs;
      
      uint64_t primary_key() const { return world_zone_id; }
      uint64_t by_character_id() const { return character_id; }
    };

    TABLE recipe {
      uint64_t            recipe_id;
      name                category;
      name                token_name;
      uint8_t             profession_lock;
      uint32_t            min_skill;
      vector<requirement> requirements;

      uint64_t primary_key() const { return recipe_id; }
    };
    
    // TABLES
    using globals_index = singleton< "globals"_n, globals >;
    
    using accounts_index = multi_index< "accounts"_n, account >;
    
    using characters_index = multi_index< "characters"_n, character >;
    
    using charhistory_index = multi_index< "charhistory"_n, charhistory >;
    
    using basestats_index = multi_index< "basestats"_n, basestat >;
    
    using auras_index = multi_index< "auras"_n, aura >;
    
    using effects_index = multi_index< "effects"_n, effect >;
    
    using abilities_index = multi_index< "abilities"_n, ability >;
    
    using drops_index = multi_index< "drops"_n, drop >;
    
    using mobs_index = multi_index< "mobs"_n, mob >;
    
    using encounters_index = multi_index< "encounters"_n, encounter >;
    
    using rewards_index = multi_index< "rewards"_n, reward >;
    
    using treasures_index = multi_index< "treasures"_n, treasure >;
    
    using resources_index = multi_index< "resources"_n, resource >;
    
    using balances_index = multi_index< "balances"_n, balances >;
    
    using stats_index = multi_index< "dgoodstats"_n, dgoodstats>;
    
    using category_index = multi_index< "categoryinfo"_n, categoryinfo>;
    
    using dgoods_index = multi_index< "dgood"_n, dgood,
            indexed_by< "byowner"_n, const_mem_fun< dgood, uint64_t, &dgood::get_owner> > >;
            
    using mapdata_index = multi_index< "mapdata"_n, mapdata,
            indexed_by< "bycharacterid"_n, const_mem_fun< mapdata, uint64_t, &mapdata::by_character_id> > >;

    using recipes_index = multi_index< "recipes"_n, recipe>;
            
    void mint( name to, name issuer, name category, name token_name,
               uint64_t issued_supply, string relative_uri, dgood_attributes attributes );
    void add_balance( name owner, name issuer, name category, name token_name,
                     uint64_t category_name_id, dasset quantity );
    void sub_balance( name owner, uint64_t category_name_id, dasset quantity );
    void apply_effect_to_character( effect e, character& c );
    void reset_stats( character& c );
    void add_item_stats( name user, character& c );
    void apply_item_auras( name user, character& c, uint64_t item_id );
    void add_token_balance( name owner, asset value, name ram_payer );
    void sub_token_balance( name owner, asset value );
    uint64_t calculate_player_attack_damage( name user,
                                             ebonhaven::character& c,
                                             ebonhaven::mob& m,
                                             bool is_ranged);
    void generate_encounter( name user, name payer, ebonhaven::character& character, vector<uint64_t> mob_ids);
    void generate_reward( name user,
                          name payer,
                          uint32_t character_level,
                          encounter s_encounter );
    void generate_resource_reward( name user, name payer, uint64_t character_it, vector<uint64_t> resource_items ) ;
    uint32_t calculate_total_experience(uint32_t character_level, ebonhaven::encounter e);
    bool is_encounter_over(ebonhaven::character c, ebonhaven::encounter e);

    int random(const int range) {
      globals_index globals( get_self(), get_self().value );
      check(globals.exists(), "globals does not exist, setup first");
      auto globals_singleton = globals.get();
      
      int prime = 65537;
      auto new_seed_value = (globals_singleton.seed + time_point_sec(current_time_point()).utc_seconds) % prime;
      
      globals_singleton.seed = new_seed_value;
      globals.set(globals_singleton, get_self());
      
      int random_result = new_seed_value % range;
      return random_result;
    }
    
    int rate_to_floor(float_t rate, int range) {
      return (int)floor(rate * range + 0.5);
    }
    
    int inventory_count(name user) {
      dgoods_index dgood_table(get_self(), get_self().value);
      auto index_by_owner = dgood_table.get_index<name("byowner")>();
      auto itr = index_by_owner.lower_bound(user.value);
      int i = 0;
      while (itr != index_by_owner.end()) {
        if (itr->owner == user && itr->equipped != true) {
          i++;
        }
        itr++;
      }
      return i;
    }

    bool is_position_within_target_radius(pair <int, int> position, pair<int, int> target, float radius) {
      for(int x = target.first - radius; x < target.first + radius; x++) {
        float yspan = radius * sin(acos(( target.first - x ) / radius));
        for(int y = target.second - yspan; y < target.second + yspan; y++) {
            if (x == position.first && y == position.second) {
                return true;
            }
        }
      }
      return false;
    }

    bool is_coordinate_walkable(vector<tiledata> tiles, uint64_t x, uint64_t y) {
      auto tile = find_if(tiles.begin(), tiles.end(), [&x, &y](const struct tiledata& el){ return el.coordinates.x == x && el.coordinates.y == y; });
      check(tile != tiles.end(), "tile not found");
      if (tile != tiles.end()) {
        nlohmann::json attr = nlohmann::json::parse(tile->attributes);
        return attr["walkable"].get<bool>();
      }
      return false;
    }
    
    template <typename Enumeration>
    auto as_integer(Enumeration const value)
      -> typename std::underlying_type<Enumeration>::type
    {
      return static_cast<typename std::underlying_type<Enumeration>::type>(value);
    }
            
  public:
    using contract::contract;
    ebonhaven(eosio::name receiver, eosio::name code, datastream<const char*> ds):contract(receiver, code, ds) {}
    
    const int WEEK_SEC = 3600*24*7;
    
    ACTION newaccount( name user );

    ACTION newcharacter( name     user,
                         string   character_name,
                         uint8_t  gender,
                         uint8_t  profession,
                         uint8_t  race );
                         
    ACTION delcharacter( name user, uint64_t character_id );
                         
    ACTION useitem( name user, uint64_t character_id, uint64_t dgood_id, uint8_t effect_idx );
    
    ACTION equipitem( name user, uint64_t character_id, uint64_t dgood_id, uint8_t equip_slot );
    
    ACTION buyability( name user, uint64_t character_id, uint64_t ability_id );
    
    ACTION equipability( name user, uint64_t character_id, uint64_t ability_id, uint8_t ability_idx );
                         
    ACTION move( name user, uint64_t character_id, position new_position );
    
    ACTION combat( name user, uint64_t encounter_id, uint8_t combat_decision, uint8_t mob_idx );
    
    ACTION claimrewards( name user, uint64_t reward_id, vector<name> selected_items );

    ACTION craft( name user, uint64_t character_id, uint64_t recipe_id );

    ACTION gather( name user, uint64_t character_id );
    
    ACTION printval( name user, uint64_t x, uint64_t y );
    
    // Admin
    ACTION spawnitems( name to, name token_name, uint64_t quantity );
    
    //Admin
    ACTION spawnability( name user, uint64_t character_id, uint64_t ability_id );
    
    // Admin
    ACTION newencounter( name user, uint64_t character_id, vector<uint64_t> mob_ids );
    
    // Admin
    ACTION modencounter( name user,
                         uint64_t encounter_id,
                         uint8_t encounter_status, 
                         vector<mod_mob> mobs );
    
    // Admin                     
    ACTION delencounter( name user, uint64_t encounter_id );
    
    // Admin
    ACTION create( name category,
                   name token_name,
                   bool fungible,
                   bool burnable,
                   bool transferable,
                   stat_attributes attributes,
                   string base_uri,
                   string max_supply );
    // Admin
    ACTION issue(name to,
                 name category,
                 name token_name,
                 string quantity,
                 string relative_uri,
                 string creator_name,
                 string memo);
  
    ACTION transfernft(name from,
                       name to,
                       vector<uint64_t> dgood_ids,
                       string memo);
    
    ACTION burnnft(name owner,
                   vector<uint64_t> dgood_ids);
    
    // Admin
    ACTION modstatus( name user, uint64_t character_id, uint8_t status );

    // Admin
    ACTION upsaura( uint64_t aura_id,
                    string aura_name, 
                    string aura_description,
                    uint8_t aura_type,
                    uint8_t is_hidden,
                    uint8_t cooldown, 
                    string& aura_data );
    
    // Admin                
    ACTION upsability( uint64_t ability_id,
                       string ability_name,
                       string ability_description,
                       asset ability_cost,
                       uint32_t level,
                       uint8_t profession_lock,
                       uint8_t race_lock,
                       string ability_data );
    
    // Admin
    ACTION upseffect( uint64_t effect_id,
                      uint8_t effect_type,
                      float_t chance,
                      uint8_t cooldown,
                      string& effect_data );
                                 
    // Admin
    ACTION upsstats( uint8_t profession,
                     uint8_t base_hp,
                     uint8_t hp_increase,
                     attack  base_attack,
                     defense base_defense,
                     attack  attack_increase,
                     defense defense_increase,
                     stats   base_stats,
                     stats   stats_increase );
                     
    // Admin                     
    ACTION upsrates( float_t combat_rate,
                     float_t resource_rate,
                     float_t discovery_rate,
                     float_t trap_rate, 
                     float_t treasure_rate,
                     float_t loot_rate );
                     
    // Admin
    ACTION upsmob( uint32_t mob_id,
                   string   mob_name,
                   uint8_t  level,
                   uint8_t  mob_type,
                   attack   attack,
                   defense  defense,
                   string   mob_data,
                   uint32_t hp,
                   uint32_t experience,
                   asset    worth,
                   uint64_t drop_id );
    
    // Admin                 
    ACTION upsdrop( uint64_t drop_id, vector<item_drop> item_drops);
    
    // Admin
    ACTION upsresource( uint64_t world_zone_id,
                        uint8_t  profession_id,
                        uint32_t min_skill,
                        vector<resource_drop> drops );
                     
    // Admin
    ACTION upstreasure( uint64_t world_zone_id, vector<item_drop> drops );
    
    // Admin
    ACTION upsmapdata( uint64_t world_zone_id,
                       name user,
                       uint64_t character_id,
                       position respawn,
                       vector<tiledata> tiles,
                       vector<trigger> triggers,
                       vector<mobdata> mobs,
                       vector<npcdata> npcs );

    ACTION upsrecipe( uint64_t recipe_id,
                      name category,
                      name token_name,
                      uint8_t profession_lock,
                      uint32_t min_skill,
                      vector<requirement> requirements );
    
    // Admin                     
    ACTION setconfig(string version);
    
    // Admin
    ACTION tokenreward( name to, asset quantity );
    
    // Admin
    ACTION tokenissue( name to, asset quantity );
    
    // Admin
    ACTION tokenxfer( name from, name to, asset quantity );
    
    // Admin
    ACTION tokenretire( asset quantity, string memo );
};