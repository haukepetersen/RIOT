#!/usr/bin/python3
# -*- coding: utf-8 -*-

# Copyright (C) 2017 Freie Universität Berlin
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
# Author: Hauke Petersen <hauke.petersen@fu-berlin.de>

import argparse
import os, sys
import yaml
import re
from pathlib import Path

moddirs = [ "boards", "core", "cpu", "drivers", "pkg", "sys"]
dirskip = [ "include" ]

modules = dict()

def parse_module(cfg):
    with open(cfg, "r") as ymlfile:
        module = yaml.load(ymlfile)
        modules.update(module)

def readdir(base):
    for file in os.listdir(base):
        path = base + "/" + file
        if re.match(".+\.yml", file):
            parse_module(path)
        if os.path.isdir(path) and not file in dirskip:
            readdir(path)


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("riotbase", default="../../..", nargs="?", help="RIOT base directory")
    args = p.parse_args()

    # with open("config.yml", 'r') as ymlfile:
    #     config = yaml.load(ymlfile)
    #     print(config)

    # for target in moddirs:
    #     readdir(args.riotbase + "/" + target)

    for target in moddirs:
        for current, folders, files in os.walk(args.riotbase + "/" + target):
            for f in files:
                # print(f)
                m = re.match("(.+)\.yml", f)
                if m:
                    # print(m.group(1))
                    modules[m.group(1)] = current + "/" + f

    print(modules)
