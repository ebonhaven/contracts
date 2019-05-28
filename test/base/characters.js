const assert = require('chai').assert;
const util = require('util');
const config = require('./../../config/test_config.json');
const EOS = require('../lib/eos');

describe('Characters', function() {
    let eos;
    let auth;
    let contractAuth;
    let account;
    let characters;
    let created;
    let item;
    let modData;

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
        let response = await eos.readTable('characters', 'alice');
        characters = response['rows'];
        response = await eos.readTable('accounts', 'alice');
        account = response['rows'][0];
    });

    it('can be created', async function() {  
        let data = {
            user: 'alice',
            character_name: 'Test Character',
            gender: 1,
            profession: 1,
            race: 1
        };
        await eos.action('newcharacter', auth, data);

        const response = await eos.readTable('characters', 'alice');

        const total = response['rows'].length;
        created = response['rows'][response['rows'].length - 1];

        assert.equal(characters.length + 1, total, `Character was not created`);
    });

    it('can be modified by contract account', async function() {
        modData = {
            user: 'alice',
            character_id: created.character_id,
            character_name: created.character_name,
            gender: created.gender,
            profession: created.profession,
            race: created.race,
            level: created.level,
            experience: created.experience,
            new_position: [ 
                created.position.world,
                created.position.zone,
                created.position.x,
                created.position.y,
                created.position.orientation
            ],
            status: created.status,
            hp: created.hp - 50,
            max_hp: created.max_hp,
            new_stats: [
               created.stats.stamina,
               created.stats.regen,
               created.stats.perception,
               created.stats.skill,
               created.stats.luck 
            ],
            new_attack: [
                created.attack.physical,
                created.attack.spell
            ],
            new_defense: [
                created.defense.physical,
                created.defense.spell
            ]
        };

        await eos.action('modcharacter', contractAuth, modData);
        const response = await eos.readTable('characters', 'alice');
        
        let prevHealth = created.hp;
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];

        assert.equal(created.hp, prevHealth - 50, `Character health was not modified`);

    });

    it('cannot be modified by users', async function() {
        try {
            await eos.action('modcharacter', auth, modData);
        } catch (error) {
            return;
        }
        assert.fail(`Characters modified by users`);
    });

    it('can use consumables', async function() {
        assert.equal(created.hp, created.max_hp - 50, `Character health not modified`);
        let data = {
            to: 'alice',
            item_ids: [202]
        };
        await eos.action('spawnitems', contractAuth, data);
        let response = await eos.readTable('items', 'alice');
        let item = response['rows'].filter((i) => {
            return i.parent_id == 202;
        })[0];
        assert.isOk(item, `Item not found`);

        data = {
            user: 'alice',
            character_id: created.character_id,
            item_id: item.item_id,
            effect_idx: 0
        };

        await eos.action('useitem', auth, data);
        response = await eos.readTable('items', 'alice');
        let delItem = response['rows'].filter((i) => {
            return i.item_id == item.item_id;
        });
        assert.equal(delItem.length, 0, `Item was not consumed`);
        response = await eos.readTable('characters', 'alice');
        let character = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.equal(character.hp, created.max_hp, `Character health was not updated`);
    });

    it('can equip items', async function() {
        let data = {
            to: 'alice',
            item_ids: [301]
        };
        await eos.action('spawnitems', contractAuth, data);
        let response = await eos.readTable('items', 'alice');
        item = response['rows'].filter((i) => {
            return i.parent_id == 301;
        })[0];
        assert.isOk(item, `Item not found`);

        data = {
            user: 'alice',
            character_id: created.character_id,
            item_id: item.item_id,
            equip_slot: 1
        };
        await eos.action('equipitem', auth, data);
        response = await eos.readTable('characters', 'alice');
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.notEqual(created.equipped.head, 0, `Item was not equipped`);

        response = await eos.readTable('accounts', 'alice');
        account = response['rows'][0];
        assert.isTrue(account.inventory.length == 0, `Item still in inventory`);
    });

    it('can unequip items', async function() {
        data = {
            user: 'alice',
            character_id: created.character_id,
            item_id: item.item_id,
            equip_slot: 0
        };
        await eos.action('equipitem', auth, data);
        let response = await eos.readTable('characters', 'alice');
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.equal(created.equipped.head, 0, `Item was not unequipped`);

        response = await eos.readTable('accounts', 'alice');
        account = response['rows'][0];
        assert.isTrue(account.inventory.length > 0, `Item did not return to inventory`);
    });

    it('can destroy items', async function() {
        data = {
            user: 'alice',
            item_id: item.item_id
        };
        await eos.action('destroyitem', auth, data);
        let response = await eos.readTable('accounts', 'alice');
        account = response['rows'][0];
        assert.equal(account.inventory.indexOf(item.item_id), -1, `Item still in inventory`);
        
        response = await eos.readTable('items', 'alice');
        let found = response['rows'].filter((i) => {
            return i.item_id == item.item_id;
        });
        assert.equal(found.length, 0, `Item was not destroyed`);

    });

    it('can buy abilities', async function() {
        let data = {
            user: 'alice',
            character_id: created.character_id,
            ability_id: 1001
        };
        await eos.action('buyability', auth, data);
        response = await eos.readTable('accounts', 'alice');
        let updatedBalance = response['rows'][0].balance;
        assert.isTrue(updatedBalance < account.balance, `Ability cost was not deducted`);

        response = await eos.readTable('history', 'alice');
        let history = response['rows'].filter((h) => {
            return h.character_id == created.character_id;
        })[0];
        assert.isTrue(history.abilities.length > 0, `Ability not added`);
    });

    it('can equip abilities', async function() {
        let data = {
            user: 'alice',
            character_id: created.character_id,
            ability_id: 1001,
            ability_idx: 1
        };
        await eos.action('equipability', auth, data);
        response = await eos.readTable('characters', 'alice');
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.isTrue(created.abilities.ability1 > 0, `Ability was not equipped`);
    });

    it('can unequip abilities', async function() {
        let data = {
            user: 'alice',
            character_id: created.character_id,
            ability_id: 1001,
            ability_idx: 0
        };
        await eos.action('equipability', auth, data);
        response = await eos.readTable('characters', 'alice');
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.isTrue(created.abilities.ability1 == 0, `Ability was not equipped`);
    });

    it('can be deleted', async function() {
        let data = {
            user: 'alice',
            character_id: created.character_id
        };

        await eos.action('delcharacter', auth, data);

        const response = await eos.readTable('characters', 'alice');
        const total = response['rows'].length;

        assert.equal(characters.length, total, `Character was not deleted`);
    });

});