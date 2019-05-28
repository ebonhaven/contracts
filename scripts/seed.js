const EOS = require('../test/lib/eos');
const config = require('./config.json');
const stats = require('../data/basestats.json');
const effects = require('../data/effects.json').effects;
const auras = require('../data/auras.json').auras;
const abilities = require('../data/abilities.json').abilities;
const items = require('../data/items.json').items;
const drops = require('../data/drops.json').drops;
const argv = require('yargs').argv

class Seeder {

    constructor() {
        this.eos = new EOS(config.endpoint, config.privateKeys);
    }

    set eos(value) {
        this.constructor.eos = value;
    }

    get eos() {
        return this.constructor.eos;
    }

    createAccounts() {
        let auth = [{
            actor: 'eosio',
            permission: 'active'
        }]; 
        config.accounts.forEach((a) => {
            this.eos.createUser(a.name, auth, a.permissions);
        });
    }

    setup() {
        console.log("Setting up...");
        let auth = [{
            actor: 'ebonhavencom',
            permission: 'active'
        }];
        let data = {
            version: '1.0.0'
        };
        this.eos.action("ebonhavencom", "setconfig", auth, data);
    }

    setupStats() {
        console.log("Seeding basestats...");
        let actions = [];

        stats.basestats.forEach((s) => {
            let action = {
                account: "ebonhavencom",
                name: "upsstats",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: s.stats
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedEffects() {
        console.log("Seeding effects...");
        let actions = [];

        effects.forEach((e) => {
            let action = {
                account: "ebonhavencom",
                name: "upseffect",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: e
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedAuras() {
        console.log("Seeding auras...");
        let actions = [];

        auras.forEach((a) => {
            let action = {
                account: "ebonhavencom",
                name: "upsaura",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: a
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedAbilities() {
        console.log("Seeding abilities...");
        let actions = [];

        abilities.forEach((a) => {
            let action = {
                account: "ebonhavencom",
                name: "upsability",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: a
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    retireAllItems() {
        console.log("Retiring items...");
        items.forEach((i) => {
            let auth = [{
                actor: "ebonhavencom",
                permission: "active"
            }];
            let data = {
                category: i.category,
                token_name: i.token_name
            };
            this.eos.action("ebonhavencom", "retire", auth, data);
        });
    }

    seedItems() {
        console.log("Seeding items...");
        let actions = [];

        items.forEach((i) => {
            let action = {
                account: "ebonhavencom",
                name: "create",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: i
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedCharacters() {
        console.log("Seeding characters...");
        let actions = [];

        config.accounts.forEach((a) => {
            let action = {
                account: "ebonhavencom",
                name: "newcharacter",
                authorization: [{
                    actor: a.name,
                    permission: "active"
                }],
                data: {}
            };
            if (a.name == "ebonhavencom") {
                action.name = "newaccount";
                action.data = {
                    user: a.name
                };
            } else {
                action.data = {
                    user: a.name,
                    character_name: a.name.charAt(0).toUpperCase() + a.name.slice(1),
                    gender: a.character.gender,
                    profession: a.character.profession,
                    race: a.character.race
                };
            }
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedDrops() {
        console.log("Seeding drops...");
        let actions = [];
        drops.forEach((d) => {
            let action = {
                account: "ebonhavencom",
                name: "upsdrop",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: d
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }
};

const seeder = new Seeder;
switch(argv._[0]) {
    case "accounts":
        console.log("Creating accounts...");
        seeder.createAccounts();
        break;
    case "setup":
        seeder.setup();
        break;
    case "basestats":
        seeder.setupStats();
        break;
    case "characters":
        seeder.seedCharacters();
        break;
    case "effects":
        seeder.seedEffects();
        break;
    case "auras":
        seeder.seedAuras();
        break;
    case "abilities":  
        seeder.seedAbilities();
        break;
    case "items":
        seeder.seedItems();
        break;
    case "drops":
        seeder.seedDrops();
        break;
    case "retireall":
        seeder.retireAllItems();
        break;
    default:
        seeder.setup();
        seeder.setupStats();
        seeder.seedEffects();
        seeder.seedAuras();
        seeder.seedAbilities();
        seeder.seedItems();
        seeder.seedCharacters();
        seeder.seedDrops();
        break;
}
