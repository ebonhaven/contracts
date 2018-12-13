#include "ebonhaven.hpp"
#include "util.cpp"

using namespace eosio;

// TODO: Implement
void ebonhaven::newitem( name user,
            string item_name,
            string item_description,
            uint8_t bind_on,
            uint32_t stack_quantity,
            uint32_t max_stack_size,
            uint8_t is_consumable,
            uint8_t quality,
            uint8_t slot,
            uint8_t profession_lock,
            vector<uint32_t> stats) {

            };

void ebonhaven::combat( name user, uint64_t character_id, uint64_t encounter_id, uint8_t user_decision) {
    require_auth( user );

    characters_table characters(_code, _code.value);
    auto& character = characters.get(character_id, "Character doesn't exist");
    eosio_assert(character.user == user, "Character doesn't belong to user");
    eosio_assert(character.in_combat == 1, "Character is not in combat");
    eosio_assert(user_decision >= 0 && user_decision < 3, "User decision not recognized");
    // Check if bribe possible

    auto roll = random(100);
    print("Roll is: ", roll);

    if (user_decision >= 0 && user_decision < 4) {
        print("Character choice is FIGHT");
        if (roll >= 42) {
            print(" Success. Get loot");
        } else if (roll < 42) {
            print(" Fail. Lose health");
        }
    } else if (user_decision == as_integer(combat_decision::FLEE)) {
        print("Character choice is FLEE");
        if (roll >= 35) {
            print(" Failed. Lose health x2");
        } else if (roll < 35) {
            print(" Success. No combat.");
        }
    } else if (user_decision == as_integer(combat_decision::BRIBE)) {
        print("Character choice is BRIBE");
        if (roll >= 25) {
            print(" Success. Deduct balance");
        } else if (roll < 25) {
            print(" Fail. Deduct balance and lose health");
        }
        // Check if user has required balance
    }

    characters.modify(character, user, [&](auto& modified ) {
        modified.in_combat = 0;
    });

};

void ebonhaven::move( name user, uint64_t character_id ) {
    require_auth( user );

    characters_table characters(_code, _code.value);
    auto& character = characters.get(character_id, "Character doesn't exist");
    eosio_assert(character.user == user, "Character doesn't belong to user");
    eosio_assert(character.in_combat < 1, "Character is in combat");

    auto num = random(300);
    print("Random number is: ", num);

    if (num >= 0 && num < 1) {
        print("Outcome is: LOOT");
    } else if (num >= 2 && num < 5) {
        print("Outcome is: TREASURE");
    } else if (num >= 5 && num < 16) {
        print("Outcome is: TRAP");
    } else if (num >= 16 && num < 20) {
        print("Outcome is: DISCOVERY");
    } else if (num >= 20 && num < 35) {
        print("Outcome is: RESOURCE");
    } else if (num >= 35 && num < 85) {
        // Generate in combat
        characters.modify(character, user, [&](auto& modified ) {
            modified.in_combat = 1;
        });
        print("Outcome is: COMBAT");
    } else {
        characters.modify(character, user, [&](auto& modified ) {
            modified.position.y += 1;
        });
        print("Outcome is: NONE");
    }

}

// Create character
void ebonhaven::newcharacter( name user, string character_name, uint8_t gender, uint8_t profession, uint8_t race ) {
    require_auth( user );

    eosio_assert(gender >= 0 && gender <= as_integer(gender::FEMALE), "Character must have a valid gender");
    eosio_assert(profession >= 0 && profession <= as_integer(profession::METALSMITH), "Character must have a valid profession");
    eosio_assert(race >= 0 && race <= as_integer(race::MECHA), "Character must have a valid race");

    // Find existing last id
    auto id_itr = _config.begin();

    // Initialize if not found or increment
    if (id_itr == _config.end()) {
        id_itr = _config.emplace( _self, [&](auto& globals) {});
    }

    characters_table characters(_code, _code.value);
    base_stats_table basestats(_code, _code.value);
    auto& base = basestats.get(profession, "Profession does not exist");

    //auto itr = characters.find(user.value);
    auto str = user.to_string() + character_name + std::to_string(current_time());
    auto hash = create_hash(str);

    characters.emplace(user, [&](auto& row ) {
        row.character_id = hash;
        row.user = user;
        row.character_name = character_name;
        row.gender = gender;
        row.profession = profession;
        row.race = race;
        row.level = 1;
        row.hp = base.base_hp;
        row.max_hp = base.base_hp;
        row.stats = base.base_stats;
        row.experience = 0;
    });
}

void ebonhaven::delcharacter( name user, uint64_t character_id ) {
    require_auth( user );
    characters_table characters(_code, _code.value);
    auto& character = characters.get(character_id, "Record does not exist");
    eosio_assert(character.user == user, "Character doesn't belong to user");

    auto itr = characters.find(character_id);
    characters.erase(itr);
}

void ebonhaven::getbalance( name user ) {
    require_auth( user );

    token t(N(eosio.token));
    auto sym_name = symbol_type(S(4,EBH)).name();
    auto balance = t.get_balance(N(user), sym_name);
    print("My balance is: ", balance);
};

void ebonhaven::upsertstats( name user, uint8_t profession_id, uint8_t base_hp, uint8_t hp_increase_by_level, vector<uint8_t> new_stats, vector<uint8_t> stat_increase_by_level ) {
    require_auth( user );
    base_stats_table basestats(_code, _code.value);
    auto itr = basestats.find(profession_id);

    stats s_stats = { new_stats[0], new_stats[1], new_stats[2], new_stats[3], new_stats[4] };
    stats s_increase = { stat_increase_by_level[0], stat_increase_by_level[1], stat_increase_by_level[2], stat_increase_by_level[3], stat_increase_by_level[4] };

    if ( itr == basestats.end() ) {
        basestats.emplace(user, [&](auto& row) {
            row.profession_id = profession_id;
            row.base_hp = base_hp;
            row.hp_increase_by_level = hp_increase_by_level;
            row.base_stats = s_stats;
            row.stat_increase_by_level = s_increase;
        });
    } else {
        basestats.modify(itr, user, [&](auto& row) {
            row.profession_id = profession_id;
            row.base_hp = base_hp;
            row.hp_increase_by_level = hp_increase_by_level;
            row.base_stats = s_stats;
            row.stat_increase_by_level = s_increase;
        });
    }
    
}

EOSIO_DISPATCH( ebonhaven, (newcharacter)(delcharacter)(move)(combat)(upsertstats)(getbalance) );