#!/bin/bash
sleep 8
cd /home/pi/Watchdog/watchdog/web
sudo nohup node ./bin/www > ./output.log 2> error.log &
sleep 10
#sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 60" -o "ServerAliveCountMax 3" -N -R 80:localhost:8080 -p 2244 rssh@j1.qr-code.ws &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 60" -o "ServerAliveCountMax 3" -N -R 3333:localhost:8080 -p 2244 rssh@j1.qr-code.ws &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 60" -o "ServerAliveCountMax 3" -N -R 3334:localhost:22 -p 2244 rssh@j1.qr-code.ws &
