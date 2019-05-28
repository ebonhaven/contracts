const assert = require('chai').assert;
const util = require('util');
const config = require('./../../config/test_config.json');
const EOS = require('../lib/eos');

describe('Array', function() {

    before(async function() {
        eos = new EOS(config.endpoint, config.privateKeys);
        auth = [{
            actor: 'alice',
            permission: 'active'
        }];
    });

    it('can be submitted empty', async function() {
        let data = {
            user: 'alice',
            array: []
        };
        await eos.action('emptyarray', auth, data);
    });

});