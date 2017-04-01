"use strict";

const WebSocket = require('ws');
const url = require('url');

module.exports = class WebSockets {
    constructor(httpsServer) {
        const wsServer = new WebSocket.Server({ server: httpsServer });
        const connections = [];

        wsServer.on('connection', function(connection) {
            const location = url.parse(connection.upgradeReq.url, true);
            console.log("Established wss connection", location);

            connections.push(connection);

            connection.on('message', function(message) {
                console.log('received: %s', message);
            });

            connection.on('close', function() {
                const location = url.parse(connection.upgradeReq.url, true);
                console.log('disconnected', location);
            });
        });

        this.write = function(data) {
            let result = false;
            for (let i in connections) {
                if (connections[i] && connections[i].readyState === connections[i].OPEN) {
                    connections[i].send(data);
                    result = true;
                }
            }
            return result;
        }
    }
};