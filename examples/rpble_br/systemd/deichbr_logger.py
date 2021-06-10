#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (C) 2019  Hauke Petersen <dev@haukepetersen.de>
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

import os
import sys
import serial
import logging
import argparse

BAUDRATE = 115200


def main():
    parser = argparse.ArgumentParser(description="DeichBR Logger - Log Serial Port to Sytemd")
    parser.add_argument("tty", help="Serial port")
    args = parser.parse_args()

    logging.basicConfig(level=logging.WARNING, format='%(asctime)s # %(message)s')
    logging.warning("DeichBR Logger v0.23")

    with serial.Serial(args.tty, BAUDRATE) as ser:
        while True:
            line = ser.readline()
            logging.warning(line[:-1].decode("utf-8"))


if __name__ == "__main__":
    main()

