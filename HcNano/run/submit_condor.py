import os
import sys
import six
import argparse

thisdir = os.path.abspath(os.path.dirname(__file__))
topdir = os.path.abspath(os.path.join(thisdir, '../'))
sys.path.append(topdir)

import run.tools.condortools as ct
from run.tools.datasettools import get_files


if __name__=='__main__':

    # read command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-s', '--samplelist', required=True, nargs='+')
    parser.add_argument('-o', '--outputdir', required=True, type=os.path.abspath)
    parser.add_argument('-m', '--maxfiles', default=-1, type=int,
      help='Number of files to process per entry in samplelist.')
    parser.add_argument('-n', '--nentries', default=-1, type=int,
      help='Number of entries to process per file.')
    parser.add_argument('--proxy', default=None)
    parser.add_argument('--dtype', default=None)
    parser.add_argument('--era', default=None)
    parser.add_argument('--globaltag', default=None)
    parser.add_argument('--year', default=None)
    args = parser.parse_args()

    # get CMSSW
    cmssw_version = os.getenv('CMSSW_BASE')
    print('Found following CMSSW base:')
    print(f'  - {cmssw_version}')
    if cmssw_version is None:
        msg = 'CMSSW version not set. Do cmsenv.'
        raise Exception(msg)

    # get proxy
    proxy = None
    if args.proxy is not None:
        proxy = os.path.abspath(args.proxy)
        print('Found following proxy:')
        print(f'  - {proxy}')

    # read samplelists
    datasets = []
    for samplelist in args.samplelist:
        with open(samplelist, 'r') as f:
            samples = [l.strip('\n') for l in f.readlines()]
            datasets += samples
    print(f'Read following datasets from provided samplelist ({len(datasets)}):')
    for d in datasets: print(f'  - {d}')

    # get input files
    inputfiles = {}
    for dataset in datasets:
        inputfiles[dataset] = get_files(dataset, maxfiles=args.maxfiles, verbose=True)
    ninputfiles = sum([len(list(files)) for files in inputfiles.values()])

    # ask for confirmation
    print(f'Will submit {ninputfiles} jobs. Continue? (y/n)')
    go = six.moves.input()
    if go!='y': sys.exit()

    # set output directories
    outputdirs = {}
    for dataset in datasets:
        dirname = dataset.strip('/').replace('/', '_')
        dirname = dirname.replace('_MINIAODSIM', '')
        dirname = dirname.replace('_MINIAOD', '')
        outputdirs[dataset] = os.path.join(args.outputdir, dirname)

    # loop over datasets and input files
    cmds = []
    for didx, dataset in enumerate(datasets):
        for fidx, f in enumerate(inputfiles[dataset]):

            # set output file
            outputfile = os.path.join(outputdirs[dataset], f'output_{fidx+1}.root')

            # set config file
            # todo: find cleaner solution, e.g. re-use the same config file for all jobs
            # or use a separate working directory for each job.
            configname = f'temp_config_{didx}_{fidx}'
        
            # make the command
            cmd = f'python3 cmsrun.py'
            cmd += f' -i {f}'
            cmd += f' -n {args.nentries}'
            cmd += f' -o {outputfile}'
            cmd += f' -c {configname}'
            if args.dtype is not None: cmd += f' --dtype {args.dtype}'
            if args.era is not None: cmd += f' --era {args.era}'
            if args.globaltag is not None: cmd += f' --globaltag {args.globaltag}'
            if args.year is not None: cmd += f' --year {args.year}'
            cmds.append(cmd)

    # make output directories
    for dataset in datasets:
        if not os.path.exists(outputdirs[dataset]):
            os.makedirs(outputdirs[dataset])

    # submit the commands
    for cmd in cmds: os.system(cmd)
    #name = 'cjob_cmsrun'
    #ct.submitCommandsAsCondorCluster(name, cmds,
    #    proxy=proxy,
    #    cmssw_version=cmssw_version,
    #    jobflavour='workday')
