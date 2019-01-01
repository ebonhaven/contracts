#include "ebonhaven.hpp"
#include "util.cpp"

void ebonhaven::newitem( name     user,
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
                         uint64_t aura_id)
{
    require_auth( user );

    items_table items(_code, _code.value);
    
    item ni = {};
    ni.item_id = item_id;
    ni.item_name = item_name;
    ni.item_description = item_description;
    ni.bind_on = bind_on;
    ni.stack_quantity = stack_quantity;
    ni.max_stack_size = max_stack_size;
    ni.is_consumable = is_consumable;
    ni.level_requirement = level_requirement;
    ni.quality = quality;
    ni.slot = slot;
    ni.profession_lock = profession_lock;

    if (aura_id > 0) {
        auras_table auras(_code, _code.value);
        auto itr = auras.find(aura_id);
        if (itr != auras.end()) {
            auto aura = auras.get(aura_id, "Couldn't get aura");
            ni.aura = aura;
        }
    }

    ebonhaven::upsert_item( user, ni );

};

void ebonhaven::combat( name user, uint64_t encounter_id, uint8_t user_decision) {
    require_auth( user );
    encounters_table encounters(_code, _code.value);
    auto& encounter = encounters.get(encounter_id, "Encounter not found");

    characters_table characters(_code, _code.value);
    auto& character = characters.get(encounter.character_id, "Character doesn't exist");
    eosio_assert(character.user == user, "Character doesn't belong to user");
    eosio_assert(character.in_combat == 1, "Character is not in combat");
    eosio_assert(user_decision >= 0 && user_decision < 3, "User decision not recognized");
    // Check if bribe possible

    auto roll = random(100);
    print("Roll is: ", roll);

    if (user_decision >= 0 && user_decision < 4) {
        print(" Character choice is FIGHT");
        if (roll >= 24) {
            print(" Success. Damage mob");

             auto itr = encounters.find(encounter_id); 
            if (encounter.mobs[0].hp - 50 > 0) {      
                encounters.modify(itr, user, [&](auto& e) {
                    e.mobs[0].hp -= 50;
                });
            } else {
                encounters.modify(itr, user, [&](auto& e) {
                    e.mobs[0].hp -= 50;
                });
                finalize_encounter( user, encounter );
                // Get drops
                print(" Combat over. Calculate drops");
            }
        } else if (roll < 24) {
            print(" Fail. Lose health");
            auto c_itr = characters.find(encounter.character_id);
            if (character.hp - 50 > 0) {
                characters.modify(c_itr, user, [&](auto& c) {
                    c.hp -= 50;
                });
                print(" HP updated");
            } else {
                print(" Not enough health. KO'd.");
                characters.modify(c_itr, user, [&](auto& c) {
                    c.hp = 0;
                    c.in_combat = 0;
                });
                auto itr = encounters.find(encounter_id);    
                encounters.erase(itr);
            }
        }
    } else if (user_decision == as_integer(combat_decision::FLEE)) {
        print(" Character choice is FLEE");
        if (roll >= 35) {
            print(" Failed. Lose health x2");
        } else if (roll < 35) {
            print(" Success. No combat.");
        }
    } else if (user_decision == as_integer(combat_decision::BRIBE)) {
        print(" Character choice is BRIBE");
        if (roll >= 25) {
            print(" Success. Deduct balance");
        } else if (roll < 25) {
            print(" Fail. Deduct balance and lose health");
        }
        // Check if user has required balance
    }
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

    characters_table characters(_code, _code.value);
    base_stats_table basestats(_code, _code.value);
    auto& base = basestats.get(profession, "Profession does not exist");

    // Create account if none exists
    accounts_table accounts(_code, _code.value);
    auto ac_itr = accounts.find( user.value );
    if( ac_itr == accounts.end() ) {
        ac_itr = accounts.emplace( user, [&](auto& ac ) {
            ac.user = user;
        });
    }

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
        row.hp = base.base_hp;
        row.max_hp = base.base_hp;
        row.attack = base.base_attack;
        row.defense = base.base_defense;
        row.stats = base.base_stats;
    });

    accounts.modify(ac_itr, user, [&](auto& ac) {
        ac.characters.push_back(hash);
    });
}

