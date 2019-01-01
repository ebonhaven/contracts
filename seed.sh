echo "=== seed base stats ==="
cleos push action ebonhaven upsertstats '["ebonhaven", 1, 200, 12, [18, 12], [18, 12], [7, 5, 10, 8, 5], [4, 2], [4, 2], [2, 1, 3, 2, 1]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '["ebonhaven", 2, 180, 10, [20, 10], [15, 15], [5, 10, 7, 5, 8], [5, 1], [5, 1], [1, 3, 2, 1, 2]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '["ebonhaven", 3, 160, 8, [12, 18], [18, 12], [4, 8, 8, 8, 7], [3, 3], [3, 3], [1, 2, 2, 2, 2]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '["ebonhaven", 4, 140, 8, [10, 22], [10, 20], [5, 8, 7, 11, 4], [1, 5], [1, 5], [1, 2, 2, 3, 1]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '["ebonhaven", 5, 220, 14, [16, 14], [20, 10], [9, 4, 6, 9, 7], [4, 2], [5, 1], [3, 2, 1, 2, 1]]' -p ebonhaven@active

echo "=== create tokens ==="
cleos push action ebonhaven tokencreate '["ebonhaven", "1000000000.0000 EBH"]' -p ebonhaven@active

echo "=== create base account ==="
cleos push action ebonhaven newaccount '["ebonhaven"]' -p ebonhaven@active

echo "=== create auras ==="
cleos push action ebonhaven upsertaura '["ebonhaven", 101, "", 0, 1, 0, 250, [0, 0, 0, 0, 0], [0, 0], [0, 0]]' -p ebonhaven@active

# user, item_id, item_name, item_description, bind_on, stack_quantity, max_stack_size, is_consumable, level_requirement, quality, slot, profession_lock, stats, aura_id
echo "=== create items ==="
cleos push action ebonhaven newitem '["ebonhaven", 101, "Meat Cleaver", "", 0, 1, 1, 0, 1, 2, 3, 0, 0]' -p ebonhaven@active
cleos push action ebonhaven newitem '["ebonhaven", 102, "Staff of Enders", "", 0, 1, 1, 0, 1, 2, 3, 0, 0]' -p ebonhaven@active
cleos push action ebonhaven newitem '["ebonhaven", 202, "Health Potion", "", 0, 1, 10, 1, 1, 2, 0, 0, 101]' -p ebonhaven@active

echo "=== create drops ==="
cleos push action ebonhaven upsertdrop '["ebonhaven", 101, [{"percentage":100.0, "item_id":101}, {"percentage":25.0, "item_id":102}]]' -p ebonhaven@active

echo "=== create mobs ==="
cleos push action ebonhaven upsertmob '["ebonhaven", 101, "Kobold", 1, 0, 5, 5, 250, 250, 0, 0, 0, 200, "0.0500 EBH", 101]' -p ebonhaven@active

echo "=== create characters ==="
cleos push action ebonhaven newcharacter '["alice", "Alice", 1, 2, 1]' -p alice@active
cleos push action ebonhaven newcharacter '["bob", "Bob", 0, 1, 2]' -p bob@active

echo "=== issue tokens ==="
cleos push action ebonhaven tokenissue '["alice", "100.0000 EBH", "initial"]' -p ebonhaven@active
cleos push action ebonhaven tokenissue '["bob", "25.0000 EBH", "initial"]' -p ebonhaven@active
