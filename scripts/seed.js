const EOS = require('../test/lib/eos');
const config = require('../data/config.json');
const stats = require('../data/basestats.json');
const effects = require('../data/effects.json').effects;
const auras = require('../data/auras.json').auras;
const abilities = require('../data/abilities.json').abilities;
const items = require('../data/items.json').items;
const drops = require('../data/drops.json').drops;
const mobs = require('../data/mobs.json').mobs;
const resources = require('../data/resources.json').resources;
const treasures = require('../data/treasures.json').treasures;
const mapdata = require('../data/mapdata.json').mapdata;
const recipes = require('../data/recipes.json').recipes;
const quests = require('../data/quests.json').quests;
const npcs = require('../data/npcs.json').npcs;
const progress = require('../data/progress.json').progress;
const argv = require('yargs').argv

class Seeder {

    constructor() {
        this.eos = new EOS(config.endpoint, config.privateKeys, 'ebonhavencom');
    }

    set eos(value) {
        this.constructor.eos = value;
    }

    get eos() {
        return this.constructor.eos;
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
        this.eos.action("setconfig", auth, data);
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

    createAdminAccount() {
        let auth = [{
            actor: 'ebonhavencom',
            permission: 'active'
        }];
        this.eos.action('newaccount', auth, { user: "ebonhavencom" });
    }

    setupStats() {
        console.log("Seeding basestats...");
        let actions = [];

        stats.basestats.forEach((s) => {
            let action = {
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
            this.eos.action("retire", auth, data);
        });
    }

    seedItems() {
        console.log("Seeding items...");
        let actions = [];

        items.forEach((i) => {
            let action = {
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

    seedMobs() {
        console.log("Seeding mobs...");
        let actions = [];
        mobs.forEach((m) => {
            let action = {
                name: "upsmob",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: m
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedResources() {
        console.log("Seeding resources...");
        let actions = [];
        resources.forEach((r) => {
            let action = {
                name: "upsresource",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: r
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedTreasures() {
        console.log("Seeding treasures...");
        let actions = [];
        treasures.forEach((t) => {
            let action = {
                name: "upstreasure",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: t
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedMapdata() {
        console.log("Seeding mapdata...");
        let actions = [];
        mapdata.forEach((m) => {
            let action = {
                name: "upsmapdata",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: m
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedRecipes() {
        console.log("Seeding recipes...");
        let actions = [];
        recipes.forEach((r) => {
            let action = {
                name: "upsrecipe",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: r
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedQuests() {
        console.log("Seeding quests...");
        let actions = [];
        quests.forEach((q) => {
            let action = {
                name: "upsquest",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: q
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedNpcs() {
        console.log("Seeding npcs...");
        let actions = [];
        npcs.forEach((n) => {
            let action = {
                name: "upsnpc",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: n
            };
            actions.push(action);
        });
        this.eos.actions(actions);
    }

    seedProgress() {
        console.log("Seeding progress...");
        let actions = [];
        progress.forEach((p) => {
            let action = {
                name: "upsprogress",
                authorization: [{
                    actor: "ebonhavencom",
                    permission: "active"
                }],
                data: p
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
    case "admin":
        seeder.createAdminAccount();
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
    case "mobs":
        seeder.seedMobs();
        break;
    case "resources":
        seeder.seedResources();
        break;
    case "treasures":
        seeder.seedTreasures();
        break;
    case "mapdata":
        seeder.seedMapdata();
        break;
    case "recipes":
        seeder.seedRecipes();
        break;
    case "quests":
        seeder.seedQuests();
        break;    
    case "npcs":
        seeder.seedNpcs();
        break;
    case "progress":
        seeder.seedProgress();
        break;
    case "retireall":
        seeder.retireAllItems();
        break;
    default:
        seeder.setup();
        seeder.createAdminAccount();
        seeder.setupStats();
        seeder.seedEffects();
        seeder.seedAuras();
        seeder.seedAbilities();
        seeder.seedItems();
        // seeder.seedCharacters();
        seeder.seedDrops();
        seeder.seedMobs();
        seeder.seedResources();
        seeder.seedTreasures();
        seeder.seedMapdata();
        seeder.seedRecipes();
        seeder.seedQuests();
        seeder.seedNpcs();
        seeder.seedProgress();
        break;
}
