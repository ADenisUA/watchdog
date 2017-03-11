'use strict';

let instance;

class Watchdog {

    constructor() {
        console.log("Watchdog started");
    }

    static getInstance() {
        if (!instance) {
            instance = new Watchdog();
            instance.listen(null);
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

    toggleStreaming() {
        if (this.isStreaming) {
            this.stopStreaming();
        } else {
            this.startStreaming();
        }

        this.isStreaming = !this.isStreaming;
    }

    stopStreaming() {
        $("#streamingView").remove();
    }

    startStreaming() {
        $("#streamingContainer").append('<img id="streamingView" src="/stream/video.mjpeg" width="100%" height="100%" />');
    }

    toggleLed() {
        if (this.isLedOn) {
            this.turnOffLed();
        } else {
            this.turnOnLed();
        }

        this.isLedOn = !this.isLedOn;
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
        $.get("/api/listen", function (data) {
            Utils.callFunction(callback);
            console.log(data);
            $("#output").html(data.data + $("#output").html());
            instance.listen(callback);
        });
    }
}

class Utils {
    static callFunction(_function, _data) {
        if (_function) {
            _function(_data);
        }
    }
}

//console.log(Watchdog.getInstance());