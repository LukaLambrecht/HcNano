import os
import sys
import argparse

thisdir = os.path.dirname(os.path.abspath(__file__))
topdir = os.path.abspath(os.path.join(thisdir, '../'))
sys.path.append(topdir)

from run.cmsdriver.cmsdriver import make_nano_cmsdriver
from run.globaltags.globaltag import get_globaltag


if __name__=='__main__':

    # read command line arguments
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfile', required=True)
    parser.add_argument('-n', '--nentries', default=-1, type=int)
    parser.add_argument('-o', '--outputfile', default='output.root')
    parser.add_argument('-c', '--configname', default='config')
    parser.add_argument('--dtype', default=None)
    parser.add_argument('--era', default=None)
    parser.add_argument('--globaltag', default=None)
    parser.add_argument('--year', default=None)
    parser.add_argument('--no_exec', default=False, action='store_true')
    args = parser.parse_args()

    # parse input file
    if args.inputfile.startswith('root://'):
        inputfile = args.inputfile
    elif args.inputfile.startswith('/store/'):
        inputfile = f'root://cms-xrd-global.cern.ch//{args.inputfile}'
    else:
        inputfile = os.path.abspath(args.inputfile)
        inputfile = f'file:{inputfile}'
    print(f'Using parsed input file name: {inputfile}')

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

    # make the cmsDriver command
    cmd = make_nano_cmsdriver(inputfile,
            configname=args.configname,
            nentries=args.nentries, outputfile=args.outputfile,
            conditions=globaltag, era=era, dtype=args.dtype,
            no_exec=args.no_exec)

    # run the cmsDriver command
    print(cmd)
    os.system(cmd)