void ebonhaven::delcharacter( name user, uint64_t character_id ) {
    require_auth( user );
    characters_table characters(_code, _code.value);
    auto& character = characters.get(character_id, "Record does not exist");
    eosio_assert(character.user == user, "Character doesn't belong to user");

    auto itr = characters.find(character_id);
    characters.erase(itr);

    accounts_table accounts(_code, _code.value);
    auto ac_itr = accounts.find( user.value );
    accounts.modify(ac_itr, user, [&](auto& ac) {
        auto c_itr = find(ac.characters.begin(), ac.characters.end(), character_id); 
        if ( c_itr != ac.characters.end() ) {
            ac.characters.erase(c_itr);
        }
    });
    
}

void ebonhaven::upsertmob( name     user,
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
                        asset    worth,
                        uint64_t drop_id) {
    require_auth( user );

    drops_table drops(_code, _code.value);
    auto& s_drop = drops.get(drop_id, "Drop not found");

    mobs_table mobs(_code, _code.value);
    auto itr = mobs.find(mob_id); 

    if ( itr == mobs.end() ) {
        mobs.emplace( user, [&]( auto& m ) {
            m.mob_id = mob_id;
            m.mob_name = mob_name;
            m.level = level;
            m.mob_type = mob_type;
            m.attack = attack;
            m.defense = defense;
            m.hp = hp;
            m.max_hp = max_hp;
            m.skill1 = skill1;
            m.skill2 = skill2;
            m.skill3 = skill3;
            m.experience = experience;
            m.worth = worth;
            m.drop = s_drop;
        });
    } else {
        mobs.modify( itr, user, [&]( auto& m ) {
            m.mob_name = mob_name;
            m.level = level;
            m.mob_type = mob_type;
            m.attack = attack;
            m.defense = defense;
            m.hp = hp;
            m.max_hp = max_hp;
            m.skill1 = skill1;
            m.skill2 = skill2;
            m.skill3 = skill3;
            m.experience = experience;
            m.worth = worth;
            m.drop = s_drop;
        });
    }
    

};

void ebonhaven::newencounter( name user, uint64_t character_id, vector<uint64_t> mob_ids) {
    require_auth( user );
    encounters_table encounters(_code, _code.value);
    for (auto& enc: encounters) {
        eosio_assert(enc.character_id != character_id, "Encounter already exists for character");
    }

    characters_table characters(_code, _code.value);
    auto character = characters.get(character_id, "No character found");
    if ( character.in_combat == 0 ) {
        auto itr = characters.find(character.character_id);
        characters.modify(itr, user, [&](auto& c) {
            c.in_combat = 1;
        });
    }

    mobs_table mobs(_code, _code.value);
    vector<mob> v_mobs = {};
    for (uint8_t i = 0; i < mob_ids.size(); i++) {
        auto mob = mobs.get(mob_ids[i], "Can't find mob");
        v_mobs.push_back(mob);
    }

    auto str = user.to_string() + to_string(character_id) + to_string(current_time());
    auto hash = create_hash(str);

    encounters.emplace( user, [&]( auto& enc ) {
        enc.encounter_id = hash;
        enc.character_id = character_id;
        enc.mobs = v_mobs;
    });
};

void ebonhaven::delencounter( name user, uint64_t encounter_id ) {
    require_auth( user );
    encounters_table encounters(_code, _code.value);
    auto& encounter = encounters.get(encounter_id, "Record does not exist");

    auto itr = encounters.find(encounter_id);
    encounters.erase(itr);

    characters_table characters(_code, _code.value);
    auto c_itr = characters.find(encounter.character_id);
    if (c_itr != characters.end()) {
        characters.modify( c_itr, user, [&](auto& c) {
            c.in_combat = 0;
        });
    }
}

ebonhaven::reward ebonhaven::generate_reward( name user, encounter s_encounter ) {
    reward r = {};
    asset total_reward = asset(0, symbol(symbol_code("EBH"),4));
    vector<uint64_t> rewardItems;
    for (auto& mob: s_encounter.mobs) {
        print(" Mob worth: ", mob.worth.amount);
        total_reward.amount += mob.worth.amount;
        auto roll = random(100);
        print(" Comp roll: ", roll);
        for (auto& drop: mob.drop.drops) {
            uint8_t perc = abs(floor((100 * drop.percentage) / 100));
            print(" Rolled on drop: ", to_string(perc));
            if ( roll <= perc ) {
                print(" Item won: ", drop.item_id);
                rewardItems.push_back(drop.item_id);
            }
        }
    }

    items_table items(_code, _code.value);
    for (auto& id: rewardItems) {
        auto itr = items.find(id);
        if (itr != items.end()) {
            item newItem = items.get(id);
            newItem.parent_id = newItem.item_id;
            auto str = to_string(newItem.item_id) + newItem.item_name + to_string(current_time());
            auto hash = create_hash(str);
            newItem.item_id = hash;
            upsert_item( user, newItem );
            r.items.push_back( newItem );
        }
    }

    r.worth = total_reward;

    auto str = "reward" + to_string(current_time());
    auto hash = create_hash(str);
    r.reward_id = hash;
    r.character_id = s_encounter.character_id;

    print(" Total mob worth: ", total_reward);
    return r;
}

