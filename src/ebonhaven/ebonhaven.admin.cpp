#include "ebonhaven.hpp"
#include "util.cpp"

using namespace std;
using namespace eosio;

void ebonhaven::upsertstats( name    user, 
                             uint8_t profession_id,
                             uint8_t base_hp,
                             uint8_t hp_increase_by_level,
                             vector<uint32_t> base_attack,
                             vector<uint32_t> base_defense,
                             vector<uint32_t> new_stats,
                             vector<uint32_t> attack_increase,
                             vector<uint32_t> defense_increase,
                             vector<uint32_t> stat_increase_by_level) {
    require_auth( user );
    base_stats_table basestats(_code, _code.value);
    auto itr = basestats.find(profession_id);

    stats s_stats = { new_stats[0], new_stats[1], new_stats[2], new_stats[3], new_stats[4] };
    attack s_attack = { base_attack[0], base_attack[1] };
    defense s_defense = { base_defense[0], base_defense[1] };
    stats s_increase = { stat_increase_by_level[0], stat_increase_by_level[1], stat_increase_by_level[2], stat_increase_by_level[3], stat_increase_by_level[4] };
    attack s_attack_increase = { attack_increase[0], attack_increase[1] };
    defense s_defense_increase = { defense_increase[0], defense_increase[1] };

    if ( itr == basestats.end() ) {
        basestats.emplace(user, [&](auto& row) {
            row.profession_id = profession_id;
            row.base_hp = base_hp;
            row.hp_increase_by_level = hp_increase_by_level;
            row.base_attack = s_attack;
            row.base_defense = s_defense;
            row.base_stats = s_stats;
            row.attack_increase_by_level = s_attack_increase;
            row.defense_increase_by_level = s_defense_increase;
            row.stat_increase_by_level = s_increase;
        });
    } else {
        basestats.modify(itr, user, [&](auto& row) {
            row.base_hp = base_hp;
            row.hp_increase_by_level = hp_increase_by_level;
            row.base_attack = s_attack;
            row.base_defense = s_defense;
            row.base_stats = s_stats;
            row.attack_increase_by_level = s_attack_increase;
            row.defense_increase_by_level = s_defense_increase;
            row.stat_increase_by_level = s_increase;
        });
    }
    
}

void ebonhaven::newaccount( name user ) {
    require_auth( user );
    accounts_table accounts(_code, _code.value);
    auto itr = accounts.find( user.value );
    eosio_assert(itr == accounts.end(), "Account already exists");
    if (itr == accounts.end() ) {
        accounts.emplace( user , [&](auto& ac) {
            ac.user = user;
        });
    }
}

void ebonhaven::upsertdrop( name user, uint64_t drop_id, vector<mob_drop> v_drops) {
    require_auth( user );
    drops_table drops(_code, _code.value);
    
    for ( auto& drop: v_drops ) {
        drop.percentage = floor( drop.percentage * 150 ) / 150;
    }

    auto itr = drops.find( drop_id );
    if ( itr == drops.end() ) {
        drops.emplace( user, [&](auto& d) {
            d.drop_id = drop_id;
            d.drops = v_drops;
        });
    } else {
        drops.modify( itr, user, [&](auto& d) {
            d.drops = v_drops;
        });
    }

    mobs_table mobs(_code, _code.value);
    vector<uint64_t> mobsForUpdate;
    for (auto& mob: mobs) {
        if (mob.drop.drop_id == drop_id) {
            mobsForUpdate.push_back(mob.mob_id);
        }
    }

    s_drop new_drop = { drop_id, v_drops };
    for (uint64_t key: mobsForUpdate) {
        auto itr = mobs.find(key);
        if ( itr != mobs.end() ) {
            mobs.modify(itr, user, [&](auto& m) {
                m.drop = new_drop;
            });
        }
    }
}

