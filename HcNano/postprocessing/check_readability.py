# Check if CRAB output files are readable (i.e. not corrupted)


import os
import sys
import six
import glob
import uproot
import argparse


if __name__=='__main__':

    # read command line args
    # note: the input directory can be anything;
    #       all ROOT files in it are found recursively
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputdir', required=True)
    parser.add_argument('-o', '--outputfile', default=None)
    args = parser.parse_args()

    # find files
    pattern = os.path.join(args.inputdir, '**/*.root')
    rootfiles = glob.glob(pattern, recursive=True)
    print(f'Found {len(rootfiles)} files.')

    # loop over files and open them
    failed = []
    for idx, rootfile in enumerate(rootfiles):
        msg = f'Reading file {idx+1} / {len(rootfiles)}'
        msg += f' (currently {len(failed)} / {idx+1} failed)'
        print(msg, end='\r')
        try:
            with uproot.open(f'{rootfile}:Events') as tree:
                nevents = tree.num_entries
        except: failed.append(rootfile)
    print()

    # print results
    print('Read {} files, of which {} seem to have failed.'.format(len(rootfiles), len(failed)))
    if len(failed)>0: print(failed)

    # also write results to file
    if args.outputfile is not None:
        with open(args.outputfile, 'w') as f:
            for failed_file in failed:
                f.write(failed_file+'\n')
