#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (C) 2017  Hauke Petersen <dev@haukepetersen.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import sys

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.exit("No filename given")

    count = [0] * 256

    with open(sys.argv[1], "r") as f:
        for line in f:
            try:
                num = int(line)
                count[num] += 1
            except:
                print("no num")

    for i in range(0, 256):
        print("%d, %d" % (i, count[i]))