void ebonhaven::spawnitems( name user, name to, vector<uint64_t> item_ids) {
    require_auth( user );
    accounts_table accounts(_code, _code.value);
    auto a_itr = accounts.find(to.value);
    auto acct = accounts.get(to.value, "Character not found");
    eosio_assert(item_ids.size() > 0, "Must supply more than 1 item id");

    vector<item> newItems;
    items_table items(_code, _code.value);
    for (auto& id: item_ids) {
        auto item = items.get(id, "Couldn't find item");
        item.parent_id = id;
        auto str = to_string(item.item_id) + item.item_name + to_string(current_time());
        auto hash = create_hash(str);
        item.item_id = hash;
        item.owner = to;
        newItems.push_back(item);
    }

    vector<item> consolidatedItems = consolidate_inventory( user, acct, newItems );

    vector<uint64_t> newInventory(acct.inventory);
    for (auto& item: consolidatedItems) {
        upsert_item( user, item );
        newInventory.push_back(item.item_id);
    }
    
    accounts.modify( a_itr, user, [&](auto& a) {
        a.inventory = newInventory;
    });

}

vector<ebonhaven::item> ebonhaven::consolidate_inventory( name payer, account acct, vector<ebonhaven::item> newItems ) {

    vector<uint64_t> idsToErase;
    vector<ebonhaven::item> itemsToErase;
    items_table items(_code, _code.value);
    for (auto i: acct.inventory) {
        auto found = find(idsToErase.begin(), idsToErase.end(), i) != idsToErase.end();
        auto i_item = items.get(i, "Couldn't find item");
        print("Checking item: ", i);
        if (!found && i_item.stack_quantity < i_item.max_stack_size ) {
            vector<uint64_t> toCheck(acct.inventory);
            auto idx = find(toCheck.begin(), toCheck.end(), i);
            toCheck.erase(idx);
            for (auto j: toCheck) {
                print(" against item ", j);
                auto j_item = items.get(j, "Couldn't get item");
                if (i_item.parent_id == j_item.parent_id && i_item.max_stack_size > 1 && i_item.stack_quantity < i_item.max_stack_size && j_item.stack_quantity < j_item.max_stack_size) {
                    print("Found matching item. Consolidating.");
                    auto res = j_item.stack_quantity + i_item.stack_quantity;
                    if (res <= i_item.max_stack_size) {
                        auto itr = items.find(i);
                        items.modify( itr, payer, [&](auto& _i) {
                            _i.stack_quantity = res;
                        });
                        itemsToErase.push_back(j_item);
                        idsToErase.push_back(j);
                    } else if (res > i_item.max_stack_size) {
                        auto itr = items.find(i);
                        items.modify( itr, payer, [&](auto& _i) {
                            _i.stack_quantity = i_item.max_stack_size;
                        });
                        itr = items.find(j);
                        if (i_item.max_stack_size - j_item.stack_quantity > 0) {
                            items.modify( itr, payer, [&](auto& _j) {
                                _j.stack_quantity = i_item.max_stack_size - j_item.stack_quantity;
                            });
                        } else {
                            itemsToErase.push_back(j_item);
                            idsToErase.push_back(j);
                        }
                    }
                }
            }

            for (auto k: newItems) {
                if (i_item.parent_id == k.parent_id && i_item.max_stack_size > 1 && i_item.stack_quantity < i_item.max_stack_size) {
                    auto res = k.stack_quantity + i_item.stack_quantity;
                    if (res <= i_item.max_stack_size) {
                        auto itr = items.find(i);
                        items.modify( itr, payer, [&](auto& _i) {
                            _i.stack_quantity = res;
                        });
                        idsToErase.push_back(k.item_id);
                    }
                }
            }
        }
    }

    if (itemsToErase.size() > 0) {
        for (auto& item: itemsToErase) {
            auto itr = items.find(item.item_id);
            items.erase( itr );
        }
    }

    if (idsToErase.size() > 0) {
        vector<uint64_t> newInventory(acct.inventory);
        newInventory.erase(
            remove_if(
                newInventory.begin(),
                newInventory.end(),
                [&idsToErase](uint64_t const & i) { return find(idsToErase.begin(), idsToErase.end(), i) != idsToErase.end(); }
            ),
            newInventory.end()  
        );

        accounts_table accounts(_code, _code.value);
        auto itr = accounts.find(acct.user.value);
        accounts.modify( itr, payer, [&](auto& a) {
            a.inventory = newInventory;
        });

        newItems.erase(
            remove_if(
                newItems.begin(),
                newItems.end(),
                [&idsToErase](ebonhaven::item const & i) { return find(idsToErase.begin(), idsToErase.end(), i.item_id) != idsToErase.end(); }
            ),
            newItems.end()
        );
    }

    return newItems;


}