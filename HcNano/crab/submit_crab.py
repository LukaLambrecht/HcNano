import os
import sys
import six
import argparse

thisdir = os.path.abspath(os.path.dirname(__file__))
topdir = os.path.abspath(os.path.join(thisdir, '../'))
sys.path.append(topdir)

from run.globaltags.globaltag import get_globaltag
from make_cmsrun_config import make_cmsrun_config


if __name__=='__main__':

    # read command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--samplelist', required=True, nargs='+')
    parser.add_argument('-o', '--outputdirname', required=True)
    parser.add_argument('-n', '--nentries', default=-1, type=int)
    parser.add_argument('--dtype', default=None)
    parser.add_argument('--era', default=None)
    parser.add_argument('--globaltag', default=None)
    parser.add_argument('--year', default=None)
    parser.add_argument('--test', default=False, action='store_true')
    args = parser.parse_args()

    # read samplelists
    datasets = []
    for samplelist in args.samplelist:
        with open(samplelist, 'r') as f:
            samples = [l.strip('\n') for l in f.readlines()]
            datasets += samples
    print(f'Read following datasets from provided samplelist ({len(datasets)}):')
    for d in datasets: print(f'  - {d}')

    # parse global tag
    if args.globaltag is not None and args.globaltag.endswith('.json'):
        if args.year is None:
            msg = 'Passing a json file for the global tag requires specifying the year.'
            raise Exception(msg)
        globaltag = get_globaltag(args.globaltag, args.year)
    else: globaltag = args.globaltag
    print(f'Using global tag: {globaltag}')

    # make the cmsRun config
    pset = 'cmsrun_config.py'
    print(f'Building cmsRun config file {pset}...')
    pset = make_cmsrun_config(pset, 
             dtype=args.dtype, era=args.era, conditions=globaltag)

    # check the CRAB config
    crab_config = 'crab_config.py'
    if not os.path.exists(crab_config):
        msg = f'CRAB config {crab_config} does not exist.'
        raise Exception(msg)

    # loop over datasets
    cmds = []
    for didx, dataset in enumerate(datasets):
        print('Submitting dataset {} ({}/{}) using CRAB...'.format(
          dataset, didx+1, len(datasets)))

        # set environment variables
        os.environ['CRAB_DATASET'] = dataset
        os.environ['CRAB_PSETNAME'] = pset
        os.environ['CRAB_OUTPUTDIR'] = args.outputdirname
        os.environ['CRAB_UNITSPERJOB'] = '1'
        os.environ['CRAB_ENTRIESPERUNIT'] = str(args.nentries)
        os.environ['CRAB_TEST'] = str(args.test)

        # run crab config
        # (only for producing some printouts for testing,
        # without actually submitting)
        if args.test: os.system(f'python3 {crab_config}')

        # submit
        else: os.system(f'crab submit -c {crab_config}')
