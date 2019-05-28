const util = require('util');

module.exports = {

inspect: function(obj) {
    console.log(util.inspect(obj, {showHidden: false, depth: null}));
},

getDefaultCharacter: function(character) {
    let data = {
        user: 'alice',
        character_id: character.character_id,
        character_name: character.character_name,
        gender: character.gender,
        profession: character.profession,
        race: character.race,
        level: character.level,
        experience: character.experience,
        new_position: [ 
            character.position.world,
            character.position.zone,
            character.position.x,
            character.position.y,
            character.position.orientation
        ],
        status: character.status,
        hp: character.hp,
        max_hp: character.max_hp,
        new_stats: [
            character.stats.stamina,
            character.stats.regen,
            character.stats.perception,
            character.stats.skill,
            character.stats.luck 
        ],
        new_attack: [
            character.attack.physical,
            character.attack.spell
        ],
        new_defense: [
            character.defense.physical,
            character.defense.spell
        ]
    };
    return data;
}

};