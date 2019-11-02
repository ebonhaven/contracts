#include <ebonhaven.hpp>

ACTION ebonhaven::newaccount( name user )
{
  require_auth( user );
  accounts_index accounts_table(get_self(), user.value);
  auto itr = accounts_table.find( user.value );
  check(itr == accounts_table.end(), "account already exists");
  accounts_table.emplace( user , [&](auto& ac) {
    ac.user = user;
  });
}

ACTION ebonhaven::newcontact( name user, name contact )
{
  require_auth( user );
  accounts_index accounts(get_self(), contact.value);
  contacts_index contacts(get_self(), user.value);
  auto a_itr = accounts.require_find(contact.value, "cannot find account");

  // Default permissions
  vector<permission> permissions;
  // transfer: true
  permission perm1 {
    as_integer(permission_type::TRANSFER), true
  };
  permissions.push_back(perm1);

  contacts.emplace( user, [&](auto& c) {
    c.contact = contact;
    c.permissions = permissions;
  });
}

ACTION ebonhaven::delcontact( name user, name contact )
{
  require_auth( user );
  accounts_index accounts(get_self(), contact.value);
  contacts_index contacts(get_self(), user.value);
  auto a_itr = accounts.require_find(contact.value, "cannot find account");
  auto c_itr = contacts.require_find(contact.value, "cannot find contact");
  contacts.erase(c_itr);
}