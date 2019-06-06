#include <ebonhaven.hpp>

//Admin
ACTION ebonhaven::create( name category,
                            name token_name,
                            bool fungible,
                            bool burnable,
                            bool transferable,
                            stat_attributes attributes,
                            string base_uri,
                            string max_supply )
{
  require_auth( get_self() );

  dasset m_supply;
  if ( fungible == true ) {
      m_supply.from_string(max_supply);
  } else {
      m_supply.from_string(max_supply, 0);
      check(m_supply.amount >= 1, "max_supply for nft must be at least 1");
  }

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

  stats_index stats_table( get_self(), category.value );
  auto existing_token = stats_table.find( token_name.value );
  check( existing_token == stats_table.end(), "Token with category and token_name exists" );
  // token type hasn't been created, create it
  stats_table.emplace( get_self(), [&]( auto& stats ) {
      stats.category_name_id = category_name_id;
      stats.issuer = name("ebonhavencom");
      stats.token_name = token_name;
      stats.fungible = fungible;
      stats.burnable = burnable;
      stats.transferable = transferable;
      stats.current_supply = 0;
      stats.issued_supply = 0;
      stats.attributes = attributes;
      stats.base_uri = base_uri;
      stats.max_supply = m_supply;
  });

  // successful creation of token, update category_name_id to reflect
  config_singleton.category_name_id++;
  config_table.set( config_singleton, get_self() );
}

ACTION ebonhaven::issue(name to,
                           name category,
                           name token_name,
                           string quantity,
                           string relative_uri,
                           string creator_name,
                           string memo)
{
  check( is_account( to ), "to account does not exist");
  check( memo.size() <= 256, "memo has more than 256 bytes" );

  // ebonhavendgdtats table
  stats_index stats_table( get_self(), category.value );
  const auto& dgood_stats = stats_table.get( token_name.value,
                                             "Token with category and token_name does not exist" );

  // ensure have issuer authorization and valid quantity
  require_auth( dgood_stats.issuer );

  dasset q;
  if (dgood_stats.fungible == false) {
    // mint nft
    q.from_string("1", 0);
    // check cannot issue more than max supply, careful of overflow of uint
    check( q.amount <= (dgood_stats.max_supply.amount - dgood_stats.current_supply), "Cannot issue more than max supply" );
    
    dgood_attributes attr;
    attr.bind_status = dgood_stats.attributes.item_binding;
    attr.item_status = 0;
    attr.durability = dgood_stats.attributes.max_durability;
    attr.creator_name = creator_name;
    attr.created_at = time_point_sec(current_time_point());
  
    mint(to, dgood_stats.issuer, category, token_name,
         dgood_stats.issued_supply, relative_uri, attr);
    add_balance(to, dgood_stats.issuer, category, token_name, dgood_stats.category_name_id, q);
  } else {
    // issue fungible
    q.from_string(quantity);
    // check cannot issue more than max supply, careful of overflow of uint
    check( q.amount <= (dgood_stats.max_supply.amount - dgood_stats.current_supply), "Cannot issue more than max supply" );
    string string_precision = "precision of quantity must be " + to_string(dgood_stats.max_supply.precision);
    check( q.precision == dgood_stats.max_supply.precision, string_precision.c_str() );
    add_balance(to, dgood_stats.issuer, category, token_name, dgood_stats.category_name_id, q);
  }

  // increase current supply
  stats_table.modify( dgood_stats, same_payer, [&]( auto& s ) {
      s.current_supply += q.amount;
      s.issued_supply += q.amount;
  });
}

