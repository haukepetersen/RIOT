#!/usr/bin/env python3

# This file is subject to the terms and conditions of the GNU Lesser
# General Public License v2.1. See the file LICENSE in the top level
# directory for more details.

import os
import sys


def testfunc(child):
    child.expect_exact('[SUCCESS]')


if __name__ == "__main__":
    path = os.path.join(os.environ['RIOTBASE'], 'dist/tools/testrunner')
    sys.path.append(path)
    from testrunner import run
    sys.exit(run(testfunc))
