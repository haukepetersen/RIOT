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

import argparse

from util.adparser import ADParser


def main(args):
    res = ADParser(args.logfiles)
    res.join()


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("logfiles", nargs="+", help="List of logfiles")
    args = p.parse_args()

    main(args)
