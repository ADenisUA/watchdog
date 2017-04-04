#!/bin/bash
sleep 8
/usr/bin/tvservice -o
cd /home/pi/Watchdog/watchdog/web
sudo nohup node ./bin/www isProduction=true > ./output.log 2> error.log &
sleep 10
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 155.94.169.47:80:localhost:80 -o ServerAliveInterval=30 root@findreview.info &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 155.94.169.47:443:localhost:443 -o ServerAliveInterval=30 root@findreview.info &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 155.94.169.47:3001:localhost:3001 -o ServerAliveInterval=30 root@findreview.info &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R 155.94.169.47:3478:localhost:3478 -o ServerAliveInterval=30 root@findreview.info &
sudo -u pi autossh -M 0 -f -o "ServerAliveInterval 30" -o "ServerAliveCountMax 3" -o "TCPKeepAlive yes" -N -R \*:2222:localhost:22 -o ServerAliveInterval=30 root@findreview.info &

#sudo iptables -A PREROUTING -t nat -p tcp --dport 80 -j REDIRECT --to-port 3333
#sudo iptables -A INPUT -m state --state NEW -m tcp -p tcp --dport 443 -j ACCEPT
#sudo iptables -t nat -A OUTPUT -o lo -p tcp --dport 3001 -j REDIRECT --to-port 3001
#sudo iptables -t nat -A PREROUTING -p tcp --dport 3001 -j REDIRECT --to-ports 3001
#sudo iptables -t nat -I OUTPUT -p tcp --dport 3001 -j REDIRECT --to-ports 3001