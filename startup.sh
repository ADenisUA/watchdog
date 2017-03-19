#!/bin/bash
sleep 8
/usr/bin/tvservice -o
cd /home/pi/Watchdog/watchdog/web
sudo nohup node ./bin/www isProduction=true > ./output.log 2> error.log &
sleep 10
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -N -R 80:localhost:80 root@107.181.154.239 &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -N -R 443:localhost:443 root@107.181.154.239 &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -N -R 3001:localhost:3001 root@107.181.154.239 &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -N -R 3478:localhost:3478 root@107.181.154.239 &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -N -R 2222:localhost:22 root@107.181.154.239 &

#sudo iptables -A PREROUTING -t nat -p tcp --dport 80 -j REDIRECT --to-port 3333
#sudo iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 443 -j ACCEPT
#sudo iptables -t nat -A OUTPUT -o lo -p tcp --dport 443 -j REDIRECT --to-port 3334
#sudo iptables -t nat -A PREROUTING -p tcp --dport 443 -j REDIRECT --to-ports 3334
#sudo iptables -t nat -I OUTPUT -p tcp --dport 443 -j REDIRECT --to-ports 3334