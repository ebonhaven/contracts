echo "=== deploying eosio.token ==="
cd ./eosio.contracts/contracts/eosio.token

eosio-cpp -I include -o eosio.token.wasm src/eosio.token.cpp --abigen

cleos set contract eosio.token ../eosio.token --abi eosio.token.abi -p eosio.token@active

cd ../../scripts