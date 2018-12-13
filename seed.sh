echo "=== seed base stats ==="
cleos push action ebonhaven upsertstats '['ebonhaven', 0, 200, 12, [7, 5, 10, 8, 5], [2, 1, 3, 2, 1]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '['ebonhaven', 1, 180, 10, [5, 10, 7, 5, 8], [1, 3, 2, 1, 2]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '['ebonhaven', 2, 160, 8, [4, 8, 8, 8, 7], [1, 2, 2, 2, 2]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '['ebonhaven', 3, 140, 8, [5, 8, 7, 11, 4], [1, 2, 2, 3, 1]]' -p ebonhaven@active
cleos push action ebonhaven upsertstats '['ebonhaven', 4, 220, 14, [9, 4, 6, 9, 7], [3, 2, 1, 2, 1]]' -p ebonhaven@active

# echo "=== create items ==="

echo "=== create characters ==="
cleos push action ebonhaven newcharacter '['alice', 'Alice', 1, 1, 1]' -p alice@active
cleos push action ebonhaven newcharacter '['bob', 'Bob', 0, 0, 1]' -p bob@active

