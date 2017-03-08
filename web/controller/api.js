/**
 * Created by davdeev on 4/21/16.
 */

var express = require("express");
var router = express.Router();
var BtSerial = require("../logic/BtSerial2.js");
var btSerial = new BtSerial();

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

module.exports = router;
