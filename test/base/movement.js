const assert = require('chai').assert;
const util = require('util');
const config = require('../../data/config.json');
const EOS = require('../lib/eos');
const helpers = require('../lib/helpers');

let outcomes = {
    DEFAULT: 0,
    DISCOVERY: 1,
    RESOURCE: 2,
    TRAP: 3,
    COMBAT: 4,
    TREASURE: 5,
    LOOT: 6,
    NONE: 7
};

function getPositionArray(character) {
    return [ character.position.world_zone_id, character.position.x, character.position.y, character.position.orientation ];
}

function normalizeFloat(x) {
    return Number.parseFloat(x).toFixed(3);
}

describe('Movement', function() {
    this.timeout(3000);

    let auth;
    let contractAuth;
    let character;
    let response;
    let rates;
    let rateData;

    async function setOutcome(outcome) {
        let data = {
            combat_rate: 0.001,
            resource_rate: 0.001,
            discovery_rate: 0.001,
            trap_rate: 0.001,
            treasure_rate: 0.001,
            loot_rate: 0.001
        }
        switch (outcome) {
            case outcomes.DISCOVERY:
                data.discovery_rate = 1.0;
                break;
            case outcomes.RESOURCE:
                data.resource_rate = 1.0;
                break;
            case outcomes.TRAP:
                data.trap_rate = 1.0;
                break;
            case outcomes.COMBAT:
                data.combat_rate = 1.0;
                break;
            case outcomes.TREASURE:
                data.treasure_rate = 1.0;
                break;
            case outcomes.LOOT:
                data.loot_rate = 1.0;
                break;
            case outcomes.DEFAULT:
                data = {
                    combat_rate: 0.15,
                    resource_rate: 0.09,
                    discovery_rate: 0.025,
                    trap_rate: 0.017,
                    treasure_rate: 0.005,
                    loot_rate: 0.0015
                };
                break;
            case outcomes.NONE:
                break;
        }
        rateData = data;
        await eos.action('upsrates', contractAuth, rateData);
        return;
    }

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
            character_name: 'Test Movement',
            gender: 1,
            profession: 1,
            race: 1
        };

        await eos.action('newcharacter', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'][response['rows'].length - 1];
    });

    it('encounter rates can be modified by contract owner', async function() {
        await setOutcome(outcomes.DEFAULT)
        response = await eos.readTable('globals', 'ebonhavencom');
        rates = response['rows'][0]['rates'];
        assert.equal(normalizeFloat(rates.combat), normalizeFloat(rateData.combat_rate), `Combat rate not updated`);
        assert.equal(normalizeFloat(rates.resource), normalizeFloat(rateData.resource_rate), `Resource rate not updated`);
        assert.equal(normalizeFloat(rates.discovery), normalizeFloat(rateData.discovery_rate), `Discovery rate not updated`);
        assert.equal(normalizeFloat(rates.trap), normalizeFloat(rateData.trap_rate), `Trap rate not updated`);
        assert.equal(normalizeFloat(rates.treasure), normalizeFloat(rateData.treasure_rate), `Treasure rate not updated`);
        assert.equal(normalizeFloat(rates.loot), normalizeFloat(rateData.loot_rate), `Loot rate not updated`);
    });

    it('encounter rates cannot be modified by users', async function() {
        try {
            await eos.action('upsrates', auth, rateData);
        } catch (error) {
            return;
        }
        assert.fail(`Rates modified by user`);
    });

    it('can update character position', async function() {
        await setOutcome(outcomes.NONE);

        let data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 1,
                y: 0,
                orientation: 0
            }
        };
        await eos.action('move', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id = character.character_id;
        })[0];
        assert.deepEqual(character.position, data.new_position, `Position was not updated`);
    });

    it('cannot move to unwalkable position', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 0,
                y: 1,
                orientation: 0
            }
        };
        try {
            await eos.action('move', auth, data);
        } catch (error) {
            return;
        }
        assert.fail(`Moved to unwalkable position`);
    });

    it('cannot move to current position', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 1,
                y: 0,
                orientation: 0
            }
        };
        try {
            await eos.action('move', auth, data);
        } catch (error) {
            return;
        }
        assert.fail(`Moved to current position`);
    });

    it('cannot move if in combat', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            status: 4
        };
        await eos.action('modstatus', contractAuth, data);
        data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 0,
                y: 0,
                orientation: 0
            }
        };
        try {
            await eos.action('move', auth, data);
        } catch (error) {
            return;
        }
        assert.fail(`Moved while in combat`);
    });

    it('can roll a discovery encounter', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            status: 0
        };
        await eos.action('modstatus', contractAuth, data);
        await setOutcome(outcomes.DISCOVERY);

        data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 0,
                y: 0,
                orientation: 0
            }
        };
        await eos.action('move', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id = character.character_id;
        })[0];
        assert.equal(character.status, 1, `Character status is not discovery`);
    });

    it('can roll a resource encounter', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            status: 0
        };
        await eos.action('modstatus', contractAuth, data);
        await setOutcome(outcomes.RESOURCE);

        data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 1,
                y: 0,
                orientation: 0
            }
        };
        await eos.action('move', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id = character.character_id;
        })[0];
        assert.equal(character.status, 2, `Character status is not resource`);
    });

    it('can roll a trap encounter', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            status: 0
        };
        await eos.action('modstatus', contractAuth, data);
        await setOutcome(outcomes.TRAP);

        data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 0,
                y: 0,
                orientation: 0
            }
        };
        await eos.action('move', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id = character.character_id;
        })[0];
        assert.equal(character.status, 3, `Character status is not resource`);
    });

    it('can roll a combat encounter', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            status: 0
        };
        await eos.action('modstatus', contractAuth, data);
        await setOutcome(outcomes.COMBAT);
        data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 1,
                y: 0,
                orientation: 0
            }
        };
        await eos.action('move', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id = character.character_id;
        })[0];
        assert.equal(character.status, 4, `Character status is not combat`);

        response = await eos.readTable('encounters', 'alice');
        assert.isTrue(response['rows'].length > 0, `Encounter not found`);
    });

    it('can roll a treasure encounter', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            status: 0
        };
        await eos.action('modstatus', contractAuth, data);
        await setOutcome(outcomes.TREASURE);

        data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 0,
                y: 0,
                orientation: 0
            }
        };
        await eos.action('move', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id = character.character_id;
        })[0];
        assert.equal(character.status, 5, `Character status is not treasure`);
    });

    it('can roll a loot encounter', async function() {
        let data = {
            user: 'alice',
            character_id: character.character_id,
            status: 0
        };
        await eos.action('modstatus', contractAuth, data);
        await setOutcome(outcomes.LOOT);

        data = {
            user: 'alice',
            character_id: character.character_id,
            new_position: {
                world_zone_id: 1,
                x: 1,
                y: 0,
                orientation: 0
            }
        };
        await eos.action('move', auth, data);
        response = await eos.readTable('characters', 'alice');
        character = response['rows'].filter((c) => {
            return c.character_id = character.character_id;
        })[0];
        assert.equal(character.status, 6, `Character status is not loot`);
    });

    after(async function() {
        setOutcome(outcomes.DEFAULT);

        let data = {
            user: 'alice',
            character_id: character.character_id
        }
        await eos.action('delcharacter', auth, data);
    });
});