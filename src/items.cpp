#include <ebonhaven.hpp>

//Admin
ACTION ebonhaven::create( name rev_partner,
                          name category,
                          name token_name,
                          bool fungible,
                          bool burnable,
                          bool sellable,
                          bool transferable,
                          double rev_split,
                          stat_attributes attributes,
                          string base_uri,
                          asset max_supply )
{
  require_auth( get_self() );

  _checkasset( max_supply, fungible );
  check( is_account( rev_partner), "rev_partner account does not exist" );
  // check split frac is between 0 and 1
  check( ( rev_split <= 1.0 ) && (rev_split >= 0.0), "rev_split must be between 0 and 1" );

  // get category_name_id
  globals_index config_table( get_self(), get_self().value );
  check(config_table.exists(), "Symbol table does not exist, setconfig first");
  auto config_singleton  = config_table.get();
  auto category_name_id = config_singleton.category_name_id;

  category_index category_table( get_self(), get_self().value );
  auto existing_category = category_table.find( category.value );
  // category hasn't been created before, create it

  if ( existing_category == category_table.end() ) {
    category_table.emplace( get_self(), [&]( auto& cat ) {
      cat.category = category;
    });
  }

  asset current_supply = asset( 0, symbol( config_singleton.symbol, max_supply.symbol.precision() ));
  asset issued_supply = asset( 0, symbol( config_singleton.symbol, max_supply.symbol.precision() ));

  stats_index stats_table( get_self(), category.value );
  auto existing_token = stats_table.find( token_name.value );
  check( existing_token == stats_table.end(), "Token with category and token_name exists" );
  // token type hasn't been created, create it
  stats_table.emplace( get_self(), [&]( auto& stats ) {
    stats.category_name_id = category_name_id;
    stats.issuer = name("ebonhavencom");
    stats.rev_partner = rev_partner;
    stats.token_name = token_name;
    stats.fungible = fungible;
    stats.burnable = burnable;
    stats.sellable = sellable;
    stats.transferable = transferable;
    stats.current_supply = current_supply;
    stats.issued_supply = issued_supply;
    stats.rev_split = rev_split;
    stats.attributes = attributes;
    stats.base_uri = base_uri;
    stats.max_supply = max_supply;
  });

  // successful creation of token, update category_name_id to reflect
  config_singleton.category_name_id++;
  config_table.set( config_singleton, get_self() );
}

ACTION ebonhaven::issue( name to,
                         name category,
                         name token_name,
                         asset quantity,
                         string relative_uri,
                         string creator_name,
                         string memo ) 
{
  check( is_account( to ), "to account does not exist");
  check( memo.size() <= 256, "memo has more than 256 bytes" );

  // ebonhavendgdtats table
  stats_index stats_table( get_self(), category.value );
  const auto& dgood_stats = stats_table.get( token_name.value,
                                             "token with category and token_name does not exist" );

  // ensure have issuer authorization and valid quantity
  require_auth( dgood_stats.issuer );

  _checkasset( quantity, dgood_stats.fungible );
  string string_precision = "precision of quantity must be " + to_string( dgood_stats.max_supply.symbol.precision() );
  check( quantity.symbol == dgood_stats.max_supply.symbol, string_precision.c_str() );
  // check cannot issue more than max supply, careful of overflow of uint
  check( quantity.amount <= (dgood_stats.max_supply.amount - dgood_stats.current_supply.amount), "cannot issue more than max supply" );

  dgood_attributes attr;
  attr.bind_status = dgood_stats.attributes.item_binding;
  attr.item_status = 0;
  attr.durability = dgood_stats.attributes.max_durability;
  attr.creator_name = creator_name;
  attr.created_at = time_point_sec(current_time_point());

  if (dgood_stats.fungible == false) {
    check( quantity.amount <= 100, "can issue 100 at a time");
    // check cannot issue more than max supply, careful of overflow of uint
    if (quantity.amount > 1 ) {
      asset issued_supply = dgood_stats.issued_supply;
      asset one_token = asset( 1, dgood_stats.max_supply.symbol);

      for (uint64_t i = 1; i <= quantity.amount; i++ ) {
        _mint(to, dgood_stats.issuer, category, token_name,
          dgood_stats.issued_supply, relative_uri, attr);
          issued_supply += one_token;
      }
    } else {
      _mint(to, dgood_stats.issuer, category, token_name,
          dgood_stats.issued_supply, relative_uri, attr);
    } 
  }

  _add_balance(to, dgood_stats.issuer, category, token_name, dgood_stats.category_name_id, quantity);

  // increase current supply
  stats_table.modify( dgood_stats, same_payer, [&]( auto& s ) {
      s.current_supply += quantity;
      s.issued_supply += quantity;
  });
}

