/**
 * Created by davdeev on 4/21/16.
 */

var express = require("express");
var request = require('request');
var router = express.Router();
var BtSerial = require("../logic/BtSerial.js");
var btSerial = new BtSerial();
var admin = require("firebase-admin");
var serviceAccount = require("../firebase.json");

router.get('/connect', function(request, response) {
    btSerial.connect(function (result) {
        //response.status(404).json({status: "error"});
        response.json(result);
    });
});

router.get('/write', function(request, response) {
    var content = request.param("content");

    console.log("attempting to write: "+ content);

    btSerial.write(content, function (result) {
        //response.status(404).json({status: "error"});
        console.log("written: "+ content);
        response.status(200).json({status: "ok"});
    });
});

var listenData = "";

router.get('/listen', function(request, response) {
    btSerial.listen(function (data) {
        listenData += data;

        if (!response.finished) {
            response.status(200).json({data: listenData});
            sendNotification(listenData);
            listenData = "";
        }
    });
});

var notificationUrls = new Array();

router.get('/register', function(request, response) {
    var url = request.param("url");

    if (notificationUrls.indexOf(url) == -1) {
        console.log("registering url: "+ url);
        notificationUrls.push(url);
    }

    response.status(200).json({status: "ok"});
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

        admin.messaging().sendToDevice(id, {notification: {
                    title: "Portugal vs. Denmark",
                    body: "5 to 1"
                }, data: {test: "test"}})
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
