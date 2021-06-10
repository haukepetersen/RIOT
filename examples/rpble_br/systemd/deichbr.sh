#!/bin/sh

IF=usb0
PREFIX=fd23:affe::/32
OWN_LL=fe80::1/64
BR_LL=fe80::2
SERVER_ADDR=fd42:affe::1
KEA_CONF=/home/hauke/services/deichbr/kea.deichbr.conf

if [ ${#} -ne 1 ]; then
    echo "usage: ${0} <up|down>"
    exit 1
fi

if [ ${1} = "up" ]; then
    echo "Deich Border Router Setup -> UP"
    # set local address
    ip address add ${SERVER_ADDR} dev lo
    # configure BR ethernet interface
    sysctl -w net.ipv6.conf.${IF}.forwarding=1
    sysctl -w net.ipv6.conf.${IF}.accept_ra=0
    ip address add ${OWN_LL} dev ${IF}
    ip route add ${PREFIX} via ${BR_LL} dev ${IF}
    # run KEA
    mkdir -p /var/kea
    mkdir -p /var/run/kea
    sleep 2
    kea-dhcp6 -c ${KEA_CONF}

elif [ ${1} = "down" ]; then
    echo "Deich Border Router Setup -> DOWN"
    # remove local address
    ip address del ${SERVER_ADDR} dev lo

else
    echo "Invalid command"
fi
