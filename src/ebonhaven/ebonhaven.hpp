#pragma once

using namespace eosio;
using namespace std;

class [[eosio::contract]] ebonhaven : public contract {
    public:
        ebonhaven( name receiver, name code, datastream<const char*> ds):contract(receiver, code, ds),
            _config(_code, _code.value){}
            
        [[eosio::action]]
        void combat( name user, uint64_t encounter_id, uint8_t user_decision);

        [[eosio::action]]
        void move( name user, uint64_t character_id );

        [[eosio::action]]
        void newcharacter( name user, string character_name, uint8_t gender, uint8_t profession, uint8_t race );

        [[eosio::action]]
        void delcharacter( name user, uint64_t character_id );
        
        // Admin actions (privileged) - not intended for public use

        [[eosio::action]]
        void newitem( name     user,
                      uint32_t item_id,
                      string   item_name,
                      string   item_description,
                      uint8_t  bind_on,
                      uint32_t stack_quantity,
                      uint32_t max_stack_size,
                      uint8_t  is_consumable,
                      uint8_t  level_requirement,
                      uint8_t  quality,
                      uint8_t  slot,
                      uint8_t  profession_lock,
                      uint64_t aura_id);
        
        [[eosio::action]]
        void upsertmob( name     user,
                     uint32_t mob_id,
                     string   mob_name,
                     uint8_t  level,
                     uint8_t  mob_type,
                     uint16_t attack,
                     uint16_t defense,
                     uint32_t hp,
                     uint32_t max_hp,
                     uint16_t skill1,
                     uint16_t skill2,
                     uint16_t skill3,
                     uint32_t experience,
                     asset worth,
                     uint64_t drop_id);

        [[eosio::action]]
        void useitem( name user, uint64_t character_id, uint64_t item_id );

        [[eosio::action]]
        void spawnitems( name user, name to, vector<uint64_t> item_ids ); 

        [[eosio::action]]
        void upsertstats( name    user, 
                          uint8_t profession_id,
                          uint8_t base_hp,
                          uint8_t hp_increase_by_level,
                          vector<uint32_t> base_attack,
                          vector<uint32_t> base_defense,
                          vector<uint32_t> new_stats,
                          vector<uint32_t> attack_increase,
                          vector<uint32_t> defense_increase,
                          vector<uint32_t> stat_increase_by_level); 

        [[eosio::action]]
        void tokencreate( name   issuer,
                    asset  maximum_supply);

        [[eosio::action]]
        void tokenissue( name to, asset quantity, string memo );

        [[eosio::action]]
        void tokenreward( name to, asset quantity );

        [[eosio::action]]
        void tokenretire( asset quantity, string memo );

        [[eosio::action]]
        void tokenxfer( name    from,
                    name    to,
                    asset   quantity,
                    string  memo );
        
        [[eosio::action]]
        void newaccount( name user );

        [[eosio::action]]
        void newencounter( name user, uint64_t character_id, vector<uint64_t> mob_ids);

        [[eosio::action]]
        void delencounter( name user, uint64_t encounter_id );

        [[eosio::action]]
        void upsertdrop( name user, uint64_t drop_id, vector<mob_drop> drops);

        [[eosio::action]]
        void claimrewards( name user, uint64_t reward_id, vector<uint64_t> item_ids);

        [[eosio::action]]
        void delrewards( name user, uint64_t reward_id );

        [[eosio::action]]
        void upsertaura( name user,
                         uint64_t aura_id,
                         string aura_description,
                         uint8_t has_charges,
                         uint8_t charges_remaining,
                         uint8_t turns_remaining,
                         int32_t hp_delta,
                         vector<uint32_t> stats_delta,
                         vector<uint32_t> attack_delta,
                         vector<uint32_t> defense_delta );

        // [[eosio::action]]
        // void tokenopen( name owner, const symbol& symbol, name ram_payer );

        // [[eosio::action]]
        // void tokenclose( name owner, const symbol& symbol );

