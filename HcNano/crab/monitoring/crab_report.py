#!/bin/env python3

# Make a report (including missing lumis) for all CRAB jobs
# More info: https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3Commands#crab_report

import os
import sys
import six
import glob
import argparse


if __name__ == '__main__':

    # read command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--crabdirs', required=True, nargs='+')
    args = parser.parse_args()

    # loop over samples
    for idx, crabdir in enumerate(args.crabdirs):
        print('Now processing sample {} of {}'.format(idx+1, len(args.crabdirs)))
        print('({})'.format(crabdir))

        # make cmd
        cmd = f'crab report {crabdir} --recovery notFinished'

        # execute command
        os.system(cmd)