void ebonhaven::apply_experience( name user, encounter s_encounter ) {
    uint32_t total_exp = 0;
    characters_table characters(_code, _code.value);
    auto character = characters.get(s_encounter.character_id, "Couldn't find character");
    for (auto& mob: s_encounter.mobs) {
        if (character.level - mob.level >= -4 || character.level - mob.level <= 4) {
            print(" Mob experience: ", mob.experience);
            total_exp += mob.experience;
        }
    }
    auto itr = characters.find(s_encounter.character_id);
    characters.modify(itr, user, [&](auto& c) {
        c.experience += total_exp;
    });
}

void ebonhaven::finalize_encounter( name user, encounter s_encounter ) {
    reward r = generate_reward( user, s_encounter );
    upsert_rewards( user, r );

    // Apply experience to character
    apply_experience( user, s_encounter );


    // Create reward

    // Delete encounter
    encounters_table encounters(_code, _code.value);
    auto itr = encounters.find(s_encounter.encounter_id);
    encounters.erase(itr);
}

void ebonhaven::upsert_item( name user, item in_item ) {

    items_table items(_code, _code.value);
    auto itr = items.find(in_item.item_id);
    if ( itr != items.end() ) {
        items.modify( itr, user, [&](auto& it) {
            it.item_id = in_item.item_id;
            it.parent_id = in_item.parent_id;
            it.item_name = in_item.item_name;
            it.item_description = in_item.item_description;
            it.bind_on = in_item.bind_on;
            it.stack_quantity = in_item.stack_quantity;
            it.max_stack_size = in_item.max_stack_size;
            it.is_consumable = in_item.is_consumable;
            it.level_requirement = in_item.level_requirement;
            it.quality = in_item.quality;
            it.slot = in_item.slot;
            it.profession_lock = in_item.profession_lock;
            it.aura = in_item.aura;
        });
    } else {
        items.emplace( user, [&](auto& it) {
            it.item_id = in_item.item_id;
            it.parent_id = in_item.parent_id;
            it.item_name = in_item.item_name;
            it.item_description = in_item.item_description;
            it.bind_on = in_item.bind_on;
            it.stack_quantity = in_item.stack_quantity;
            it.max_stack_size = in_item.max_stack_size;
            it.is_consumable = in_item.is_consumable;
            it.level_requirement = in_item.level_requirement;
            it.quality = in_item.quality;
            it.slot = in_item.slot;
            it.profession_lock = in_item.profession_lock;
            it.aura = in_item.aura;
        });
    }
}

void ebonhaven::upsert_rewards( name user, reward in_reward ) {
    rewards_table rewards(_code, _code.value);
    auto itr = rewards.find(in_reward.reward_id);
    if (itr != rewards.end()) {
        rewards.modify(itr, user, [&](auto& r) {
            r.character_id = in_reward.character_id;
            r.worth = in_reward.worth;
            r.items = in_reward.items;
        });
    } else {
        rewards.emplace( user, [&](auto& r) {
            r.reward_id = in_reward.reward_id;
            r.character_id = in_reward.character_id;
            r.worth = in_reward.worth;
            r.items = in_reward.items;
        });
    }
}

void ebonhaven::claimrewards( name user, uint64_t reward_id, vector<uint64_t> item_ids) {
    require_auth( user );

    rewards_table rewards(_code, _code.value);
    auto reward = rewards.get(reward_id, "Reward not found");

    accounts_table accounts(_code, _code.value);
    auto acct = accounts.get( user.value );
    auto c_found = find(acct.characters.begin(), acct.characters.end(), reward.character_id) != acct.characters.end();
    print("Character id found: ", c_found);
    eosio_assert(c_found, "Character not found");

    // Transfer tokens
    //tokenreward( user, reward.worth );
    SEND_INLINE_ACTION( *this, tokenreward,
                        { { user , "active"_n} },
                        { user, reward.worth } );

    // Assign ownership to items
    items_table items(_code, _code.value);
    for (auto& item: reward.items) {
        auto itr = items.find(item.item_id);
        auto i_found = find(item_ids.begin(), item_ids.end(), item.item_id) != item_ids.end();
        if (i_found) {
            items.modify(itr, user, [&](auto& i) {
                i.owner = user;
            });
            auto a_itr = accounts.find(user.value);
            accounts.modify(a_itr, user, [&](auto& a) {
                a.inventory.push_back(item.item_id);
            });
        } else {
            items.erase(itr);
        }
    }

    delrewards( user, reward_id );
}

