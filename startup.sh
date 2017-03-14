#!/bin/bash
sleep 8
cd /home/pi/Watchdog/watchdog/web
sudo nohup node ./bin/www isProduction=true > ./output.log 2> error.log &
sleep 10
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 60" -o "ServerAliveCountMax 3" -N -R 3333:localhost:8080 -p 2244 rssh@j1.qr-code.ws &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 60" -o "ServerAliveCountMax 3" -N -R 3334:localhost:8081 -p 2244 rssh@j1.qr-code.ws &
#sudo iptables -A PREROUTING -t nat -p tcp --dport 80 -j REDIRECT --to-port 3333
#sudo iptables -A PREROUTING -t nat -p tcp --dport 443 -j REDIRECT --to-port 3334