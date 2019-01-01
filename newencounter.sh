NAME=$1
CHARID=$2

echo "=== create basic encounter ==="
cleos push action ebonhaven newencounter '["'$NAME'", "'$CHARID'", [101]]' -p $NAME@active 