ACTION ebonhaven::transfernft( name from,
                               name to,
                               vector<uint64_t> dgood_ids,
                               string memo ) {
  // ensure authorized to send from account
  check( dgood_ids.size() <= 20, "max batch size of 20" );

  check( from != to, "cannot transfer to self" );
  require_auth( from );

  // ensure 'to' account exists
  check( is_account( to ), "to account does not exist");
  
  accounts_index accounts_table( get_self(), to.value );
  auto acct = accounts_table.get(to.value, "account not found");

  // check memo size
  check( memo.size() <= 256, "memo has more than 256 bytes" );

  _changeowner( from, to, dgood_ids, memo, true );
}

ACTION ebonhaven::burnnft(name owner, vector<uint64_t> dgood_ids)
{
  require_auth(owner);

  check( dgood_ids.size() <= 20, "max batch size of 20" );
  // loop through vector of dgood_ids, check token exists
  lock_index lock_table( get_self(), get_self().value );
  dgoods_index dgood_table( get_self(), get_self().value );

  for ( auto const& dgood_id: dgood_ids ) {
    const auto& token = dgood_table.get( dgood_id, "token does not exist" );
    check( token.owner == owner, "must be token owner" );

    stats_index stats_table( get_self(), token.category.value );
    const auto& dgood_stats = stats_table.get( token.token_name.value, "dgood stats not found" );

    check( dgood_stats.burnable == true, "not burnable");
    check( dgood_stats.fungible == false, "cannot call burnnft on fungible token, call burn instead");

    auto locked_nft = lock_table.find( dgood_id );
    check(locked_nft == lock_table.end(), "token locked");

    asset quantity(1, dgood_stats.max_supply.symbol);

    // decrease current supply
    stats_table.modify( dgood_stats, same_payer, [&]( auto& s ) {
        s.current_supply -= quantity;
    });

    // lower balance from owner
    _sub_balance(owner, dgood_stats.category_name_id, quantity);

    // erase token
    dgood_table.erase( token );
  }
}

ACTION ebonhaven::listsalenft( name seller,
                               vector<uint64_t> dgood_ids,
                               asset net_sale_amount )
{
  require_auth( seller );

  check (dgood_ids.size() <= 20, "max batch size of 20");
  check( net_sale_amount.amount > .02, "minimum price of at least 0.02 EOS");
  check( net_sale_amount.symbol == symbol( symbol_code("EOS"), 4), "only accept EOS for sale" );

  dgoods_index dgood_table( get_self(), get_self().value );
  for ( auto const& dgood_id: dgood_ids ) {
    const auto& token = dgood_table.get( dgood_id, "token does not exist" );

    stats_index stats_table( get_self(), token.category.value );
    const auto& dgood_stats = stats_table.get( token.token_name.value, "dgood stats not found" );

    check( dgood_stats.sellable == true, "not sellable");
    check ( seller == token.owner, "not token owner");

    // make sure token not locked;
    lock_index lock_table( get_self(), get_self().value );
    auto locked_nft = lock_table.find( dgood_id );
    check(locked_nft == lock_table.end(), "token locked");

    // add token to lock table
    lock_table.emplace( seller, [&]( auto& l ) {
      l.dgood_id = dgood_id;
    });
  }

  asks_index ask_table( get_self(), get_self().value );
  // add batch to table of asks
  // set id to the first dgood being listed, if only one being listed, simplifies life
  ask_table.emplace( seller, [&]( auto& a ) {
    a.batch_id = dgood_ids[0];
    a.dgood_ids = dgood_ids;
    a.seller = seller;
    a.amount = net_sale_amount;
    a.expiration = time_point_sec(current_time_point()) + WEEK_SEC;
  });
}

