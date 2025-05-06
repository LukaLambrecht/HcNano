import os
import sys
import json
import argparse
import numpy as np
import awkward as ak
import matplotlib.pyplot as plt

thisdir = os.path.abspath(os.path.dirname(__file__))
topdir = os.path.abspath(os.path.join(thisdir, '../'))
sys.path.append(topdir)

from plotting.plot import plot
from tools.plottools import make_hist
from tools.variabletools import read_variables
from tools.samplelisttools import read_sampledict


if __name__=='__main__':

    # read command line args
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfiles', required=True, nargs='+')
    parser.add_argument('-v', '--variables', required=True)
    parser.add_argument('-o', '--outputdir', required=True)
    parser.add_argument('--donormalized', default=False, action='store_true')
    parser.add_argument('--dolog', default=False, action='store_true')
    parser.add_argument('--extracmstext', default=None)
    parser.add_argument('--lumiheader', default=None)
    parser.add_argument('--extrainfo', default=None)
    parser.add_argument('--entry_start', default=None)
    parser.add_argument('--entry_stop', default=None)
    args = parser.parse_args()

    # hard-coded arguments
    args.chargebranch1 = 'DStarMeson_Pi2_charge'
    args.chargebranch2 = 'DStarMeson_K_charge'
    genmatchbranches = ([
      'DStarMeson_hasFastGenmatch',
      'DStarMeson_hasFastAllOriginGenmatch'
    ])

    # read variables
    variables = read_variables(args.variables)
    variablelist = [v.variable for v in variables]

    # set branches to read
    branches_to_read = variablelist
    branches_to_read += genmatchbranches
    branches_to_read.append(args.chargebranch1)
    branches_to_read.append(args.chargebranch2)

    # read the input file
    events = {}
    treename = 'Events'
    dummykey = 'all'
    sampledict = {dummykey: args.inputfiles}
    print('Reading ntuple...')
    events = read_sampledict(sampledict,
                          mode='uproot',
                          treename=treename,
                          branches=branches_to_read,
                          entry_start=args.entry_start,
                          entry_stop=args.entry_stop)

    # flatten all variables
    new_events = {}
    for key, sample in events.items():
        new_sample = {}
        for variable in sample.fields:
            new_sample[variable] = ak.flatten(sample[variable], axis=None)
        new_sample = ak.Array(new_sample)
        new_events[key] = new_sample
    events = new_events
    
    # make masks for opposite sign and same sign candidates
    charge1 = events[dummykey][args.chargebranch1].to_numpy().astype(int)
    charge2 = events[dummykey][args.chargebranch2].to_numpy().astype(int)
    mask_os = (charge1*charge2 < 0)
    mask_ss = (~mask_os)
    mask_pp = ((mask_ss) & (charge1>0))
    mask_nn = ((mask_ss) & (charge1<0))

    # make masks for gen-match categories
    mask_genmatch = events[dummykey]['DStarMeson_hasFastGenmatch'].to_numpy().astype(bool)
    mask_allgenmatch = events[dummykey]['DStarMeson_hasFastAllOriginGenmatch'].to_numpy().astype(bool)
    mask_nonhardscattergenmatch = ((mask_allgenmatch) & (~mask_genmatch))

    # split events into categories
    events_matched = events[dummykey][((mask_genmatch) & (mask_os))]
    events_matched_nonhardscatter = events[dummykey][((mask_nonhardscattergenmatch) & (mask_os))]
    events_notmatched_os = events[dummykey][((~mask_allgenmatch) & (mask_os))]
    events_notmatched_ss = events[dummykey][((~mask_allgenmatch) & (mask_ss))]
    events_notmatched_pp = events[dummykey][((~mask_allgenmatch) & (mask_pp))]
    events_notmatched_nn = events[dummykey][((~mask_allgenmatch) & (mask_nn))]
    events = {}
    events['notmatched-os'] = events_notmatched_os
    #events['matched-nhs'] = events_matched_nonhardscatter
    #events['matched'] = events_matched
    events['notmatched-pp'] = events_notmatched_pp
    events['notmatched-nn'] = events_notmatched_nn
    events['notmatched-ss'] = events_notmatched_ss
        
    # set colors
    colordict = {
        dummykey: 'dodgerblue',
        'matched': 'darkorchid',
        'matched-nhs': 'orchid',
        'notmatched-os': 'dodgerblue',
        'notmatched-ss': 'red',
        'notmatched-pp': 'gold',
        'notmatched-nn': 'forestgreen'
    }

    # set labels
    labeldict = {
        dummykey: 'All events',
        'matched': 'Gen-matched',
        'matched-nhs': 'Gen-matched (not from proton)',
        'notmatched-os': 'Not gen-matched',
        'notmatched-ss': 'SS',
        'notmatched-pp': 'SS (++)',
        'notmatched-nn': 'SS (--)'
    }

    # set styles
    styledict = {
        dummykey: 'fill',
        'matched': 'fill',
        'matched-nhs': 'fill',
        'notmatched-os': 'fill',
        'notmatched-ss': 'step',
        'notmatched-pp': 'step',
        'notmatched-nn': 'step'
    }

    # make output directory
    if not os.path.exists(args.outputdir):
        os.makedirs(args.outputdir)

    # loop over variables
    for varidx, variable in enumerate(variables):
        print(f'Plotting variable {variable.name}')
        
        # make histograms
        hists = {}
        for key, sample in events.items():
            values = sample[variable.variable].to_numpy().astype(float)
            weights = None
            hists[key] = make_hist(values, variable, weights=weights)

        # loop over plot options
        stacklist = list(events.keys())
        normalization = [False]
        if args.donormalized: normalization.append(True)
        log = [False]
        if args.dolog: log.append(True)
        for normalize in normalization:
          for logscale in log:

            # define how to stack histograms
            stacklist = ['matched', 'matched-nhs', 'notmatched-os']
            if normalize: stacklist = None

            # make plot
            yaxtitle = 'Events'
            if normalize: yaxtitle += ' (normalized)'
            fig, ax = plot(bkg=hists,
                       stacklist=stacklist,
                       variable=variable,
                       colordict=colordict,
                       labeldict=labeldict,
                       styledict=styledict,
                       normalize=normalize,
                       logscale=logscale,
                       extracmstext=args.extracmstext,
                       lumiheader=args.lumiheader,
                       yaxtitle=yaxtitle,
                       dolegend=True)

            # add some extra info
            if args.extrainfo is not None:
                ax.text(0.02, 0.9, args.extrainfo, ha='left', va='top',
                        transform=ax.transAxes, fontsize=12)

            # save the figure
            fig.tight_layout()
            figname = variable.name
            if normalize: figname += '_norm'
            if logscale: figname += '_log'
            figname += '.png'
            figname = os.path.join(args.outputdir, figname)
            fig.savefig(figname)
            plt.close(fig)
