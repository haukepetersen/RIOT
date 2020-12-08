#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2020 Freie Universität Berlin
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA
#
# author: Hauke Petersen <hauke.petersen@fu-berlin.de>


import os
import re
import sys
import argparse
from enum import Enum

CWA_SVC_STR =  "03036FFD"

class AddrType(Enum):
    PUBLIC = 0
    RANDOM = 1
    RPA_PUBLIC = 2
    RPA_RANDOM = 3


class EventType(Enum):
    ADV_IND = 0
    DIR_IND = 1
    SCAN_IND = 2
    NONCONN_IND = 3
    SCAN_RSP = 4


class Addr():
    def __init__(self, addr_type, addr):
        self.addr = addr
        self.type = AddrType(int(addr_type))

    def __eq__(self, other):
        return self.type == other.type and self.addr == other.addr

    def __lt__(self, other):
        return str(self) < str(other)

    def __str__(self):
        return "{}-{}".format(self.addr, self.type.name)

    def __hash__(self):
        return hash(str(self))


class Pkt():
    def __init__(self, time, type, addr, rssi, payload, raw):
        self.time = time
        self.type = type
        self.addr = addr
        self.rssi = rssi
        self.payload = payload
        self.raw = raw

    def __str__(self):
        return "TIME:{} SRC:{} RSSI:{}dbm".format(self.time, self.addr, self.rssi)

    def static(self):
        return "{};{}".format(self.addr, self.payload)


class ADParser():
    def __init__(self, logs):
        self.pkts = []

        self.nodes = {}


        for logfile in logs:
            self.parse(logfile)


    def parse(self, logfile):
        cnt = 0
        with open(logfile, "r", encoding="utf-8") as f:
            for line in f:
                cnt += 1
                m = re.match(r'(?P<time>\d+\.\d+);'
                             r'(?P<event_type>\d);'
                             r'(?P<addr_type>\d);(?P<addr>[:a-fA-F0-9]+);'
                             r'(?P<rssi>-?\d+);'
                             r'(?P<payload>[a-zA-Z0-9]+)', line)
                if m:

                    pkt = Pkt(float(m.group("time")),
                              EventType(int(m.group("event_type"))),
                              Addr(m.group("addr_type"), m.group("addr")),
                              int(m.group("rssi")),
                              m.group("payload"),
                              line)
                    self.pkts.append(pkt)

    def join(self):
        for pkt in self.pkts:
            print(pkt.raw, end="")


    def filter_cwa(self):
        for pkt in self.pkts:
            if CWA_SVC_STR in pkt.payload:
                print(pkt.raw, end="")


    def filter_noncwa(self):
        for pkt in self.pkts:
            if CWA_SVC_STR not in pkt.payload:
                print(pkt.raw, end="")


    def compress(self):
        seen = {}

        for pkt in self.pkts:
            static = pkt.static()
            if static not in seen:
                seen[static] = 1
                print(pkt.raw, end="")
            else:
                seen [static] += 1


    def nodeinfo(self):
        for pkt in self.pkts:
            if pkt.addr not in self.nodes:
                self.nodes[pkt.addr] = {
                    "rssi": [],
                    "time": [],
                }
            self.nodes[pkt.addr]["rssi"].append(pkt.rssi)
            self.nodes[pkt.addr]["time"].append(pkt.time)

        for node in sorted(self.nodes):
            n = self.nodes[node]

            # try to calculate advertising interval
            itvl = []
            cur = n["time"][0]
            for t in n["time"][1:]:
                itvl.append(t - cur)
                cur = t

            print("{}:".format(node))
            print("  rssi: {}dbm / {}dbm / {:.2f}dbm (min/max/avg)".format(
                        min(n["rssi"]),
                        max(n["rssi"]),
                        (sum(n["rssi"]) / len(n["rssi"]))))
            if len(itvl) > 0:
                print("  itvl: {:.2f}s / {:.2f}s / {:.2f}s".format(
                        min(itvl), max(itvl), (sum(itvl) / len(itvl))))
