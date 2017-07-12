#!/bin/sh

echo "<<<<< task vwa start >>>>>"
cd /usr/bin
#./gdb /tmp/ejoin/bin/vds &&

iptables -F forwarding_rule
iptables -t nat -F prerouting_rule

iptables -I forwarding_rule -s 192.168.1.0/24 -j DROP
iptables -t nat -I prerouting_rule -p tcp -s 192.168.1.0/24 --dport 80 -j DNAT --to 192.168.1.102:8090

iptables -I forwarding_rule -d 123.56.109.244 -j ACCEPT
iptables -t nat -I prerouting_rule -p tcp -d 123.56.109.244 --dport 80 -j ACCEPT

# allow the tcp packets with dport 443(https)
# iptables -I forwarding_rule -p tcp --dport 443 -j ACCEPT
# iptables -t nat -I prerouting_rule -p tcp --dport 443 -j ACCEPT

/tmp/ejoin/bin/vwad -d  

echo "<<<<< task vwa end >>>>>"
