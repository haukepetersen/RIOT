#!/usr/bin/env python3


import subprocess
import re

releases = ['origin/2015.09-branch',
            'origin/2015.12-branch',
            'origin/2016.04-branch',
            'origin/2016.07-branch',
            'origin/2016.10-branch',
            'origin/2017.01-branch',
            'origin/2017.04-branch',
            'origin/2017.07-branch',
            'origin/2017.10-branch',
            'origin/2018.01-branch',
            'origin/2018.04-branch',
            'origin/2018.07-branch',
            'origin/master']

apis = ['core/include/irq.h',
        'core/include/kernel_defines.h',
        'core/include/kernel_init.h',
        'core/include/kernel_types.h',
        'core/include/mbox.h',
        'core/include/mutex.h',
        'core/include/msg.h',
        'core/include/thread.h',
        'drivers/include/net/netdev.h',
        'drivers/include/periph/gpio.h']

def get_last_commit(branch, api_header):
    try:
        foo = subprocess.check_output(['git', 'log', '-q', '-n 1', branch, api_header])
        m = re.match("commit ([0-9a-f]+)", foo.decode('utf-8'))
        if m:
            print("  {} - {}".format(m.group(1), api_header))
        else:
            print("  {} not found".format(api_header))
    except:
        print("  not found")


for branch in releases:
    print("analyzing branch '{}'".format(branch))
    for a in apis:
        get_last_commit(branch, a)
