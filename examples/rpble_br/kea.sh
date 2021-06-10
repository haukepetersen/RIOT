#!/bin/sh

mkdir -p /var/run/kea
kea-dhcp6 -c kea-dhcp6.conf
