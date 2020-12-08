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
import numpy as np
import matplotlib.pyplot as plt

class Plotter:

    def __init__(self, outfile_name):
        self.outfile_name = outfile_name


    def init_plot(self):
        fig, ax = plt.subplots()
        fig.set_size_inches(11.7, 8.3)
        return fig, ax


    def finish_plot(self, plt, title, suffix):
        pdffile = "{}_{}.pdf".format(self.outfile_name, suffix)
        pngfile = "{}_{}.png".format(self.outfile_name, suffix)
        # if os.path.isfile(pdffile):
            # os.remove(pdffile)
        if title != None:
            plt.title(title)
        plt.savefig(pdffile, dpi=300, format='pdf', bbox_inches='tight')
        plt.savefig(pngfile, dpi=300, format='png', bbox_inches='tight', pad_inches=0.01)
        plt.show()
        plt.close()


    def linechart(self, info, data, xticks=None, grid=None, step=True):
        '''
        info: title, labels, suffix, ylim:[min,max], ...,
        data: list of axes defs:
              -> [{x: [], y: [], label: "", style:"-"}, ...]
        '''

        fig, ax = self.init_plot()

        for i, line in enumerate(data):
            style = "-"
            if "style" in line:
                style = line["style"]
            else:
                if i > 8:
                    style = ":"
            color = line["color"] if "color" in line else None
            if step:
                ax.step(line["x"], line["y"], where="post", label=line["label"], linestyle=style, color=color)
            else:
                ax.plot(line["x"], line["y"], label=line["label"], linestyle=style, color=color)
            # plt.plot(line["x"], line["y"], 'C2o', alpha=0.5) # for verification...

        if xticks:
            ax.set_xticks(xticks["ticks"])
            ax.set_xticklabels(xticks["labels"])

        if grid and "x" in grid:
            ax.xaxis.grid(grid["x"])
        if grid and "y" in grid:
            ax.yaxis.grid(grid["y"])


        plt.xlabel(info["xlabel"])
        plt.ylabel(info["ylabel"])
        plt.legend(fontsize=8)
        if "ylim" in info:
            plt.ylim(info["ylim"])
        if "xlim" in info:
            plt.xlim(info["xlim"])
        self.finish_plot(plt, info["title"], info["suffix"])


    def _bar(self, ax, title, ticks, ys, labels, ylim=None):
        x = np.arange(len(ticks))
        width = 0.8 / len(ys)
        offset = 0.8 / len(ys) / 2

        ax.tick_params(axis='both', which='major', labelsize=8)
        ax.set_title(title, fontsize=9)

        if ylim:
            ax.set_ylim(bottom=ylim[0], top=ylim[1])
            # yts = [ylim[0], ylim[1] / 2, ylim[1]]
            # ax.set_yticks(np.arange(len(yts)))
            # ax.set_yticklabels(yts)

        ax.set_xticks(x)
        ax.set_xticklabels(ticks)
        ax.tick_params(labelrotation=90)

        for i, y in enumerate(ys):
            ax.bar(x - 0.4 + ((2 * i + 1) * offset), y, width, label=labels[i])

        ax.legend(fontsize=8)


    def barchart(self, info, data, stacked=True):
        '''
        info: [title, xlabel, ylabel, suffix, grid[True,False]]
        data: {
            x: tick labels for x axix -> len(data["x"]) == len(data["y"][0])
            label: [] -> label for each bar group -> len(data["label"]) == len(data["y"])
            y: [[1, 2, 3,4], [4, 5, 6, 7], ...] one array for each bar group
            [opt] ylim: limit for y axix
            ylim: maximum y axis value
        }
        '''
        fix, ax = self.init_plot()
        self._bar(ax, info['title'], data["x"], data["y"], data["label"])

        plt.xlabel(info['xlabel'])
        plt.ylabel(info['ylabel'])

        if "grid" in info:
            ax.yaxis.grid(info["grid"])
        else:
            ax.yaxis.grid(True)

        self.finish_plot(plt, info['title'], info['suffix'])
