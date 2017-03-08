"use strict";

var btSerial = new (require('bluetooth-serial-port')).BluetoothSerialPort();


var BtSerial = module.exports = function BtSerial() {
    var _this = this;
    var _callback = null;
    var _address = null;
    const DEVICE_NAME = "Makeblock";

    btSerial.on('found', function found(address, name){
        console.log('Discovered device ' + name + ' ' + address);

        if (!btSerial.isOpen()) {
            _connect(name, address);
        }
    },function() {
        console.log('found nothing');
    });

    btSerial.on('data', function(buffer) {
        console.log(buffer.toString('utf-8'));
    });

    this.connect = function(callback) {
        _callback = callback;

        if (btSerial.isOpen()) {
            if (_callback) _callback();
            return;
        }

        btSerial.listPairedDevices(function (devices) {

            console.log("Paired Devices");

            for (var i=0; i<devices.length; i++) {
                var _device = devices[i];
                if (_device && _device.name) {
                    console.log(_device.name, _device.address);
                    if (_connect(_device.name, _device.address)) {
                        return;
                    }
                }
            }

            btSerial.inquire();
        });

        //btSerial.inquire();
    };

    var _connect = function(name, address) {
        if (name.indexOf(DEVICE_NAME) > -1) {
            _address = address;

            btSerial.findSerialPortChannel(address, function (channel) {
                console.log('Found RFCOMM channel for serial port on %s: ', name, channel);

                if (channel < 0) {
                    _address = null;
                    console.log("Channel is " + channel + " retrying");
                    _this.connect(_callback);
                    return;
                }

                // make bluetooth connect to remote device
                btSerial.connect(address, channel, function () {

                    console.log('Connected to ', name);
                    if (_callback) _callback();

                }, function() {
                    console.log('Unable to connect ');
                    _this.connect(_callback);
                });

            });
            return true;
        }
        return false;
    }

    this.write = function(content, callback) {
        if (!btSerial.isOpen()) {
            console.log("Connection is closed. connecting...");
            _this.connect(function () {
                btSerial.write(new Buffer(content, 'utf-8'), function(error, bytesWritten) {
                    if (error) console.log(error);
                    console.log("bytesWritten", bytesWritten);
                    if (callback) callback();
                });
            });
        } else {
            btSerial.write(new Buffer(content, 'utf-8'), function(error, bytesWritten) {
                if (error) console.log(error);
                console.log("bytesWritten", bytesWritten);
                if (callback) callback();
            });
        }
    };
};