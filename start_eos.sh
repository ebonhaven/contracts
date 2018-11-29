keosd >> keosd.log 2>&1 & nodeos -e -p eosio \
    --plugin eosio::producer_plugin \
    --plugin eosio::chain_api_plugin \
    --plugin eosio::http_plugin \
    -d data/eosio/data \
    --config-dir data/eosio/config \
    --access-control-allow-origin=* \
    --contracts-console \
    --http-validate-host=false \
    —filter-on=‘*’ >> nodeos.log 2>&1