ACTION ebonhaven::closesalenft( name seller,
                             uint64_t batch_id )
{
  asks_index ask_table( get_self(), get_self().value );
  const auto& ask = ask_table.get( batch_id, "cannot find sale to close" );

  lock_index lock_table( get_self(), get_self().value );
  // if sale has expired anyone can call this and ask removed, token sent back to orig seller
  if ( time_point_sec(current_time_point()) > ask.expiration ) {
    for ( auto const& dgood_id: ask.dgood_ids ) {
        const auto& locked_nft = lock_table.get( dgood_id, "dgood not found in lock table" );
        lock_table.erase( locked_nft );
    }
    ask_table.erase( ask );

  } else {
    require_auth( seller );
    check( ask.seller == seller, "only the seller can cancel a sale in progress");
    for ( auto const& dgood_id: ask.dgood_ids ) {
      const auto& locked_nft = lock_table.get( dgood_id, "dgood not found in lock table" );
      lock_table.erase( locked_nft );
    }
    ask_table.erase( ask );
  }
}

ACTION ebonhaven::buynft( name from,
                          name to,
                          asset quantity,
                          string memo )
{
    // allow EOS to be sent by sending with empty string memo
    if ( memo == "deposit" ) return;
    // don't allow spoofs
    if ( to != get_self() ) return;
    if ( from == name("eosio.stake") ) return;
    check( quantity.symbol == symbol( symbol_code("EOS"), 4), "Buy only with EOS" );
    check( memo.length() <= 32, "memo too long" );

    //memo format comma separated
    //batch_id,to_account
    uint64_t batch_id;
    name to_account;
    tie( batch_id, to_account ) = parsememo(memo);

    asks_index ask_table( get_self(), get_self().value );
    const auto& ask = ask_table.get( batch_id, "cannot find listing" );
    check( ask.amount.amount == quantity.amount, "send the correct amount" );
    check( ask.expiration > time_point_sec(current_time_point()), "sale has expired" );

    // nft(s) bought, change owner to buyer regardless of transferable
    _changeowner( ask.seller, to_account, ask.dgood_ids, "bought by: " + to_account.to_string(), false );

    // amounts owed to all parties
    map<name, asset> fee_map = _calcfees(ask.dgood_ids, ask.amount, ask.seller);
    for(auto const& fee : fee_map) {
        auto account = fee.first;
        auto amount = fee.second;

        // if seller is contract, no need to send EOS again
        if ( account != get_self() ) {
            // send EOS to account owed
            action( permission_level{ get_self(), name("active") },
                    name("eosio.token"), name("transfer"),
                    make_tuple( get_self(), account, amount, string("sale of dgood") ) ).send();
        }
    }

    // remove locks, remove from ask table
    lock_index lock_table( get_self(), get_self().value );

    for ( auto const& dgood_id: ask.dgood_ids ) {
        const auto& locked_nft = lock_table.get( dgood_id, "dgood not found in lock table" );
        lock_table.erase( locked_nft );
    }
    // remove sale listing
    ask_table.erase( ask );
}

// Private
map<name, asset> ebonhaven::_calcfees(vector<uint64_t> dgood_ids, asset ask_amount, name seller) {
  map<name, asset> fee_map;
  dgoods_index dgood_table( get_self(), get_self().value );
  int64_t tot_fees = 0;
  for ( auto const& dgood_id: dgood_ids ) {
    const auto& token = dgood_table.get( dgood_id, "token does not exist" );

    stats_index stats_table( get_self(), token.category.value );
    const auto& dgood_stats = stats_table.get( token.token_name.value, "dgood stats not found" );

    name rev_partner = dgood_stats.rev_partner;
    if ( dgood_stats.rev_split == 0.0 ) {
      continue;
    }

    double fee = static_cast<double>(ask_amount.amount) * dgood_stats.rev_split / static_cast<double>( dgood_ids.size() );
    asset fee_asset( fee, ask_amount.symbol );
    auto ret_val = fee_map.insert( {rev_partner, fee_asset} );
    tot_fees += fee_asset.amount;
    if ( ret_val.second == false ) {
      fee_map[rev_partner] += fee_asset;
    }
  }
  //add seller to fee_map minus fees
  asset seller_amount(ask_amount.amount - tot_fees, ask_amount.symbol);
  auto ret_val = fee_map.insert({seller, seller_amount});
  if ( ret_val.second == false ) {
    fee_map[seller] += seller_amount;
  }
  return fee_map;
}

