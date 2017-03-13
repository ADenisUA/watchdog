'use strict';

/**
 * Created by davdeev on 3/12/17.
 */

class Api {

    constructor() {
        console.log("Api started");

        var _isStreaming = false;
        var _isLedOn = false;

        this.setIsStreaming = function(value) {
            _isStreaming = value;
        }

        this.isStreaming = function() {
            return _isStreaming;
        }

        this.setIsLedOn = function(value) {
            _isLedOn = value;
        }

        this.isLedOn = function() {
            return _isLedOn;
        }
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

    toggleStreaming() {
        if (this.isStreaming()) {
            this.stopStreaming();
        } else {
            this.startStreaming();
        }

        this.setIsStreaming(!this.isStreaming());
    }

    stopStreaming() {
        $("#streamingView").remove();
    }

    startStreaming() {
        $("#streamingContainer").append('<img id="streamingView" src="/stream/video.mjpeg" width="100%" height="100%" />');
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
        // $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D2", function () {
        //     $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D5", function () {
        //         $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D11", function () {
        //             Utils.callFunction(callback);
        //         });
        //     });
        // });

        $.get("/api/write?content=led+r%3D255+g%3D255+b%3D255+index%3D2", function () {
        });
    }

    turnOffLed(callback) {
        $.get("/api/write?content=led+r%3D0+g%3D0+b%3D0", function () {
            Utils.callFunction(callback);
        });
    }

    beep(callback) {
        $.get("/api/write?content=beep+duration%3D500+frequency%3D1000", function () {
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

    listen(callback) {
        var _this = this;
        $.get("/api/listen", function (data) {
            Utils.callFunction(callback, data);

            if (data && data.data) {
                console.log(data.data);
                $("#output").html(data.data);
            }

            _this.listen(callback);
        });
    }

    register(url, callback) {
        var _this = this;
        $.get("/api/register?url="+encodeURI(url), function (response) {
            Utils.callFunction(callback);
        });
    }
}
