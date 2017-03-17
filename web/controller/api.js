/**
 * Created by davdeev on 4/21/16.
 */

var express = require("express");
var request = require('request');
var router = express.Router();
var BtSerial = require("../lib/BtSerial.js");
var btSerial = new BtSerial();
var admin = require("firebase-admin");
var serviceAccount = require("../firebase.json");

const RESULT_OK = "OK";

var _listenData = "";
var _lastResponse = null;

btSerial.connect(function (result) {
    if (result == RESULT_OK) {
        btSerial.listen(function (data) {
            _listenData += data;

            sendNotification(data);

            if (_lastResponse && !_lastResponse.finished) {
                _lastResponse.status(200).json({data: _listenData});
                _listenData = "";
            }
        });
    }
});

router.get('/write', function(request, response) {
    var content = request.param("content");

    btSerial.write(content, function (result) {
        response.status((result == RESULT_OK) ? 200 : 500).json({status: result});
    });
});

router.get('/listen', function(request, response) {
    _lastResponse = response;
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
        var host = url.substring(0, url.lastIndexOf("/"));
        var id = url.substring(url.lastIndexOf("/") + 1);
        var payload = "";

        if (data && (data.indexOf("\"event\"") > -1)) {
            //we need to process only events

            var rows = data.match(/[^\r\n]+/g);
            payload = "[";

            for (var i in rows) {
                if (rows[i].indexOf("\"event\"") > -1) {
                    payload += rows[i];
                }
            }

            payload += "]";
        } else {
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
