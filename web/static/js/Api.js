'use strict';

/**
 * Created by davdeev on 3/12/17.
 */

class Api {

    constructor() {
        console.log("Api started");

        var _isLedOn = false;

        this.setIsLedOn = function(value) {
            _isLedOn = value;
        }

        this.isLedOn = function() {
            return _isLedOn;
        }
    }

    moveForward(callback) {
        $.get("/api/write?content=forward+speed%3D90", function () {
            Utils.callFunction(callback);
        });
    }

    moveBackward(callback) {
        $.get("/api/write?content=backward+speed%3D80", function () {
            Utils.callFunction(callback);
        });
    }

    turnLeft(callback) {
        $.get("/api/write?content=left+speed%3D80", function () {
            Utils.callFunction(callback);
        });
    }

    turnRight(callback) {
        $.get("/api/write?content=right+speed%3D80", function () {
            Utils.callFunction(callback);
        });
    }

    moveForwardRight(callback) {
        $.get("/api/write?content=forwardRight+speed%3D90", function () {
            Utils.callFunction(callback);
        });
    }

    moveForwardLeft(callback) {
        $.get("/api/write?content=forwardLeft+speed%3D90", function () {
            Utils.callFunction(callback);
        });
    }

    moveBackwardRight(callback) {
        $.get("/api/write?content=backwardRight+speed%3D90", function () {
            Utils.callFunction(callback);
        });
    }

    moveBackwardLeft(callback) {
        $.get("/api/write?content=backwardLeft+speed%3D90", function () {
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

    toggleLed() {
        if (this.isLedOn()) {
            this.turnOffLed();
        } else {
            this.turnOnLed();
        }

        this.setIsLedOn(!this.isLedOn());
    }

    turnOnLed(callback) {
        $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D2", function () {
            $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D5", function () {
                $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D11", function () {
                    Utils.callFunction(callback);
                });
            });
        });
        //
        // $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D2", function () {
        // });
    }

    turnOffLed(callback) {
        $.get("/api/write?content=led+r%3D0+g%3D0+b%3D0", function () {
            Utils.callFunction(callback);
        });
    }

    beep(callback) {
        $.get("/api/write?content=beep+duration%3D250+frequency%3D1000", function () {
            Utils.callFunction(callback);
        });
    }

    getTemperature(callback) {
        $.get("/api/write?content=getTemperature", function () {
            Utils.callFunction(callback);
        });
    }

    getSoundLevel(callback) {
        $.get("/api/write?content=getSoundLevel", function () {
            Utils.callFunction(callback);
        });
    }

    getLightLevel(callback) {
        $.get("/api/write?content=getLightLevel", function () {
            Utils.callFunction(callback);
        });
    }

    setTemperatureThreshold(threshold, callback) {
        $.get("/api/write?content=setTemperatureThreshold+threshold%3D"+threshold, function () {
            Utils.callFunction(callback);
        });
    }

    setSpeed(speed, callback) {
        $.get("/api/write?content=setSpeed+speed%3D"+speed, function () {
            Utils.callFunction(callback);
        });
    }

    setSoundLevelThreshold(threshold, callback) {
        $.get("/api/write?content=setSoundLevelThreshold+threshold%3D"+threshold, function () {
            Utils.callFunction(callback);
        });
    }

    setLightLevelThreshold(threshold, callback) {
        $.get("/api/write?content=setLightLevelThreshold+threshold%3D"+threshold, function () {
            Utils.callFunction(callback);
        });
    }

    setTimestamp(timestamp, callback) {
        $.get("/api/write?content=setTimestamp+timestamp%3D"+timestamp, function () {
            Utils.callFunction(callback);
        });
    }

    listen(callback) {
        var ws = new WebSocket("wss://" + location.hostname + ":" + location.port);
        ws.onopen = function (event) {
            console.log("Connected to ws", event);
            ws.onmessage = function(event) {
                console.log("ws got message", event);
                if (event.data) {
                    Utils.callFunction(callback, event.data);
                }
            }

            ws.onclose = function (event) {
                console.log("ws onclose", event);
            }

            ws.onerror = function (event) {
                console.log("ws onerror", event);
            }
        };
    }

    register(url, callback) {
        var _this = this;
        $.get("/api/register?url="+encodeURI(url), function (response) {
            Utils.callFunction(callback);
        });
    }
}
