const assert = require('chai').assert;
const util = require('util');
const config = require('../../data/config.json');
const EOS = require('../lib/eos');

describe('Skills', function() {
    let eos;
    let character;
    let auth;
    let contractAuth;
    let items;
    let created;

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
        let data = {
            user: 'alice',
            character_name: 'Alice',
            gender: 1,
            profession: 1,
            race: 1
        };

        await eos.action('newcharacter', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'][response['rows'].length - 1];
    });

    it('recipe items can be learned', async function() {  
        let data = {
            to: 'alice',
            token_name: 'recipe1'
        };
        await eos.action('spawnitem', contractAuth, data);
        let response = await eos.readTable('dgood', 'ebonhavencom');
        let recipe = response['rows'].filter((i) => {
            return i.token_name == 'recipe1' && i.owner == 'alice';
        })[0];
        assert.isOk(recipe, `Item not found`);

        data = {
            user: 'alice',
            character_id: character.character_id,
            dgood_id: recipe.id,
            effect_idx: 0
        };

        await eos.action('useitem', auth, data);
        response = await eos.readTable('dgood', 'ebonhavencom');

        let found = response['rows'].filter((i) => {
            return i.id == recipe.id && i.owner == 'alice';
        });
        assert.isTrue(found.length == 0, `Recipe was not consumed`);

        response = await eos.readTable('charhistory', 'alice');
        let history = response['rows'].filter((h) => {
            return h.character_id == character.character_id;
        })[0];
        assert.isTrue(history.learned_recipes.length > 0, `Recipe not learned`);
    });

    after(async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id
        }
        await eos.action('delcharacter', auth, data);
    });

});