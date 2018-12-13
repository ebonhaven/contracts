#pragma once

#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/print.hpp>
#include <functional>
#include <string.h>

using namespace eosio;
using namespace std;

class [[eosio::contract]] ebonhaven : public contract {
    public:
        ebonhaven( name receiver, name code, datastream<const char*> ds):contract(receiver, code, ds),
            _config(_code, _code.value){}

        [[eosio::action]]
        void newitem( name user,
            string item_name,
            string item_description,
            uint8_t bind_on,
            uint32_t stack_quantity,
            uint32_t max_stack_size,
            uint8_t is_consumable,
            uint8_t quality,
            uint8_t slot,
            uint8_t profession_lock,
            vector<uint32_t> stats);
            
        [[eosio::action]]
        void combat( name user, uint64_t character_id, uint64_t encounter_id, uint8_t user_decision);

        [[eosio::action]]
        void move( name user, uint64_t character_id );

        [[eosio::action]]
        void newcharacter( name user, string character_name, uint8_t gender, uint8_t profession, uint8_t race );

        [[eosio::action]]
        void delcharacter( name user, uint64_t character_id );

        [[eosio::action]]
        void getbalance( name user );
        
        // Admin actions (privileged) - not intended for public use

        [[eosio::action]]
        void upsertstats( name user, uint8_t profession_id, uint8_t base_hp, uint8_t hp_increase_by_level, vector<uint8_t> stats, vector<uint8_t> stat_increase_by_level );


    private:
        enum class gender: uint8_t {
            MALE = 0,
            FEMALE = 1
        };

        enum class race: int8_t {
            NONE = -1,
            HUMAN = 0,
            EARTHEN = 1,
            AMAZON = 2,
            HIGHBORNE = 3,
            GOBLIN = 4,
            MECHA = 5
        };

        enum class profession: int8_t {
            NONE = -1,
            ROCKHOUND = 0,
            SURVIVALIST = 1,
            ALCHEMIST = 2,
            CONJUROR = 3,
            METALSMITH = 4
        };

        enum class slot: uint8_t {
            NONE = 0,
            HEAD = 1,
            OUTFIT = 2,
            WEAPON = 3,
            JEWELRY = 4
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

        enum class quality: uint8_t {
            JUNK = 0,
            POOR = 1,
            COMMON = 2,
            UNCOMMON = 3,
            RARE = 4,
            EPIC = 5,
            LEGENDARY = 6
        };

        enum class bind_on: uint8_t {
            NONE = 0,
            EQUIP = 1,
            ACCOUNT = 2
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
            uint8_t level;
            uint32_t experience;
            position position;
            uint8_t in_combat = 0;
            uint64_t hp = 0;
            uint64_t max_hp = 0;
            stats stats;
            equipment equipped;

            uint64_t primary_key() const { return character_id; }
        };

        typedef eosio::multi_index<"characters"_n, character> characters_table;

        // TODO: Add equip and use effects to auras
        struct [[eosio::table]] item {
            uint64_t item_id;
            string item_name;
            string item_description;
            uint8_t bind_on = 0;
            uint32_t stack_quantity = 1;
            uint32_t max_stack_size = 1;
            uint8_t is_consumable = 0;
            name owner;
            uint64_t equipped_by;
            stats stats;
            string creator_name;
            string creator_account;
            uint8_t quality = 0;
            uint8_t slot = 0;
            uint8_t profession_lock;
        };

        typedef eosio::multi_index<"items"_n, item> items_table;

        struct [[eosio::table]] account {
            name user;
            asset balance;
            uint8_t total_characters = 0;
            vector<character> characters;
            vector<item> inventory;

            auto primary_key() const { return user; }
        };

        typedef eosio::multi_index<"accounts"_n, account> accounts_table;

        struct [[eosio::table]] basestat {
            uint8_t profession_id;
            uint8_t base_hp;
            uint8_t hp_increase_by_level;
            stats base_stats;
            stats stat_increase_by_level;

            auto primary_key() const { return profession_id; }
        };

        typedef eosio::multi_index<"basestats"_n, basestat> base_stats_table;

        int random( const int range );

        uint64_t create_hash( const string& str );

        string to_hex( const uint8_t* data, uint32_t length );
};