void ebonhaven::_changeowner(name from, name to, vector<uint64_t> dgood_ids, string memo, bool istransfer) {
  check (dgood_ids.size() <= 20, "max batch size of 20");
  // loop through vector of dgood_ids, check token exists
  dgoods_index dgood_table( get_self(), get_self().value );
  lock_index lock_table( get_self(), get_self().value );
  for ( auto const& dgood_id: dgood_ids ) {
    const auto& token = dgood_table.get( dgood_id, "token does not exist" );

    stats_index stats_table( get_self(), token.category.value );
    const auto& dgood_stats = stats_table.get( token.token_name.value, "dgood stats not found" );

    if ( istransfer ) {
        check( token.owner == from, "must be token owner" );
        check( dgood_stats.transferable == true, "not transferable");
        auto locked_nft = lock_table.find( dgood_id );
        check( locked_nft == lock_table.end(), "token locked, cannot transfer");
    }

    // notifiy both parties
    require_recipient( from );
    require_recipient( to );
    dgood_table.modify( token, same_payer, [&] (auto& t ) {
      t.owner = to;
    });

    // amount 1, precision 0 for NFT
    asset quantity(1, dgood_stats.max_supply.symbol);
    _sub_balance(from, dgood_stats.category_name_id, quantity);
    _add_balance(to, get_self(), token.category, token.token_name, dgood_stats.category_name_id, quantity);
  }
}

void ebonhaven::_checkasset( asset amount, bool fungible ) {
  auto sym = amount.symbol;
  if (fungible) {
    check( amount.amount > 0, "amount must be positive" );
  } else {
    check( sym.precision() == 0, "NFT must be an int, precision of 0" );
    check( amount.amount >= 1, "NFT amount must be >= 1" );
  }

  globals_index config_table(get_self(), get_self().value);
  auto config_singleton = config_table.get();
  check( config_singleton.symbol.raw() == sym.code().raw(), "symbol must match symbol in config" );
  check( amount.is_valid(), "invalid amount" );
}

void ebonhaven::_mint(name to,
                      name issuer,
                      name category,
                      name token_name,
                      asset issued_supply,
                      string relative_uri,
                      dgood_attributes attributes)
{

  dgoods_index dgood_table( get_self(), get_self().value);
  auto dgood_id = dgood_table.available_primary_key();
  if (dgood_id == 0) { dgood_id++; }
  if ( relative_uri.empty() ) {
    dgood_table.emplace( issuer, [&]( auto& dg) {
      dg.id = dgood_id;
      dg.serial_number = issued_supply.amount + 1;
      dg.owner = to;
      dg.category = category;
      dg.token_name = token_name;
      dg.attributes = attributes;
    });
  } else {
    dgood_table.emplace( issuer, [&]( auto& dg ) {
      dg.id = dgood_id;
      dg.serial_number = issued_supply.amount + 1;
      dg.owner = to;
      dg.category = category;
      dg.token_name = token_name;
      dg.relative_uri = relative_uri;
      dg.attributes = attributes;
    });
  }
}

// Private
void ebonhaven::_add_balance( name owner, name ram_payer, name category, name token_name,
                                uint64_t category_name_id, asset quantity) {
  balances_index to_account( get_self(), owner.value );
  auto acct = to_account.find( category_name_id );
  if ( acct == to_account.end() ) {
    to_account.emplace( ram_payer, [&]( auto& a ) {
      a.category_name_id = category_name_id;
      a.category = category;
      a.token_name = token_name;
      a.amount = quantity;
    });
  } else {
    to_account.modify( acct, same_payer, [&]( auto& a ) {
      a.amount += quantity;
    });
  }
}

// Private
void ebonhaven::_sub_balance( name owner, uint64_t category_name_id, asset quantity ) {

  balances_index from_account( get_self(), owner.value );
  const auto& acct = from_account.get( category_name_id, "token does not exist in account" );
  check( acct.amount.amount >= quantity.amount, "quantity is more than account balance");

  if ( acct.amount.amount == quantity.amount ) {
    from_account.erase( acct );
  } else {
    from_account.modify( acct, same_payer, [&]( auto& a ) {
        a.amount -= quantity;
    });
  }
}