#!/bin/bash
sleep 8
/usr/bin/tvservice -o
cd /home/pi/Watchdog/watchdog/web
sudo nohup node ./bin/www isProduction=true > ./output.log 2> error.log &
sleep 10
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 0.0.0.0:80:localhost:80 -o ServerAliveInterval=30 root@findreview.info &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 0.0.0.0:443:localhost:443 -o ServerAliveInterval=30 root@findreview.info &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 0.0.0.0:3001:localhost:3001 -o ServerAliveInterval=30 root@findreview.info &
#sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 0.0.0.0:3478:localhost:3478 -o ServerAliveInterval=30 root@findreview.info &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 0.0.0.0:2222:localhost:22 -o ServerAliveInterval=30 root@findreview.info &