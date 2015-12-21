#!/bin/bash

sudo ip tuntap add tap0 mode tap user ${USER}
sudo ip link set tap0 up
sudo ip route add 10.10.10.0/24 dev tap0
