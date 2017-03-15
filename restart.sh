#!/usr/bin/env bash

sudo killall node
sleep 5
cd /home/pi/Watchdog/watchdog/web
sudo nohup node ./bin/www isProduction=true > ./output.log 2> error.log &