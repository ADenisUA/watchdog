"use strict";

var btSerial = new (require('bluetooth-serial-port')).BluetoothSerialPort();

var BtSerial = module.exports = function BtSerial() {
    var _this = this;
    var _listenCallback = null;
    const DEVICE_NAME = "Makeblock";
    const DEVICE_ADDRESS = "00:0D:19:70:12:2C";

    const RESULT_ERROR_BAD_CHANNEL = "Bad channel";
    const RESULT_ERROR_CONNECTION_FAILURE = "Unable to connect";
    const RESULT_ERROR_WRITE_ERROR = "Write error";
    const RESULT_OK = "OK";

    // btSerial.on('found', function found(address, name){
    //     console.log('Discovered BT device ' + name + ' ' + address);
    //
    //     if (!btSerial.isOpen()) {
    //         _connect(name, address);
    //     }
    // },function() {
    //     console.log('no BT devices discovered');
    //     if (_address == null) {
    //         _this.connect(_callback);
    //     }
    // });

    btSerial.on('data', function(buffer) {
        var data = buffer.toString('utf-8');
        console.log("Received data:", data);
        if (_lastCommand) {
            console.log(_lastCommand);
        }
        if (_lastCommand && data.indexOf(_lastCommand) > -1) {
            _lastCommand = null;
            _writeIsInProgress = false;
            _processNextWriteQueueElement();
        }
        if (_listenCallback) _listenCallback(data);
    });

    btSerial.on('closed', function() {
        console.log("BT connection is closed");
    });

    btSerial.on('failure', function(error) {
        console.log("BT error", error);
    });

    // btSerial.on('finished', function() {
    //     console.log("BT discovery is completed");
    //     if (_address == null) {
    //         _this.connect(_callback);
    //     }
    // });

    this.connect = function(callback) {
        // console.log("Trying to find BT device");
        //
        // if (btSerial.isOpen()) {
        //     if (_callback) _callback();
        //     return;
        // } else if (_address != null) {
        //     _connect(DEVICE_NAME, _address);
        //     return;
        // }
        //
        // btSerial.listPairedDevices(function (devices) {
        //
        //     console.log("Paired BT Devices:");
        //
        //     for (var i=0; i<devices.length; i++) {
        //         var _device = devices[i];
        //         if (_device && _device.name) {
        //             console.log(_device.name, _device.address);
        //             if (_connect(_device.name, _device.address)) {
        //                 return;
        //             }
        //         }
        //     }
        //
        //     console.log("not found. Trying to discover devices");
        //
        //     btSerial.inquire();
        // });

        _connect(DEVICE_NAME, DEVICE_ADDRESS, callback);
    };

    var _connect = function(name, address, callback) {
        if (name.indexOf(DEVICE_NAME) > -1) {
            btSerial.findSerialPortChannel(address, function (channel) {
                console.log('Found BT COM channel for serial port on %s: ', name, address, channel);

                if (channel < 0) {
                    console.log(RESULT_ERROR_BAD_CHANNEL, channel);
                    if (callback) callback(RESULT_ERROR_BAD_CHANNEL);
                    return;
                }

                // make bluetooth connect to remote device
                btSerial.connect(address, channel, function () {
                    console.log("Connected to BT device", name, address);

                    _write("setTimestamp timestamp="+(Math.round(new Date().getTime()/1000)), callback);
                }, function() {
                    console.log(RESULT_ERROR_CONNECTION_FAILURE, name, address);
                    if (callback) callback(RESULT_ERROR_CONNECTION_FAILURE);
                });

            });
            return true;
        }
        return false;
    };

    var _writeQueue = new Array();
    var _writeIsInProgress = false;
    var _lastCommand = null;

    var _write = function (content, callback) {

        console.log("Attempting to write", content);

        if (_writeIsInProgress) {
            console.log("writeIsInProgress. Pushing request to the queue");
            _writeQueue.push({content: content, callback: callback});
            return;
        } else {
            _writeIsInProgress = true;
            _lastCommand = content;
        }

        btSerial.write(new Buffer(content, 'utf-8'), function(error, bytesWritten) {
            if (error) {
                console.log(error);
                if (callback) callback(RESULT_ERROR_WRITE_ERROR);
            } else {
                console.log("Written", content);
                if (callback) callback(RESULT_OK);
            }
        });
    };

    var _processNextWriteQueueElement = function () {
        console.log("_processNextWriteQueueElement", _writeQueue.size);
        if (_writeQueue.size > 0) {
            var nextCommand = _writeQueue.shift();
            console.log("nextCommand", nextCommand);
            _write(nextCommand.content, nextCommand.callback);
        }
    }

    this.write = function(content, callback) {
        if (!btSerial.isOpen()) {
            console.log("BT connection is closed. Reconnecting");

            _this.connect(function (result) {
                if (result == RESULT_OK) {
                    _write(content, callback);
                } else {
                    if (callback) callback(result);
                }
            });
        } else {
            _write(content, callback);
        }
    };

    this.listen = function(callback) {
        _listenCallback = callback;
    }
};