    private:
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
            DISCOVERY = 1,
            RESOURCE = 2,
            TRAP = 3,
            COMBAT = 4,
            TREASURE = 5,
            LOOT = 6
        };

        enum class combat_decision: uint8_t {
            PRIMARY = 0,
            ABILITY1 = 1,
            ABILITY2 = 2,
            ABILITY3 = 3,
            FLEE = 4,
            BRIBE = 5
        };

        enum class orientation: uint8_t {
            NORTH = 0,
            EAST = 1,
            SOUTH = 2,
            WEST = 3 
        };

        struct [[eosio::table]] globals {
            uint64_t key = 1;
            uint32_t seed = 1;
            asset    supply = asset(0, symbol(symbol_code("EBH"),4));
            asset    max_supply;
            name     issuer;
            
            auto primary_key() const { return key; }
        };

        typedef multi_index<"globals"_n, globals> config_table;
        config_table _config;

        struct position {
            uint8_t world = 0;
            uint8_t zone = 0;
            uint8_t x = 0;
            uint8_t y = 0;
            uint8_t orientation = 0;
        };

        struct equipment {
            uint64_t head;
            uint64_t outfit;
            uint64_t weapon;
            uint64_t jewelry;
        };

        struct [[eosio::table]] character {
            uint64_t character_id;
            name user;
            string character_name;
            uint8_t gender;
            uint8_t profession;
            uint8_t race;
            int32_t level = 1;
            uint32_t experience = 0;
            position position;
            uint8_t in_combat = 0;
            int64_t hp = 0;
            uint64_t max_hp = 0;
            stats stats;
            attack attack;
            defense defense;
            equipment equipped;

            uint64_t primary_key() const { return character_id; }
        };

        typedef eosio::multi_index<"characters"_n, character> characters_table;

        struct [[eosio::table]] account {
            name user;
            asset balance = asset(0, symbol(symbol_code("EBH"),4));
            vector<uint64_t> characters;
            vector<uint64_t> inventory;

            auto primary_key() const { return user.value; }
        };

        typedef eosio::multi_index<"accounts"_n, account> accounts_table;

        struct [[eosio::table]] basestat {
            uint8_t profession_id;
            uint8_t base_hp;
            uint8_t hp_increase_by_level;
            attack base_attack;
            defense base_defense;
            stats base_stats;
            attack attack_increase_by_level;
            defense defense_increase_by_level;
            stats stat_increase_by_level;

            auto primary_key() const { return profession_id; }
        };

        typedef eosio::multi_index<"basestats"_n, basestat> base_stats_table;

        struct [[eosio::table]] aura {
            uint64_t aura_id = 0;
            string aura_description;
            uint8_t has_charges = 0;
            uint8_t charges_remaining = 1;
            uint8_t turns_remaining = 0;
            int32_t hp_delta = 0;
            stats stats_delta;
            attack attack_delta;
            defense defense_delta;

            uint64_t primary_key() const { return aura_id; }
        };

        typedef eosio::multi_index<"auras"_n, aura> auras_table;

        struct [[eosio::table]] item {
            uint64_t item_id;
            uint64_t parent_id = 0;
            string item_name;
            string item_description;
            uint8_t bind_on = 0;
            uint32_t stack_quantity = 1;
            uint32_t max_stack_size = 1;
            uint8_t is_consumable = 0;
            uint8_t level_requirement = 0;
            name owner;
            string creator_name;
            name creator_account;
            uint32_t created_at;
            uint8_t quality = 0;
            uint8_t slot = 0;
            uint8_t profession_lock;
            aura aura;

            uint64_t primary_key() const { return item_id; }
        };

        typedef eosio::multi_index<"items"_n, item> items_table;

        struct [[eosio::table]] s_drop {
            uint64_t drop_id;
            vector<mob_drop> drops;

            uint64_t primary_key() const { return drop_id; }
        };

        typedef eosio::multi_index<"drops"_n, s_drop> drops_table;

        struct [[eosio::table]] mob {
            uint32_t mob_id;
            string mob_name;
            int32_t level = 1;
            uint8_t mob_type = 0;
            uint16_t attack = 0;
            uint16_t defense = 0;
            uint32_t hp = 0;
            uint32_t max_hp = 0;
            uint16_t skill1 = 0;
            uint16_t skill2 = 0;
            uint16_t skill3 = 0;
            uint32_t experience = 0;
            asset worth = asset(0, symbol(symbol_code("EBH"),4));
            s_drop drop;

            uint64_t primary_key() const { return mob_id; }
        };

        typedef eosio::multi_index<"mobs"_n, mob> mobs_table;

        struct [[eosio::table]] encounter {
            uint64_t encounter_id;
            uint64_t character_id;
            vector<mob> mobs;            

            uint64_t primary_key() const { return encounter_id; }

            const mob& get_mob_by_id(int id) const {
                for (int i = 0; i < mobs.size(); i++) {
                    if (mobs[i].mob_id == id) {
                        return mobs[i];
                    }
                }

                eosio_assert(0, "No mob with that id");
                return mobs[0];
            }
        };

        typedef eosio::multi_index<"encounters"_n, encounter> encounters_table;

        struct [[eosio::table]] reward {
            uint64_t reward_id;
            uint64_t character_id;
            asset worth = asset(0, symbol(symbol_code("EBH"),4));
            vector<item> items;

            uint64_t primary_key() const { return reward_id; }
        };

        typedef eosio::multi_index<"rewards"_n, reward> rewards_table;

        reward generate_reward( name user, encounter s_encounter );
        
        void finalize_encounter( name user, encounter s_encounter );
        void apply_experience( name user, encounter s_encounter );
        vector<item> consolidate_inventory( name payer, account acct, vector<item> newItems );

        void upsert_item( name user, item in_item );
        void upsert_rewards( name user, reward in_reward );

        void sub_balance( name owner, asset value );
        void add_balance( name owner, asset value, name ram_payer );

        int random( const int range );

        uint64_t create_hash( const string& str );

        string to_hex( const uint8_t* data, uint32_t length );
};