ACTION ebonhaven::transfernft(name from,
                                 name to,
                                 vector<uint64_t> dgood_ids,
                                 string memo ) {
  // ensure authorized to send from account
  check( from != to, "cannot transfer to self" );
  require_auth( from );

  // ensure 'to' account exists
  check( is_account( to ), "to account does not exist");
  
  accounts_index accounts_table( get_self(), to.value );
  auto acct = accounts_table.get(to.value, "account not found");

  // check memo size
  check( memo.size() <= 256, "memo has more than 256 bytes" );

  // loop through vector of dgood_ids, check token exists
  dgoods_index dgood_table( get_self(), get_self().value );
  for ( auto const& dgood_id: dgood_ids ) {
    auto& token = dgood_table.get( dgood_id, "token does not exist" );
    check( token.owner == from, "must be token owner" );
    check( token.attributes.item_status != 1, "cannot transfer broken items");
    check( token.equipped != 1, "cannot transfer equipped items");
    check( token.attributes.bind_status != 2, "cannot transfer bound items");
    check( inventory_count(to) <= acct.max_inventory - 1, "not enough inventory space");

    stats_index stats_table( get_self(), token.category.value );
    const auto& dgood_stats = stats_table.get( token.token_name.value, "dgood stats not found" );
    
    check( dgood_stats.attributes.max_durability == token.attributes.durability, "cannot tranfer damaged items");
    check( dgood_stats.transferable == true, "not transferable");

    // notifiy both parties
    require_recipient( from );
    require_recipient( to );
    dgood_table.modify( token, same_payer, [&] (auto& t ) {
        t.owner = to;
    });

    // amount 1, precision 0 for NFT
    dasset quantity;
    quantity.amount = 1;
    quantity.precision = 0;
    sub_balance(from, dgood_stats.category_name_id, quantity);
    add_balance(to, from, token.category, token.token_name, dgood_stats.category_name_id, quantity);
  }
}

ACTION ebonhaven::burnnft(name owner, vector<uint64_t> dgood_ids)
{
  require_auth(owner);

  // loop through vector of dgood_ids, check token exists
  dgoods_index dgood_table( get_self(), get_self().value );
  for ( auto const& dgood_id: dgood_ids ) {
    const auto& token = dgood_table.get( dgood_id, "token does not exist" );
    check( token.owner == owner, "must be token owner" );

    stats_index stats_table( get_self(), token.category.value );
    const auto& dgood_stats = stats_table.get( token.token_name.value, "dgood stats not found" );

    check( dgood_stats.burnable == true, "not burnable");
    check( dgood_stats.fungible == false, "cannot call burnnft on fungible token, call burn instead");

    dasset quantity;
    quantity.from_string("1", 0);

    // decrease current supply
    stats_table.modify( dgood_stats, same_payer, [&]( auto& s ) {
        s.current_supply -= quantity.amount;
    });

    // lower balance from owner
    sub_balance(owner, dgood_stats.category_name_id, quantity);

    // erase token
    dgood_table.erase( token );
  }
}

// Private
void ebonhaven::mint(name to,
                        name issuer,
                        name category,
                        name token_name,
                        uint64_t issued_supply,
                        string relative_uri,
                        dgood_attributes attributes)
{

  dgoods_index dgood_table( get_self(), get_self().value);
  auto dgood_id = dgood_table.available_primary_key();
  if (dgood_id == 0) {
    dgood_id++;
  }
  if ( relative_uri.empty() ) {
    dgood_table.emplace( issuer, [&]( auto& dg) {
        dg.id = dgood_id;
        dg.serial_number = issued_supply + 1;
        dg.owner = to;
        dg.category = category;
        dg.token_name = token_name;
        dg.attributes = attributes;
    });
  } else {
    dgood_table.emplace( issuer, [&]( auto& dg ) {
        dg.id = dgood_id;
        dg.serial_number = issued_supply + 1;
        dg.owner = to;
        dg.category = category;
        dg.token_name = token_name;
        dg.relative_uri = relative_uri;
        dg.attributes = attributes;
    });
  }
}

// Private
void ebonhaven::add_balance( name owner, name ram_payer, name category, name token_name,
                                uint64_t category_name_id, dasset quantity) {
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
      a.amount.amount += quantity.amount;
    });
  }
}

// Private
void ebonhaven::sub_balance( name owner, uint64_t category_name_id, dasset quantity ) {

  balances_index from_account( get_self(), owner.value );
  const auto& acct = from_account.get( category_name_id, "token does not exist in account" );
  check( acct.amount.amount >= quantity.amount, "quantity is more than account balance");

  if ( acct.amount.amount == quantity.amount ) {
    from_account.erase( acct );
  } else {
    from_account.modify( acct, same_payer, [&]( auto& a ) {
        a.amount.amount -= quantity.amount;
    });
  }
}