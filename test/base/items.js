const assert = require('assert');
const util = require('util');
const config = require('./../../config/test_config.json');
const item_data = require('./../data/items.json');
const EOS = require('../lib/eos');

describe('Items', function() {
    let eos;
    let auth;
    let items;
    let created;

    before(async function() {
        eos = new EOS(config.endpoint, config.privateKeys);
        auth = [{
            actor: 'ebonhavencom',
            permission: 'active'
        }];
        const response = await eos.readTable('items', 'ebonhavencom');
        items = response['rows'];
    });

    it('can be spawned by contract account', async function() {  
        let data = item_data[0];
        data["user"] = "ebonhavencom";
        await eos.action('newitem', auth, data);

        const response = await eos.readTable('items', 'ebonhavencom');
        
        // console.log(util.inspect(response, {showHidden: false, depth: null}));

        const total = response['rows'].length;
        created = response['rows'][response['rows'].length - 1];

        assert.equal(items.length + 1, total, `Item was not created`);
    });

    it('can be deleted by contract account', async function() {
        let data = {
            user: 'ebonhavencom',
            item_id: created.item_id
        };

        await eos.action('delitem', auth, data);

        const response = await eos.readTable('items', 'ebonhavencom');
        const total = response['rows'].length;

        assert.equal(items.length, total, `Item was not deleted`);
    });

    

});