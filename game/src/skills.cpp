#include <ebonhaven.hpp>

// TODO: Restrict crafting unless character is at valid crafting location
ACTION ebonhaven::craft( name user, uint64_t character_id, uint64_t recipe_id )
{
  require_auth( user );
  characters_index characters(get_self(), user.value);
  charhistory_index charhistory(get_self(), user.value);
  recipes_index recipes(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  dgoods_index dgoods(get_self(), get_self().value);
  lock_index lock_table( get_self(), get_self().value );
  auto character = characters.get(character_id, "couldn't find character");
  check(character.hp > 0, "cannot craft while dead");
  check(character.status == 0, "character status does not allow crafting");
  auto history = charhistory.get(character_id, "couldn't find history");
  bool found = (find(history.learned_recipes.begin(), history.learned_recipes.end(), recipe_id) != history.learned_recipes.end());
  check(found != false, "recipe hasn't been learned by character");
  auto recipe = recipes.get(recipe_id, "couldn't find recipe");

  vector<uint64_t> to_burn;

  for (auto& r: recipe.requirements) {
    vector<dgood> matching_items;
    auto owner_index = dgoods.get_index<name("byowner")>();
    copy_if(owner_index.lower_bound(user.value), owner_index.upper_bound(user.value), back_inserter(matching_items), [&user, &r, &lock_table](const struct dgood& el) {
      bool is_valid = false;
      if (el.owner == user && el.token_name == r.token_name && el.equipped == false) {
        auto locked_nft = lock_table.find( el.id );
        if( locked_nft == lock_table.end() ) {
          is_valid = true;
        }
      }
      return is_valid;
    });
    check(matching_items.size() >= r.quantity, "insufficient quantity of available items in inventory");
    for (int i = 0; i < r.quantity; i++) {
      to_burn.push_back(matching_items[i].id);
    }
  }

  action(
    permission_level{ user, name("active") },
    name("ebonhavencom"),
    name("burnnft"),
    make_tuple( user, to_burn )
  ).send();

  auto quantity = asset(1, symbol(symbol_code("EBON"), 0));
  action(
    permission_level{ get_self(), name("active") },
    name("ebonhavencom"),
    name("issue"),
    make_tuple( user, name("ebonhavencom"), recipe.token_name, quantity, string("1"), user.to_string(), string("issued by ebonhavencom"))
  ).send();

  if (history.profession_skill.craft <= recipe.max_skill) {
    auto h_itr = charhistory.find(character_id);
    charhistory.modify(h_itr, user, [&](auto& h) {
      h.profession_skill.craft++;
    });
  }
  
}

ACTION ebonhaven::gather( name user, uint64_t character_id )
{
  require_auth( user );
  characters_index characters(get_self(), user.value);
  charhistory_index charhistory(get_self(), user.value);
  mapdata_index mapdata_table(get_self(), user.value);
  auto character = characters.get(character_id, "couldn't find character");
  check(character.hp > 0, "cannot gather while dead");
  check(character.status == 2, "character status doesn't allow gathering");
  auto history = charhistory.get(character_id, "couldn't find history");

  auto md = find_if(mapdata_table.begin(), mapdata_table.end(),
    [&character](const struct mapdata& el){ return el.world_zone_id == character.position.world_zone_id &&
                                            el.character_id == character.character_id; });
  check(md != mapdata_table.end(), "cannot find mapdata for character");
  auto zd = find_if(md->resources.begin(), md->resources.end(),
    [&character](const struct zone_drop& el){ return el.profession_id == character.profession; });

  check(zd != md->resources.end(), "cannot find resources for profession in mapdata");
  resources_index resources(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
  auto resource = resources.get(zd->resource_name.value, "couldn't find resource");

  check(history.profession_skill.gather >= resource.min_skill, "gathering skill not high enough");

  auto roll = 100;
  auto num = random(roll);
  vector<name> reward_items;
  
  for (auto& drop: resource.drops) {
    auto r = rate_to_floor(drop.percentage, roll);
    uint8_t amt = drop.min_items;
    if (drop.max_items > drop.min_items) {
      auto range = drop.max_items - drop.min_items;
      amt = random(range) + drop.min_items;
    }
    if (num <= r) {
      for (int i = 0; i < amt; i++) {
        reward_items.push_back(drop.token_name);
      }
    }
  }

  uint32_t exp = resource.experience;
  if (history.profession_skill.gather > resource.max_skill) {
    exp = 0;
  }

  generate_resource_reward( user, user, character_id, exp, reward_items );

  if (character.status == 2) {
    auto c_itr = characters.find(character_id);
    characters.modify(c_itr, user, [&](auto& c) {
      c.status = 0;
    });

    if (history.profession_skill.gather <= resource.max_skill) {
      auto h_itr = charhistory.find(character_id);
      charhistory.modify(h_itr, user, [&](auto& h) {
        h.profession_skill.gather++;
      });
    }
  }
}

void ebonhaven::generate_resource_reward( name user, name payer, uint64_t character_id, uint32_t experience, vector<name> resource_items ) {
  rewards_index rewards(get_self(), user.value);

  reward rew = {};
  rew.reward_id = rewards.available_primary_key();
  rew.character_id = character_id;
  rew.experience = experience;
  rew.items = resource_items;
  rewards.emplace( payer, [&](auto& r) {
    r = rew;
  });
}