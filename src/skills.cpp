#include <ebonhaven.hpp>

// TODO: Restrict crafting unless character is at valid crafting location
ACTION ebonhaven::craft( name user, uint64_t character_id, uint64_t recipe_id )
{
  require_auth( user );
  charhistory_index charhistory(get_self(), user.value);
  recipes_index recipes(get_self(), get_self().value);
  dgoods_index dgoods(get_self(), get_self().value);
  auto history = charhistory.get(character_id, "couldn't find history");
  bool found = (find(history.learned_recipes.begin(), history.learned_recipes.end(), recipe_id) != history.learned_recipes.end());
  check(found != false, "recipe hasn't been learned by character");
  auto recipe = recipes.get(recipe_id, "couldn't find recipe");

  vector<uint64_t> to_burn;

  // FIXME: This is no bueno
  for (auto& r: recipe.requirements) {
    vector<dgood> matching_items;
    copy_if(dgoods.begin(), dgoods.end(), back_inserter(matching_items), [&user, &r](const struct dgood& el) {
      return el.owner == user && el.token_name == r.token_name && el.equipped == false;
    });
    check(matching_items.size() >= r.quantity, "insufficient quantity of items in inventory");
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

  action(
    permission_level{ get_self(), name("active") },
    name("ebonhavencom"),
    name("issue"),
    make_tuple( user, name("ebonhavencom"), recipe.token_name, string("1"), string("1"), user.to_string(), string("issued by ebonhavencom"))
  ).send();
}

ACTION ebonhaven::gather( name user, uint64_t character_id )
{
  require_auth( user );
  characters_index characters(get_self(), user.value);
  charhistory_index charhistory(get_self(), user.value);
  auto character = characters.get(character_id, "couldn't find character");
  check(character.status == 2, "character status doesn't allow gathering");
  auto history = charhistory.get(character_id, "couldn't find history");
  resources_index resources(get_self(), character.profession);
  auto resource = resources.get(character.position.world_zone_id, "couldn't find resource");

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

  generate_resource_reward( user, user, character_id, reward_items );

  if (character.status == 2) {
    auto c_itr = characters.find(character_id);
    characters.modify(c_itr, user, [&](auto& c) {
      c.status = 0;
    });

    auto h_itr = charhistory.find(character_id);
    charhistory.modify(h_itr, user, [&](auto& h) {
      h.profession_skill.gather++;
    });
  }
}

void ebonhaven::generate_resource_reward( name user, name payer, uint64_t character_id, vector<name> resource_items ) {
  rewards_index rewards(get_self(), user.value);
  stats_index stats_table(get_self(), get_self().value);

  reward rew = {};
  rew.reward_id = rewards.available_primary_key();
  if (rew.reward_id == 0) { rew.reward_id++; }
  rew.character_id = character_id;
  rew.items = resource_items;
  rewards.emplace( payer, [&](auto& r) {
    r = rew;
  });
}