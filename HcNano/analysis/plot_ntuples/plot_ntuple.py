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
    parser.add_argument('--genmatchbranch', default=None)
    parser.add_argument('--chargebranch1', default=None)
    parser.add_argument('--chargebranch2', default=None)
    parser.add_argument('--weighted', default=False, action='store_true')
    parser.add_argument('--xsec', default=0, type=float)
    parser.add_argument('--lumi', default=0, type=float)
    parser.add_argument('--donormalized', default=False, action='store_true')
    parser.add_argument('--dolog', default=False, action='store_true')
    parser.add_argument('--extracmstext', default=None)
    parser.add_argument('--lumiheader', default=None)
    parser.add_argument('--extrainfo', default=None)
    parser.add_argument('--entry_start', default=None)
    parser.add_argument('--entry_stop', default=None)
    args = parser.parse_args()

    # read variables
    variables = read_variables(args.variables)
    variablelist = [v.variable for v in variables]

    # set branches to read
    branches_to_read = variablelist
    if args.genmatchbranch is not None:
        branches_to_read.append(args.genmatchbranch)
    if args.chargebranch1 is not None:
        branches_to_read.append(args.chargebranch1)
    if args.chargebranch2 is not None:
        branches_to_read.append(args.chargebranch2)
    if args.weighted:
        branches_to_read.append('genWeight')

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

    # for weighted events, make the weights
    weights = None
    if args.weighted:
        runs = read_sampledict(sampledict,
                 mode='uproot',
                 treename='Runs',
                 branches=['genEventSumw'],
                 verbose=False)
        sumweights = runs[dummykey]['genEventSumw']
        sumweights = np.sum(sumweights)
        norm = args.xsec * args.lumi / sumweights
        weights = events[dummykey]['genWeight'] * norm
        # convert per-event weights to per-candidate weights
        # by first broadcasting and then flattening
        # (note: this assumes all variables have the same nested structure)
        dummyvar = variablelist[0]
        dummyvalues = events[dummykey][dummyvar]
        weights = ak.broadcast_arrays(weights, dummyvalues)[0]
        weights = ak.flatten(weights, axis=None).to_numpy()

    # flatten all variables
    new_events = {}
    for key, sample in events.items():
        new_sample = {}
        for variable in sample.fields:
            if variable == 'genWeight': continue
            new_sample[variable] = ak.flatten(sample[variable], axis=None)
        # add weights
        if args.weighted: new_sample['weight'] = weights
        new_sample = ak.Array(new_sample)
        new_events[key] = new_sample
    events = new_events
    
    # make masks
    mask_genmatch = None
    if args.genmatchbranch is not None:
        mask_genmatch = events[dummykey][args.genmatchbranch].to_numpy().astype(bool)
    mask_os = np.ones(len(events[dummykey]))
    if args.chargebranch1 is not None and args.chargebranch2 is not None:
        charge1 = events[dummykey][args.chargebranch1].to_numpy().astype(int)
        charge2 = events[dummykey][args.chargebranch2].to_numpy().astype(int)
        mask_os = (charge1*charge2 < 0)

    # split events into categories
    if mask_genmatch is not None:
        events_matched = events[dummykey][((mask_genmatch) & (mask_os))]
        events_notmatched = events[dummykey][((~mask_genmatch) & (mask_os))]
        events = {}
        events['notmatched'] = events_notmatched
        events['matched'] = events_matched
        
        # print the counts
        print(f"Total events: {len(events_matched) + len(events_notmatched)}")
        print(f"Matched events: {len(events_matched)}")
        print(f"Not matched events: {len(events_notmatched)}")

    # set colors
    colordict = {
        dummykey: 'dodgerblue',
        'matched': 'darkorchid',
        'notmatched': 'dodgerblue'
    }

    # set labels
    labeldict = {
        dummykey: 'All events',
        'matched': 'Gen-matched',
        'notmatched': 'Not gen-matched'
    }

    # set styles
    styledict = {
        dummykey: 'step',
        'matched': 'step',
        'notmatched': 'fill'
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
            if args.weighted: weights = sample['weight'].to_numpy().astype(float)
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
            stacklist = list(events.keys())
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
