import os
import sys
import json
import argparse
import numpy as np
import awkward as ak

thisdir = os.path.abspath(os.path.dirname(__file__))
topdir = os.path.abspath(os.path.join(thisdir, '../'))
sys.path.append(topdir)

from tools.samplelisttools import read_sampledict


if __name__=='__main__':

    # read command line args
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfile', required=True)
    parser.add_argument('--weighted', default=False, action='store_true')
    parser.add_argument('--entry_start', default=None)
    parser.add_argument('--entry_stop', default=None)
    args = parser.parse_args()

    # set branches to read
    fragmentation_branches = [
      'cFragmentationPdgId',
      'cBarFragmentationPdgId'
    ]
    branches_to_read = fragmentation_branches[:]
    if args.weighted:
        raise Exception('Not yet implemented.')

    # read the input file
    events = {}
    treename = 'analyzer/Events'
    dummykey = 'all'
    sampledict = {dummykey: [args.inputfile]}
    print('Reading ntuple...')
    events = read_sampledict(sampledict,
                          treename=treename,
                          branches=branches_to_read,
                          entry_start=args.entry_start,
                          entry_stop=args.entry_stop)
    events = events[dummykey]

    # loop over c and cbar
    for branch in fragmentation_branches:
        print(f'--- {branch} ---')
        values = np.abs(events[branch].to_numpy().astype(int))
        pdgids, counts = np.unique(values, return_counts=True)
        fracs = counts/len(values)
        for pdgid, count, frac in zip(pdgids, counts, fracs):
            info = f'Pdg ID {pdgid} (abs): {count}'
            info += ' ({:.2f}%)'.format(frac*100)
            print(info)
