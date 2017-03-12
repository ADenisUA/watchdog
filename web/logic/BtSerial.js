"use strict";

var btSerial = new (require('bluetooth-serial-port')).BluetoothSerialPort();


var BtSerial = module.exports = function BtSerial() {
    var _this = this;
    var _callback = null;
    var _listenCallback = null;
    var _address = null;
    const DEVICE_NAME = "Makeblock";
    const DEVICE_ADDRESS = "00:0D:19:70:12:2C";

    btSerial.on('found', function found(address, name){
        console.log('Discovered BT device ' + name + ' ' + address);

        if (!btSerial.isOpen()) {
            _connect(name, address);
        }
    },function() {
        console.log('no BT devices discovered');
        if (_address == null) {
            _this.connect(_callback);
        }
    });

    btSerial.on('data', function(buffer) {
        var data = buffer.toString('utf-8');
        console.log(data);
        if (_listenCallback) _listenCallback(data);
    });

    btSerial.on('closed', function() {
        console.log("BT connection is closed");
    });

    btSerial.on('failure', function(error) {
        console.log("BT error", error);
    });

    btSerial.on('finished', function() {
        console.log("BT discovery is completed");
        if (_address == null) {
            _this.connect(_callback);
        }
    });

    this.connect = function(callback) {
        _callback = callback;

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

        _connect(DEVICE_NAME, DEVICE_ADDRESS);
    };

    var _connect = function(name, address) {
        if (name.indexOf(DEVICE_NAME) > -1) {
            _address = address;

            btSerial.findSerialPortChannel(address, function (channel) {
                console.log('Found BT COM channel for serial port on %s: ', name, channel);

                if (channel < 0) {
                    _address = null;
                    console.log("BT channel is " + channel + ". Retrying");
                    _this.connect(_callback);
                    return;
                }

                // make bluetooth connect to remote device
                btSerial.connect(address, channel, function () {

                    console.log('Connected to BT device ', name);
                    if (_callback) _callback();

                }, function() {
                    console.log('Unable to connect to BT device. Retrying', name);
                    _this.connect(_callback);
                });

            });
            return true;
        }
        return false;
    }

    this.write = function(content, callback) {
        if (!btSerial.isOpen()) {
            console.log("BT connection is closed. Reconnecting");
            _this.connect(function () {
                btSerial.write(new Buffer(content, 'utf-8'), function(error, bytesWritten) {
                    if (error) console.log(error);
                    //console.log("BT bytesWritten", bytesWritten);
                    if (callback) callback();
                });
            });
        } else {
            btSerial.write(new Buffer(content, 'utf-8'), function(error, bytesWritten) {
                if (error) console.log(error);
                //console.log("bytesWritten", bytesWritten);
                if (callback) callback();
            });
        }
    };

    this.listen = function(callback) {
        _listenCallback = callback;
    }
};