const assert = require('chai').assert;
const util = require('util');
const config = require('../../data/config.json');
const EOS = require('../lib/eos');

describe('Characters', function() {
    let eos, auth, contractAuth, account, characters, created, item, modData;

    before(async function() {
        eos = new EOS(config.endpoint, config.privateKeys, 'ebonhavencom');
        auth = [{
            actor: 'alice',
            permission: 'active'
        }];
        contractAuth = [{
            actor: 'ebonhavencom',
            permission: 'active'
        }];
        let response = await eos.readTable('characters', 'alice');
        let data = {
            to: 'alice',
            quantity: '1.00 EBON'
        };
        await eos.action('tokenreward', contractAuth, data);
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

    it('can use consumables', async function() {
        let data = {
            user: 'alice',
            character_id: created.character_id,
            health: created.max_hp - 50
        };
        await eos.action('modhp', contractAuth, data);
        let response = await eos.readTable('characters', 'alice');
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.equal(created.hp, data.health, `Character health not modified`);

        data = {
            to: 'alice',
            token_name: 'potion1'
        };
        await eos.action('spawnitem', contractAuth, data);
        response = await eos.readTable('dgood', 'ebonhavencom');
        let potion = response['rows'].filter((i) => {
            return i.token_name == 'potion1' && i.owner == 'alice';
        })[0];
        assert.isOk(potion, `Item not found`);

        data = {
            user: 'alice',
            character_id: created.character_id,
            dgood_id: potion.id,
            effect_idx: 0
        };

        await eos.action('useitem', auth, data);
        response = await eos.readTable('dgood', 'ebonhavencom');
        let table = response['rows'].filter((i) => {
            return i.id == potion.id && i.owner == 'alice';
        });
        assert.equal(table.length, 0, `Item was not consumed`);
        response = await eos.readTable('characters', 'alice');
        let character = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.equal(character.hp, created.max_hp, `Character health was not updated`);
    });

    it('can equip items', async function() {
        let data = {
            to: 'alice',
            token_name: 'weapon1'
        };
        await eos.action('spawnitem', contractAuth, data);
        let response = await eos.readTable('dgood', 'ebonhavencom');
        item = response['rows'].filter((i) => {
            return i.token_name == 'weapon1' && i.owner == 'alice';
        })[0];
        assert.isOk(item, `Item not found`);

        data = {
            user: 'alice',
            character_id: created.character_id,
            dgood_id: item.id,
            equip_slot: 11
        };
        await eos.action('equipitem', auth, data);
        response = await eos.readTable('characters', 'alice');
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.notEqual(created.equipped.weapon, 0, `Character was not updated with equipped item`);

        response = await eos.readTable('dgood', 'ebonhavencom');
        item = response['rows'].filter((i) => {
            return i.token_name == 'weapon1' && i.owner == 'alice';
        })[0];
        assert.isTrue(item.equipped == 1, `Item not equipped properly`);
    });

    it('can unequip items', async function() {
        let data = {
            user: 'alice',
            character_id: created.character_id,
            dgood_id: item.id,
            equip_slot: 0
        };
        await eos.action('equipitem', auth, data);
        let response = await eos.readTable('characters', 'alice');
        created = response['rows'].filter((c) => {
            return c.character_id == created.character_id;
        })[0];
        assert.equal(created.equipped.weapon, 0, `Item was not unequipped`);

        response = await eos.readTable('dgood', 'ebonhavencom');
        item = response['rows'].filter((i) => {
            return i.token_name == 'weapon1' && i.owner == 'alice';
        })[0];
        assert.isTrue(item.equipped == 0, `Item not equipped properly`);
    });

    it('can burn items', async function() {
        data = {
            owner: 'alice',
            dgood_ids: [item.id]
        };
        await eos.action('burnnft', auth, data);
        
        response = await eos.readTable('dgood', 'ebonhavencom');
        let found = response['rows'].filter((i) => {
            return i.id == item.id;
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

        response = await eos.readTable('charhistory', 'alice');
        let history = response['rows'].filter((h) => {
            return h.character_id == created.character_id;
        })[0];
        assert.isTrue(history.learned_abilities.length > 0, `Ability not added`);
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