const { Api, JsonRpc, RpcError, Serialize } = require('eosjs');
const { TextEncoder, TextDecoder } = require('util');
const { JsSignatureProvider } = require('eosjs/dist/eosjs-jssig');
//const JsSignatureProvider = require('eosjs/dist/eosjs-jssig').default;
const fetch = require('node-fetch').default;

class EOS {

    constructor(endpoint, keys, account) {
        let signatureProvider = new JsSignatureProvider(keys);
        this.signatureProvider = signatureProvider;
        let rpc = new JsonRpc(endpoint, { fetch });
        this.rpc = rpc;
        this.account = account;
        this.api = new Api({
            rpc,
            signatureProvider,
            textDecoder: new TextDecoder(),
            textEncoder: new TextEncoder()
        });
    };

    setAccount(account) {
      this.account = account;
    }

    async getInfo() {
        let response = await this.rpc.get_info();
        return response;
    }

    async readTable(table, scope, limit = 10) {
        let response = await this.rpc.get_table_rows({
            json: true,
            code: this.account,
            scope: scope,
            table: table,
            limit: limit
        });

        return response;
    }

    async createUser(user, authorization, permissions, broadcast = true, sign = true, blocksBehind = 3, expireSeconds = 30) {
        let result = await this.api.transact({
            actions: [{
                account: 'eosio',
                name: 'newaccount',
                authorization: authorization,
                data: {
                    creator: 'eosio',
                    name: user,
                    owner: {
                        threshold: 1,
                        keys: [{
                            key: permissions.owner,
                            weight: 1
                        }],
                        accounts: [],
                        waits: []
                    },
                    active: {
                        threshold: 1,
                        keys: [{
                            key: permissions.active,
                            weight: 1
                        }],
                        accounts: [],
                        waits: []
                    }
                }
            }]
        }, {
            broadcast: broadcast,
            sign: sign,
            blocksBehind: blocksBehind,
            expireSeconds: expireSeconds
        });

        return result;
    }

    async action(actionName, authorization, data, broadcast = true, sign = true, blocksBehind = 3, expireSeconds = 30) {
        let response = await this.api.transact({
            actions: [{
                account: this.account,
                name: actionName,
                authorization: authorization,
                data: data
            }]
        }, 
        {
            broadcast: broadcast,
            sign: sign,
            blocksBehind: blocksBehind,
            expireSeconds: expireSeconds
        });

        return response;
    };

    async actions(actions, broadcast = true, sign = true, blocksBehind = 3, expireSeconds = 30) {
        let accountActions = [];
        actions.forEach((a) => {
            a.account = this.account;
            accountActions.push(a);
        });
        let response = await this.api.transact({
            actions: accountActions
        }, 
        {   
            broadcast: broadcast,
            sign: sign,
            blocksBehind: blocksBehind,
            expireSeconds: expireSeconds
        });

        return response;
    };

}

module.exports = EOS;