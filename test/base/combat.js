const assert = require('chai').assert;
const config = require('./../../config/test_config.json');
const EOS = require('../lib/eos');
const helpers = require('../lib/helpers');

describe('Combat', function() {
    this.timeout(5000);

    let eos;
    let auth;
    let contractAuth;
    let character;
    let prevCharacter;
    let encounter;
    let prevEncounter;
    let weapon;
    let ranged;

    before(async function() {
        eos = new EOS(config.endpoint, config.privateKeys);
        auth = [{
            actor: 'alice',
            permission: 'active'
        }];
        contractAuth = [{
            actor: 'ebonhavencom',
            permission: 'active'
        }];
        let data = {
            user: 'alice',
            character_name: 'Test Combat',
            gender: 1,
            profession: 1,
            race: 1
        };
        await eos.action('newcharacter', auth, data);
        let response = await eos.readTable('characters', 'alice');
        character = response['rows'][0];
        
        data = {
            to: 'alice',
            item_ids: [102, 103]
        };
        await eos.action('spawnitems', contractAuth, data);
        response = await eos.readTable('items', 'alice');
        weapon = response['rows'].filter((i) => {
            return i.parent_id == 102;
        })[0];
        ranged = response['rows'].filter((i) => {
            return i.parent_id == 103;
        })[0];
    });

    it('players can melee attack', async function() {
        let char = helpers.getDefaultCharacter(character);
        char.status = 0;
        await eos.action('modcharacter', contractAuth, char);

        data = {
            user: 'alice',
            character_id: character.character_id,
            item_id: weapon.item_id,
            equip_slot: 10
        };
        
        await eos.action('equipitem', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id == character.character_id;
        })[0];
        assert.isTrue(character.equipped.weapon > 0, "Melee weapon not equipped");

        char = helpers.getDefaultCharacter(character);
        char.status = 4;
        await eos.action('modcharacter', contractAuth, char);

        data = {
            user: 'alice',
            encounter_id: encounter.encounter_id,
            combat_decision: 1,
            mob_idx: 0
        }
        await eos.action('combat', auth, data);
        response = await eos.readTable('encounters', 'alice');
        
        prevEncounter = encounter;
        encounter = response['rows'][0];
        assert.isTrue(encounter.turn_counter > prevEncounter.turn_counter, `Turn counter not incremented`);
        assert.equal(encounter.last_decision, data.combat_decision, `Decision not stored`);
        if (encounter.last_decision_hit == 1) {
            assert.isTrue(encounter.mobs[data.mob_idx].hp < prevEncounter.mobs[data.mob_idx].hp, `Player hit but no damage to mob`);
        }
        response = await eos.readTable('characters', 'alice');
        prevCharacter = character;
        character = response['rows'].filter((c) => {
            return c.character_id == encounter.character_id;
        })[0];
        
        let hit = encounter.mobs.filter((m) => {
            return m.last_decision_hit == 1;
        });
        if (hit.length > 0) {
            assert.isTrue(character.hp < prevCharacter.hp, `Mobs hit but no damage to player`);
        }
    });

    it('players cannot ranged attack without ranged weapon equipped', async function() {
        let data = {
            user: 'alice',
            encounter_id: encounter.encounter_id,
            combat_decision: 2,
            mob_idx: 0
        }
        try {
            await eos.action('combat', auth, data);
        } catch (error) {
            return;
        }
        assert.fail(`Players can ranged attack without weapon`);
    });

    it('players can ranged attack', async function() {
        let char = helpers.getDefaultCharacter(character);
        char.status = 0;
        await eos.action('modcharacter', contractAuth, char);

        data = {
            user: 'alice',
            character_id: character.character_id,
            item_id: ranged.item_id,
            equip_slot: 11
        };
        await eos.action('equipitem', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id == character.character_id;
        })[0];
        assert.isTrue(character.equipped.ranged > 0, "Ranged weapon not equipped");

        char = helpers.getDefaultCharacter(character);
        char.status = 4;
        await eos.action('modcharacter', contractAuth, char);

        data = {
            user: 'alice',
            encounter_id: encounter.encounter_id,
            combat_decision: 2,
            mob_idx: 0
        }
        await eos.action('combat', auth, data);
        response = await eos.readTable('encounters', 'alice');
        prevEncounter = encounter;
        encounter = response['rows'][0];
        assert.isTrue(encounter.turn_counter > prevEncounter.turn_counter, `Turn counter not incremented`);
        assert.equal(encounter.last_decision, data.combat_decision, `Decision not stored`);
        if (encounter.last_decision_hit == 1) {
            assert.isTrue(encounter.mobs[data.mob_idx].hp < prevEncounter.mobs[data.mob_idx].hp, `Player hit but no damage to mob`);
        }
    });

    it('players cannot use an ability without equipping', async function() {
        let data = {
            user: 'alice',
            encounter_id: encounter.encounter_id,
            combat_decision: 3,
            mob_idx: 0
        }
        try {
            await eos.action('combat', auth, data);
        } catch (error) {
            return;
        }
        assert.fail(`Ability used without equipping`);
        
    });

    it('players cannot equip ability in combat', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            ability_id: 1001
        };
        await eos.action('buyability', auth, data);

        data = {
            user: 'alice',
            character_id: character.character_id,
            ability_id: 1001,
            ability_idx: 1
        };
        
        try {
            await eos.action('equipability', auth, data);
        } catch (error) {
            return;
        }
        assert.fail(`Ability equipped during combat`);
        
    });

    it('players can use ability', async function() {
        let char = helpers.getDefaultCharacter(character);
        char.status = 0;
        await eos.action('modcharacter', contractAuth, char);

        response = await eos.readTable('encounters', 'alice');
        prevEncounter = response['rows'][0];

        let data = {
            user: 'alice',
            character_id: character.character_id,
            ability_id: 1001,
            ability_idx: 1
        };
        await eos.action('equipability', auth, data);

        char = helpers.getDefaultCharacter(character);
        char.status = 4;
        await eos.action('modcharacter', contractAuth, char);

        data = {
            user: 'alice',
            encounter_id: encounter.encounter_id,
            combat_decision: 3,
            mob_idx: 0
        }
        await eos.action('combat', auth, data);
        response = await eos.readTable('encounters', 'alice');
        encounter = response['rows'][0];

        assert.isTrue(encounter.turn_counter > prevEncounter.turn_counter, `Turn counter not incremented`);
        assert.equal(encounter.last_decision, data.combat_decision, `Decision not stored`);
    });

    after(async function() {

        let data = {
            user: 'alice',
            character_id: character.character_id
        }
        await eos.action('delcharacter', auth, data);
    });

});