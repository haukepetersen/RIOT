#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2019 Hauke Petersen <hauke.petersen@fu-berlin.de>
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

import re
import os
import argparse

def lout(var, raw):
    return "  {} ?= {}\n".format(var, "".join([x[-2:] for x in raw.split(", ")]))


def parseline(line):
    res = ""
    m = re.match(r'(?P<org>[-a-zA-Z0-9_]+)\s+(?P<id>[a-z0-9]+)\s+'
                 r'{ (?P<appeui_lsb>[x ,0-9a-zA-Z]+) }\s+'
                 r'{ (?P<appeui_msb>[x ,0-9a-zA-Z]+) }\s+'
                 r'{ (?P<deveui_lsb>[x ,0-9a-zA-Z]+) }\s+'
                 r'{ (?P<deveui_msb>[x ,0-9a-zA-Z]+) }\s+'
                 r'{ (?P<appkey_lsb>[x ,0-9a-zA-Z]+) }\s+'
                 r'{ (?P<appkey_msb>[x ,0-9a-zA-Z]+) }.*', line)
    if m:
        res += "ifeq ({},$(ID))\n".format(m.group("id"))
        res += lout("DEVEUI", m.group("deveui_msb"))
        res += lout("APPEUI", m.group("appeui_msb"))
        res += lout("APPKEY", m.group("appkey_msb"))
        res += "endif\n"
    return res



def main(args):
    out = ""
    with open(args.cfgfile, "r", encoding="utf-8") as f:
        for line in f:
            out += parseline(line)
    print(out)
    with open(args.outfile, "w", encoding="utf-8") as f:
        f.write(out)

if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("cfgfile", default="cfg.txt", nargs="?", help="Key config file")
    p.add_argument("outfile", default="nodecfg.inc.mk", nargs="?")
    args = p.parse_args()
    main(args)
