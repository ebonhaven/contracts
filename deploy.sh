CONTRACTSPATH="$( pwd -P )/src/ebonhaven"
cd $CONTRACTSPATH

echo "=== compiling and deploying ==="
(
    eosio-cpp -o ebonhaven.wasm ebonhaven.cpp -abigen
) &&

cleos set contract ebonhaven ../ebonhaven -p ebonhaven@active

cd $( pwd -P )