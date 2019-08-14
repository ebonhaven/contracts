#include <ebonhaven.hpp>

ACTION ebonhaven::acceptquest( name user,
                               uint64_t character_id, 
                               uint64_t npc_id,
                               name quest_name )
{
    require_auth( user );
    characters_index characters(get_self(), user.value);
    charhistory_index charhistory(get_self(), user.value);
    auto history = charhistory.get(character_id, "cannot find character history");
    auto found = (find(history.completed_quests.begin(), history.completed_quests.end(), quest_name) != history.completed_quests.end());
    check(!found, "character completed this quest already");

    quests_index quests(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
    auto src_quest = quests.get(quest_name.value, "cannot find quest");
    auto character = characters.get(character_id, "cannot find character");
    check(character.hp > 0, "cannot accept quests while dead");
    check(character.level >= src_quest.min_level, "character level not high enough to accept quest");
    if (src_quest.profession_lock > 0) {
        check(character.profession == src_quest.profession_lock, "character profession cannot accept this quest");
    }
    if (src_quest.race_lock > 0) {
        check(character.race == src_quest.race_lock, "character race cannot accept this quest");
    }
    mapdata_index mapdata_table( get_self(), user.value );
    auto md = find_if(mapdata_table.begin(), mapdata_table.end(),
    [&character](const struct mapdata& el){ return el.world_zone_id == character.position.world_zone_id &&
                                            el.character_id == character.character_id; });
    check(md != mapdata_table.end(), "mapdata not found");
    auto npcd = find_if(md->npcs.begin(), md->npcs.end(), [&npc_id](const struct npcdata& el){ return el.npc_id == npc_id; });
    check(npcd != md->npcs.end(), "npc not found in mapdata");

    npcs_index npcs(ADMIN_CONTRACT, ADMIN_CONTRACT.value);
    auto npc = npcs.get(npcd->npc_id, "cannot find npc");                           
    auto quest_found = (find(npc.quests.begin(), npc.quests.end(), src_quest.quest_name) != npc.quests.end());
    check(quest_found != false, "npc does not offer that quest");

    auto character_pos = make_pair(character.position.x, character.position.y);
    auto npc_pos = make_pair(npcd->coordinates.x, npcd->coordinates.y);
    if (!is_position_within_target_radius(character_pos, npc_pos, npcd->radius)) {
        check(false, "character not within npc radius");
    }

    quests_index player_quests(get_self(), user.value);
    auto player_quest_found = find_if(player_quests.begin(), player_quests.end(),
    [&character, &src_quest](const struct quest& el){ return el.quest_name == src_quest.quest_name && el.character_id == character.character_id; });
    if (player_quest_found != player_quests.end()) {
        check(false, "quest currently active");
    }

    src_quest.character_id = character.character_id;
    src_quest.quest_id = player_quests.available_primary_key();
    if ( src_quest.quest_id == 0 ) { src_quest.quest_id++; }

    player_quests.emplace( user, [&](auto& q) {
        q = src_quest;
    });
}

ACTION ebonhaven::endquest( name user, uint64_t character_id, uint64_t npc_id, uint64_t quest_id )
{
    require_auth( user );

    quests_index quests(get_self(), user.value);
    auto q_index = quests.get_index<name("byquestid")>();
    auto itr = q_index.find(quest_id);
    characters_index characters(get_self(), user.value);
    auto character = characters.get(character_id, "cannot find character");
    check(character.hp > 0, "cannot end quests while dead");
    check(itr->character_id == character_id, "quest does not belong to this character");
    if ( npc_id == 0 ) {
        q_index.erase(itr);
    } else {
        mapdata_index u_mapdata(get_self(), user.value);
        auto md = find_if(u_mapdata.begin(), u_mapdata.end(),
            [&character](const struct mapdata& el){ return el.world_zone_id == character.position.world_zone_id &&
                                                    el.character_id == character.character_id; });
        check(md != u_mapdata.end(), "mapdata not found");
        auto npcd = find_if(md->npcs.begin(), md->npcs.end(), [&npc_id](const struct npcdata& el){ return el.npc_id == npc_id; });
        check(npcd != md->npcs.end(), "cannot find npc");

        auto quest = q_index.get(quest_id, "cannot find quest");
        check(quest.complete_npc_id == npcd->npc_id, "incorrect quest complete npc");
        vector<objective> completed_objectives;
        copy_if(quest.objectives.begin(), quest.objectives.end(), back_inserter(completed_objectives),
            [&](const struct objective& el){ return el.completed == 1; });

        check(completed_objectives.size() == quest.objectives.size(), "all objectives not completed");

        auto character_pos = make_pair(character.position.x, character.position.y);
        auto npc_pos = make_pair(npcd->coordinates.x, npcd->coordinates.y);
        if (!is_position_within_target_radius(character_pos, npc_pos, npcd->radius)) {
            check(false, "character not within npc radius");
        }

        generate_quest_reward( user, user, character.character_id, quest.rewards );

        if (quest.repeatable == 0) {
            charhistory_index history(get_self(), user.value);
            auto h_itr = history.find(character_id);
            history.modify( h_itr, user, [&](auto& h) {
                h.completed_quests.push_back(quest.quest_name);
            });
        }

        q_index.erase(itr);
    }
}

void ebonhaven::update_kill_quest_objectives( name user, uint64_t character_id, uint64_t mob_id ) {

    quests_index quests(get_self(), user.value);
    vector<quest> character_quests;
    copy_if(quests.begin(), quests.end(), back_inserter(character_quests),
        [&character_id](const struct quest& el){ return el.character_id == character_id; });
    
    if (character_quests.size() == 0) { return; }
    for (auto& q: character_quests) {
        bool updated = false;
        for (auto& o: q.objectives) {
            if (o.objective_type != 0) { break; }
            json j = json::parse(o.objective_data);
            if (j["mob_id"].get<uint64_t>() == mob_id ) {
                updated = true;
                uint8_t count = j["count"].get<uint8_t>();
                if (count > 0) {
                    j["count"] = count - 1;
                }
                print(j.dump());
                o.objective_data = j.dump();
                if (j["count"] == 0) {
                    o.completed = 1;
                }
            }
        }
        auto q_index = quests.get_index<name("byquestid")>();
        auto itr = q_index.find(q.quest_id);
        q_index.modify( itr, user, [&](auto& nq) {
            nq.objectives = q.objectives;
        });
    }
}

void ebonhaven::generate_quest_reward( name user, name payer, uint64_t character_id, vector<name> quest_items ) {
  rewards_index rewards(get_self(), user.value);

  reward rew = {};
  rew.reward_id = rewards.available_primary_key();
  if (rew.reward_id == 0) { rew.reward_id++; }
  rew.character_id = character_id;
  rew.items = quest_items;
  rewards.emplace( payer, [&](auto& r) {
    r = rew;
  });
}