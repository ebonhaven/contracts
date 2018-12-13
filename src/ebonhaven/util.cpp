#include "ebonhaven.hpp"

using namespace eosio;
using namespace std;

// Simple Pseudo Random Number Algorithm, randomly pick a number within 0 to n-1
int ebonhaven::random(const int range) {
  // Find the existing seed
  auto seed_itr = _config.begin();

  // Initialize the seed with default value if it is not found
  if (seed_itr == _config.end()) {
    seed_itr = _config.emplace( _self, [&]( auto& globals ) { });
  }

  // Generate new seed value using the existing seed value
  int prime = 65537;
  auto new_seed_value = (seed_itr->seed + now()) % prime;

  // Store the updated seed value in the table
  _config.modify( seed_itr, _self, [&]( auto& s ) {
    s.seed = new_seed_value;
  });

  // Get the random result in desired range
  int random_result = new_seed_value % range;
  return random_result;
}

uint64_t ebonhaven::create_hash( const string& str ) {
  // Hash the string to a checksum
  hash<string> hasher;
  uint64_t result = hasher(str);
  // Return resulting digest
  return result;
};

template <typename Enumeration>
auto as_integer(Enumeration const value)
    -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}