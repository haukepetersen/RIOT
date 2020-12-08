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
import sys
import argparse
from dateutil import tz
from datetime import datetime

from util.adparser import ADParser
from util.plotter import Plotter



CWA_SVC_STR =  "03036FFD"


class PltDevs(ADParser):

    def __init__(self, logfiles):
        super().__init__(logfiles)

        basename = os.path.splitext(os.path.basename(logfiles[0]))[0]
        if len(logfiles) > 1:
            basename += "+{}".format(len(logfiles) - 1)
        self.plotter = Plotter(basename)


    def _tim_str(self, ts):
        return "{}".format(datetime.utcfromtimestamp(ts))


    def plot_num_of_devs(self, binsize=3600):
        curbin = int(self.pkts[0].time / binsize) * binsize

        cnt_non = []
        cnt_cwa = []
        ticks = []

        for pkt in self.pkts:
            if pkt.time > curbin:
                cnt_non.append(0)
                cnt_cwa.append(0)
                ticks.append(self._tim_str(curbin))
                curbin += binsize

            if CWA_SVC_STR in pkt.payload:
                cnt_cwa[-1] += 1
            else:
                cnt_non[-1] += 1

        info = {
            "title": "Number of Received Advertising packets per {}s".format(binsize),
            "xlabel": "Date",
            "ylabel": "# of recorded advertising packtes",
            "suffix": "cnt",
        }

        data = {
            "x": ticks,
            "y": [cnt_non, cnt_cwa],
            "label": ["Non-CWA packets", "Corona-Warn-App packets"],
        }

        self.plotter.barchart(info, data)


def main(args):
    res = PltDevs(args.logfiles)
    res.plot_num_of_devs()


if __name__ == "__main__":
    p = argparse.ArgumentParser()
    p.add_argument("logfiles", nargs="+", help="List of logfiles")
    args = p.parse_args()

    main(args)
