import os
import sys
import six
import argparse

thisdir = os.path.abspath(os.path.dirname(__file__))
topdir = os.path.abspath(os.path.join(thisdir, '../'))
sys.path.append(topdir)

from run.globaltags.globaltag import get_globaltag
from run.tools.samplelisttools import read_samplelists
from make_cmsrun_config import make_cmsrun_config


if __name__=='__main__':

    # read command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--samplelist', required=True, nargs='+')
    parser.add_argument('-o', '--outputdirname', required=True)
    parser.add_argument('--splitting', default='FileBased',
      choices = ['Automatic', 'FileBased', 'LumiBased', 'EventAwareLumiBased'],
      help = 'See https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile')
    parser.add_argument('--units_per_job', default=-1, type=int,
      help = 'See https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile')
    parser.add_argument('--total_units', default=-1, type=int,
      help = 'See https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile')
    parser.add_argument('--recovery', default=False, action='store_true')
    parser.add_argument('--dtype', default=None, choices=[None, 'mc', 'data'])
    parser.add_argument('--era', default=None)
    parser.add_argument('--globaltag', default=None)
    parser.add_argument('--year', default=None)
    parser.add_argument('--test', default=False, action='store_true')
    args = parser.parse_args()

    # read samplelists
    datasets = read_samplelists(args.samplelist, verbose=True)

    # parse global tag
    if args.globaltag is not None and args.globaltag.endswith('.json'):
        if args.year is None:
            msg = 'Passing a json file for the global tag requires specifying the year.'
            raise Exception(msg)
        globaltag = get_globaltag(args.globaltag, year=args.year, dtype=args.dtype)['globaltag']
    else: globaltag = args.globaltag
    print(f'Using global tag: {globaltag}')

    # parse era
    if args.era is not None and args.era.endswith('.json'):
        if args.year is None:
            msg = 'Passing a json file for the era requires specifying the year.'
            raise Exception(msg)
        era = get_globaltag(args.era, year=args.year, dtype=args.dtype)['era']
    else: era = args.era
    print(f'Using era: {era}')

    # make the cmsRun config
    pset = 'cmsrun_config.py'
    print(f'Building cmsRun config file {pset}...')
    pset = make_cmsrun_config(pset, 
             dtype=args.dtype, era=era, conditions=globaltag)

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

        # make a request name (for CRAB internal bookkeeping)
        (_, sample, version, tier) = dataset.split('/')
        sample_short = '_'.join(sample.split('_')[:2])
        version_short = '_'.join(version.split('_')[:1])
        request_name = sample_short + '_' + version_short

        # handle case of recovery job
        if args.recovery:
            # check if crab working directory already exists
            # note: naming convention must be in sync with crab_config.py,
            #       maybe try to automate later.
            crab_area = os.path.join('crab_logs', sample, version, 'crab_' + request_name)
            crab_area = os.path.abspath(crab_area)
            if not os.path.exists(crab_area):
                msg = f'WARNING: requested recovery job for dataset {dataset},'
                msg += f' but the expected crab area {crab_area} does not exist.'
                print(msg)
                continue
            # run the crab report command
            reportcmd = f'crab report {crab_area} --recovery notFinished'
            os.system(reportcmd)
            missing_lumis = os.path.join(crab_area, 'results', 'notFinishedLumis.json')
            if not os.path.exists(missing_lumis):
                msg = f'WARNING: could not find notFinishedLumis.json;'
                msg += f' either there are no missing lumis, or the crab report command failed.'
                print(msg)
                continue
            # make a new request name
            request_name = request_name + '_recovery'

        # set environment variables
        os.environ['CRAB_DATASET'] = dataset
        os.environ['CRAB_REQUESTNAME'] = request_name
        os.environ['CRAB_PSETNAME'] = pset
        os.environ['CRAB_OUTPUTDIR'] = args.outputdirname
        os.environ['CRAB_SPLITTING'] = args.splitting
        os.environ['CRAB_UNITSPERJOB'] = str(args.units_per_job)
        os.environ['CRAB_TOTALUNITS'] = str(args.total_units)
        os.environ['CRAB_LUMIMASK'] = missing_lumis if args.recovery else ''

        # run crab config
        # (only for producing some printouts for testing,
        # without actually submitting)
        if args.test: os.system(f'python3 {crab_config}')

        # submit
        else:
            cmd = f'crab submit -c {crab_config}'
            print(cmd)
            os.system(cmd)
