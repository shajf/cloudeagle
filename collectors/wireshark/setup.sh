#!/bin/bash
#By shajf

[ `id -u` -ne 0 ] && {
	echo "You must run this by root" >&2
	exit 1
}
mkdir -p /var/data/mirro
/usr/local/wireshark/bin/tshark  -i eth3  -f 'tcp port 80 and (((ip[2:2] - ((ip[0]&0xf)<<2)) - ((tcp[12]&0xf0)>>2)) != 0)' -R 'http.request' -T mirror -J /var/data/mirro -M "/usr/local/apache2/bin/mlogc /opt/waf/conf/mlogc.conf"

