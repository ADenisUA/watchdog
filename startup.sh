#!/bin/bash
sleep 8
cd /home/pi/Watchdog/watchdog/web
sudo nohup  node ./bin/www > ./output.log 2> error.log &
sshpass -p 4545sfAzvafq ssh -nNT -R 3333:localhost:80 -p 2244 rssh@j1.qr-code.ws &
