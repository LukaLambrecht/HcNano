# Do post-processing of CRAB output


import os
import sys
import six
import glob
import argparse


if __name__=='__main__':

    # read command line args
    # note: the input directory is supposed to be the top-level CRAB output directory,
    #       with the following sub-structure (as defined in the CRAB config file):
    #       <top-level>/<sample name>/<version name>/<sample name>/<request name>/<datetag>/0000/nanoaod_*.root
    # note: output will look like this:
    #       <top-level>/<sample name>_<version name>/nanoaod_*.root
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputdir', required=True)
    parser.add_argument('-t', '--test', default=False, action='store_true')
    args = parser.parse_args()

    # find samples
    samples = {}
    for samplename in os.listdir(args.inputdir):
        sampledir = os.path.join(args.inputdir, samplename)
        samples[samplename] = {}
        # find versions per sample
        for version_name in os.listdir(sampledir):
            versiondir = os.path.join(sampledir, version_name)
            if not os.path.isdir(versiondir): continue
            samples[samplename][version_name] = []
            # check that there is only one request name and datetag per version
            datetags = glob.glob(os.path.join(versiondir, '*/*/*'))
            if len(datetags)!=1:
                msg = 'Found unexpected number of subdirectories'
                msg += f' for sample {sample_name} / {version_name}:'
                msg += f' {datetags}'
                raise Exception(msg)
            # find files for this sample/version
            rootfiles = glob.glob(os.path.join(versiondir, '*/*/*/*/*.root'))
            samples[samplename][version_name] = rootfiles

    # do printouts
    print('Found following samples:')
    for samplename, versiondict in samples.items():
        print(f'  - {samplename}')
        for version_name, files in versiondict.items():
            print(f'    - {version_name}: {len(files)} files')

    # make a sample_version directory for each sample and move all files
    newsamples = []
    for samplename, versiondict in samples.items():
        for version_name, files in versiondict.items():
            versiondir = os.path.join(args.inputdir, samplename, version_name)
            source = os.path.join(versiondir, '*/*/*/*/*.root')
            newsamplename = samplename+'_'+version_name
            newsamples.append(newsamplename)
            newsampledir = os.path.join(args.inputdir, newsamplename)
            mkdircmd = f'mkdir -p {newsampledir}'
            if not args.test: os.system(mkdircmd)
            mvcmd = f'mv {source} {newsampledir}'
            print(mvcmd)
            if not args.test: os.system(mvcmd)

    # do printouts
    print('New samples:')
    for samplename in newsamples:
        sampledir = os.path.join(args.inputdir, samplename)
        if os.path.exists(sampledir):
            rootfiles = [f for f in os.listdir(sampledir) if f.endswith('.root')]
            print(f'  - {samplename}: {len(rootfiles)} files')
        else: print(f'  - {samplename}: does not exist.')
    print('Original directories:')
    for samplename, versiondict in samples.items():
        print(f'  - {samplename}')
        for version_name, files in versiondict.items():
            versiondir = os.path.join(args.inputdir, samplename, version_name)
            rootfiles = glob.glob(os.path.join(versiondir, '*/*/*/*/*.root'))
            print(f'    - {version_name}: {len(rootfiles)} files')

    # remove original directories
    print('Remove original directories (should be empty if all went fine)? (y/n)')
    go = six.moves.input()
    if go == 'y':
        for samplename in samples.keys():
            sampledir = os.path.join(args.inputdir, samplename)
            rmcmd = f'rm -r {sampledir}'
            print(rmcmd)
            if not args.test: os.system(rmcmd)
