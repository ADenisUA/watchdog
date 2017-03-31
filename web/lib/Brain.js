"use strict";

const Utils = require("./Utils.js");
const BtSerial = require("../lib/BtSerial.js");
const btSerial = new BtSerial();
const admin = require("firebase-admin");
const serviceAccount = require("../firebase.json");

const WebSockets = require('./WebSockets.js');

admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://watchdog-d1d6f.firebaseio.com"
});

let instance;
const notificationUrls = [];

module.exports = class Brain {

    constructor() {
        console.log("Brain started");

        let webSocket = null;
        let _this = this;

        btSerial.connect(function (result) {
            if (result === Brain.RESULT_OK) {
                btSerial.listen(function (data) {
                    if (webSocket !== null) {
                        webSocket.write(data);
                    }

                    _this.sendNotification(data);
                });
            }
        });

        this.useServer = function(httpsServer) {
            if (webSocket === null) {
                webSocket = new WebSockets(httpsServer);
            }
        }
    }

    static get RESULT_OK() {
        return "OK";
    };

    static getInstance() {
        if (!instance) {
            console.log("Creating brain");
            instance = new Brain();
        }
        return instance;
    }

    write(content, callback) {
        btSerial.write(content, callback);
    }

    addListener(url) {
        if (notificationUrls.indexOf(url) === -1) {
            console.log("registering url: "+ url);
            notificationUrls.push(url);
        }
    }

    getApi() {
        return api;
    }

    sendNotification(data) {
        for (let i in notificationUrls) {
            let url = notificationUrls[i];
            // Set up the request
            let id = url.substring(url.lastIndexOf("/") + 1);
            let payload = "";
            let eventsCount = 0;

            if (data && data.indexOf && (data.indexOf("\"event\"") > -1)) {
                //we need to process only events

                let rows = data.match(/[^\r\n]+/g);
                payload = "[";

                for (let i in rows) {
                    try {
                        if (rows[i].indexOf("\"event\"") > -1 && JSON.parse(rows[i])) {
                            payload += (payload.length > 1) ? "," : "";
                            payload += rows[i];
                        }
                        eventsCount++;
                    } catch (e) {

                    }
                }

                payload += "]";
            } else {
                return;
            }

            if (eventsCount === 0) {
                console.log("No events detected", data);
                return;
            }

            console.log("sendNotification", payload);

            admin.messaging().sendToDevice(id, {notification: {
                title: "Watchdog",
                body: ""
            }, data: {payload: payload}})
                .then(function (response) {
                    // See the MessagingDevicesResponse reference documentation for
                    // the contents of response.
                    console.log("Successfully sent message:", response);
                })
                .catch(function (error) {
                    console.log("Error sending message:", error);
                });
        }
    }
};