/**
 * Created by davdeev on 4/21/16.
 */

const express = require("express");
const request = require('request');
const router = express.Router();
const BtSerial = require("../lib/BtSerial.js");
const btSerial = new BtSerial();
const Utils = require("../lib/Utils.js");
const admin = require("firebase-admin");
const serviceAccount = require("../firebase.json");

const RESULT_OK = "OK";

var _btDataCallback = null;

btSerial.connect(function (result) {
    if (result == RESULT_OK) {
        btSerial.listen(function (data) {

            Utils.callFunction(_btDataCallback, data);
            sendNotification(data);
        });
    }
});

router.onBtData = function(callback) {
    _btDataCallback = callback;
}

router.get('/write', function(request, response) {
    var content = request.param("content");

    btSerial.write(content, function (result) {
        Utils.respond(response, (result == RESULT_OK) ? 200 : 500, {status: result});
    });
});

var notificationUrls = new Array();

router.get('/register', function(request, response) {
    var url = request.param("url");

    if (notificationUrls.indexOf(url) == -1) {
        console.log("registering url: "+ url);
        notificationUrls.push(url);
    }

    response.status(200).json({status: "OK"});
});

admin.initializeApp({
    credential: admin.credential.cert(serviceAccount),
    databaseURL: "https://watchdog-d1d6f.firebaseio.com"
});

function sendNotification(data) {

    for (var i in notificationUrls) {
        var url = notificationUrls[i];
        // Set up the request
        var id = url.substring(url.lastIndexOf("/") + 1);
        var payload = "";

        var eventsCount = 0;

        if (data && data.indexOf && (data.indexOf("\"event\"") > -1)) {
            //we need to process only events

            var rows = data.match(/[^\r\n]+/g);
            payload = "[";

            for (var i in rows) {
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

        if (eventsCount == 0) {
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

module.exports = router;
