'use strict';

let instance;

class Watchdow {

    constructor() {
        console.log("Watchdow started");
    }

    static getInstance() {
        if (!instance) {
            instance = new Watchdow();
        }
        return instance;
    }

    moveForward(callback) {
        $.get("/api/write?content=forward", function () {
            Utils.callFunction(callback);
        });
    }

    moveBackward(callback) {
        $.get("/api/write?content=backward", function () {
            Utils.callFunction(callback);
        });
    }

    turnLeft(callback) {
        $.get("/api/write?content=left", function () {
            Utils.callFunction(callback);
        });
    }

    turnRight(callback) {
        $.get("/api/write?content=right", function () {
            Utils.callFunction(callback);
        });
    }

    moveForwardRight(callback) {
        $.get("/api/write?content=forwardRight", function () {
            Utils.callFunction(callback);
        });
    }

    moveForwardLeft(callback) {
        $.get("/api/write?content=forwardLeft", function () {
            Utils.callFunction(callback);
        });
    }

    moveBackwardRight(callback) {
        $.get("/api/write?content=backwardRight", function () {
            Utils.callFunction(callback);
        });
    }

    moveBackwardLeft(callback) {
        $.get("/api/write?content=backwardLeft", function () {
            Utils.callFunction(callback);
        });
    }

    stop(callback) {
        $.get("/api/write?content=stop", function () {
            Utils.callFunction(callback);
        });
    }

    findLightDirection(callback) {
        $.get("/api/write?content=findLightDirection", function () {
            Utils.callFunction(callback);
        });
    }

    navigate(callback) {
        $.get("/api/write?content=navigate", function () {
            Utils.callFunction(callback);
        });
    }

    stopStreaming() {
        $("#streamingView").remove();
    }

    startStreaming() {
        $("#streamingContainer").append('<img id="streamingView" src="/stream/video.mjpeg" width="100%" height="100%" />');
    }
}

class Utils {
    static callFunction(_function, _data) {
        if (_function) {
            _function(_data);
        }
    }
}

//console.log(Watchdow.getInstance());