ROOT="$( pwd -P )/"

echo "=== reset data ==="
pkill nodeos
rm -rf data/
echo "" > nodeos.log
echo "" > keosd.log

echo "=== setup blockchain accounts and smart contract ==="

# start nodeos ( local node of blockchain )
nodeos -e -p eosio \
    --plugin eosio::producer_plugin \
    --plugin eosio::chain_api_plugin \
    --plugin eosio::http_plugin \
    -d data/eosio/data \
    --config-dir data/eosio/config \
    --access-control-allow-origin=* \
    --contracts-console \
    --http-validate-host=false \
    --verbose-http-errors > nodeos.log 2>&1 &
sleep 1s
until curl localhost:8888/v1/chain/get_info
do
  sleep 1s
done

# Sleep for 2 to allow time 4 blocks to be created so we have blocks to reference when sending transactions
sleep 2s

# First key import is for eosio system account
echo "=== unlock default wallet ==="
cleos wallet unlock --password PW5Jy8pnfEXKQbWb4KNsCmbbA5Vdkt9D1rpYbbQWgBaeqhhbSZznJ

echo "=== start create accounts in blockchain ==="
cleos create account eosio ebonhaven EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio alice EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV
cleos create account eosio bob EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV