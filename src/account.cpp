#include <ebonhavencom.hpp>

ACTION ebonhavencom::newaccount( name user )
{
  require_auth( user );
  accounts_index accounts_table(get_self(), user.value);
  auto itr = accounts_table.find( user.value );
  check(itr == accounts_table.end(), "account already exists");
  accounts_table.emplace( user , [&](auto& ac) {
    ac.user = user;
  });
}