void ebonhaven::delrewards( name user, uint64_t reward_id ) {
    require_auth( user );
    rewards_table rewards(_code, _code.value);
    auto& reward = rewards.get(reward_id, "Record does not exist");

    auto itr = rewards.find(reward_id);
    rewards.erase(itr);
}

void ebonhaven::upsertaura( name user,
                            uint64_t aura_id,
                            string aura_description,
                            uint8_t has_charges,
                            uint8_t charges_remaining,
                            uint8_t turns_remaining,
                            int32_t hp_delta,
                            vector<uint32_t> stats_delta,
                            vector<uint32_t> attack_delta,
                            vector<uint32_t> defense_delta ) {
    require_auth( user );

    stats s_stats = { stats_delta[0], stats_delta[1], stats_delta[2], stats_delta[3], stats_delta[4] };
    attack s_attack = { attack_delta[0], attack_delta[1] };
    defense s_defense = { defense_delta[0], defense_delta[1] };

    auras_table auras(_code, _code.value);
    auto itr = auras.find(aura_id);
    if (itr != auras.end()) {
        auras.modify(itr, user, [&](auto& a) {
            a.aura_description = aura_description;
            a.has_charges = has_charges;
            a.charges_remaining = charges_remaining;
            a.turns_remaining = turns_remaining;
            a.hp_delta = hp_delta;
            a.stats_delta = s_stats;
            a.attack_delta = s_attack;
            a.defense_delta = s_defense;
        });
    } else {
        auras.emplace(user, [&](auto& a) {
            a.aura_id = aura_id;
            a.aura_description = aura_description;
            a.has_charges = has_charges;
            a.charges_remaining = charges_remaining;
            a.turns_remaining = turns_remaining;
            a.hp_delta = hp_delta;
            a.stats_delta = s_stats;
            a.attack_delta = s_attack;
            a.defense_delta = s_defense;
        });
    }

    items_table items(_code, _code.value);
    for (auto& item: items) {
        if (item.aura.aura_id == aura_id) {
            auto itr = items.find(item.item_id);
            items.modify(itr, user, [&](auto& i) {
                i.aura.aura_description = aura_description;
                i.aura.has_charges = has_charges;
                i.aura.hp_delta = hp_delta;
                i.aura.stats_delta = s_stats;
                i.aura.attack_delta = s_attack;
                i.aura.defense_delta = s_defense;
            });
        }
    }

}

void ebonhaven::useitem( name user, uint64_t character_id, uint64_t item_id ) {
    require_auth( user );

    accounts_table accounts(_code, _code.value);
    characters_table characters(_code, _code.value);
    items_table items(_code, _code.value);
    auto c_itr = characters.find(character_id);
    eosio_assert(c_itr != characters.end(), "Character not found");
    auto character = characters.get(character_id);
    eosio_assert(character.hp != 0, "Character KO'd");
    
    auto acct = accounts.get(user.value);
    auto itr = find(acct.inventory.begin(), acct.inventory.end(), item_id);
    eosio_assert(itr != acct.inventory.end(), "Couldn't find item");
    auto item = items.get(item_id);
    if (item.is_consumable > 0) {
        
        if (item.aura.hp_delta > 0) {
            int64_t change = character.hp + item.aura.hp_delta;
            if (change > character.max_hp) {
                change = character.max_hp;
            }
            characters.modify(c_itr, user, [&](auto& c) {
                c.hp = change;
            });
        }

        auto i_itr = items.find(item_id);
        if (item.stack_quantity == 1) {
            items.erase(i_itr);
            auto a_itr = accounts.find(user.value);
            vector<uint64_t> newInventory(acct.inventory);
            auto idx = find(newInventory.begin(), newInventory.end(), item_id);
            if (idx != newInventory.end()) {
                newInventory.erase(idx);
                accounts.modify( a_itr, user, [&](auto& a) {
                    a.inventory = newInventory;
                });
            }
            
        } else {
            items.modify(i_itr, user, [&](auto& i) {
                i.stack_quantity -= 1;
            });
        }
        
    } 

};