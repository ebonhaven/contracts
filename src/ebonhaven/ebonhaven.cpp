#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/transaction.hpp>
#include <eosiolib/crypto.h>
#include <eosiolib/print.hpp>
#include <eosiolib/fixedpoint.hpp>
#include <functional>
#include <string.h>
#include <math.h>

using namespace std;
using namespace eosio;

#include "table/item.hpp"
#include "table/encounter.hpp"
#include "ebonhaven.hpp"
#include "ebonhaven.game.cpp"
#include "ebonhaven.token.cpp"
#include "ebonhaven.admin.cpp"

EOSIO_DISPATCH( ebonhaven, 
    (newcharacter)
    (delcharacter)
    (move)
    (combat)
    (tokencreate)
    (tokenreward)
    (tokenissue)
    (tokenretire)
    (tokenxfer)
    (upsertstats)
    (upsertdrop)
    (upsertmob)
    (newaccount)
    (newitem)
    (useitem)
    (spawnitems)
    (newencounter)
    (delencounter)
    (claimrewards)
    (delrewards)
    (upsertaura)
);