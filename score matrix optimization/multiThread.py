# -*- coding: utf-8 -*-
"""
Created on Wed Jan 24 14:57:32 2018

@author: ATPs
"""

#! /usr/bin/python
# -*- coding: utf-8 -*-
###################################################
# Program: **.py
# Function:
# Version: V1.0
# Date: Fri Oct 25 11:30:50 2013
###################################################
import re
import time
import sys
import threading
import time
import random
import os
import glob

def main(Args):
    if not Args:
        print ("Usage: python <thread_count> <in_file>")
        sys.exit(0)
    thread_count = int(Args[0])
    in_file = Args[1]

    class Worker(threading.Thread):
        def __init__(self, shell):
            self.shell = shell
            threading.Thread.__init__(self)
        def run(self):
            os.system(self.shell)
            time.sleep(random.randint(10, 100) / 1000.0)

    threading_dict = {}
    threading_count = 1
    for lines in open(in_file, "r"):
        if re.match("#", lines) or not re.search("[0-9a-zA-Z]+", lines):
            continue
        threading_dict[threading_count] = Worker(lines.rstrip())
        threading_count += 1

    for keys in threading_dict:
        threading_dict[keys].start()
        while True:
            if threading.active_count() > thread_count:
                time.sleep(2)
            else:
                break

#########################################################################
USAGE = """
location <command> <thread_count> <in_file>
Version: V1.0
  Date: Fri Oct 25 11:30:50 2013
        """
#########################################################################
if __name__ == '__main__':
    if len(sys.argv) > 1:
        main(sys.argv[1:])
    else:
        print (USAGE)
        sys.exit(0)
