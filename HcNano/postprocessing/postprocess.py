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
    parser.add_argument('-m', '--mode', default='cp', choices=['cp', 'mv'])
    parser.add_argument('-t', '--test', default=False, action='store_true')
    args = parser.parse_args()

    # find samples
    samples = {}
    for sample_name in os.listdir(args.inputdir):
        sample_dir = os.path.join(args.inputdir, sample_name)
        samples[sample_name] = {}
        # find versions per sample
        for version_name in os.listdir(sample_dir):
            version_dir = os.path.join(sample_dir, version_name)
            if not os.path.isdir(version_dir): continue
            samples[sample_name][version_name] = {}
            # find request names per version
            # (assumed to come from different recovery tasks)
            for request_name in os.listdir(os.path.join(version_dir, sample_name)):
                request_dir = os.path.join(version_dir, sample_name, request_name)
                samples[sample_name][version_name][request_name] = []
                # check that there is only one datetag per requestname
                datetags = os.listdir(request_dir)
                if len(datetags)!=1:
                    msg = 'Found unexpected number of datetags'
                    msg += f' for sample {sample_name} / {version_name} / {request_name}:'
                    msg += f' {datetags}'
                    raise Exception(msg)
                # find files for this sample/version
                rootfiles = glob.glob(os.path.join(request_dir, '*/*/*.root'))
                samples[sample_name][version_name][request_name] = rootfiles

    # remove empty ones
    for sample_name in list(samples.keys())[:]:
        for version_name in list(samples[sample_name].keys())[:]:
            for request_name in list(samples[sample_name][version_name].keys())[:]:
                if len(samples[sample_name][version_name][request_name])==0:
                    samples[sample_name][version_name].pop(request_name)
            if len(samples[sample_name][version_name])==0:
                samples[sample_name].pop(version_name)
        if len(samples[sample_name])==0:
            samples.pop(sample_name)

    # do printouts
    print('Found following samples:')
    for sample_name, versiondict in samples.items():
        print(f'  - {sample_name}')
        for version_name, request_dict in versiondict.items():
            print(f'    - {version_name}')
            for request_name, rootfiles in request_dict.items():
                print(f'      - {request_name}: {len(rootfiles)} files')

    # make new sample dir names and check (non-)existence
    newsamples = []
    exist = []
    for sample_name, versiondict in samples.items():
        for version_name, files in versiondict.items():
            newsample_name = sample_name+'_'+version_name
            newsamples.append(newsample_name)
            newsample_dir = os.path.join(args.inputdir, newsample_name)
            if os.path.exists(newsample_dir): exist.append(newsample_dir)
    if len(exist)>0:
        msg = 'The following new sample dirs already exist:\n'
        msg += ''.join([f'  - {d}\n' for d in exist])
        msg += 'Clean? (y/n)'
        print(msg)
        go = six.moves.input()
        if go!='y': sys.exit()
        for d in exist: os.system(f'rm -r {d}')

    # make new directory for each sample and copy or move all files
    for sample_name, versiondict in samples.items():
        for version_name, request_dict in versiondict.items():
            newsample_name = sample_name+'_'+version_name
            newsample_dir = os.path.join(args.inputdir, newsample_name)
            mkdircmd = f'mkdir -p {newsample_dir}'
            if not args.test: os.system(mkdircmd)
            all_root_files = list(request_dict.values())
            all_root_files = [f for subset in all_root_files for f in subset]
            for idx, root_file in enumerate(all_root_files):
                source = root_file
                target = os.path.join(newsample_dir, f'nanoaod_{idx}.root')
                if args.mode=='mv': mvcmd = f'mv {source} {target}'
                elif args.mode=='cp': mvcmd = f'cp {source} {target}'
                if not args.test: os.system(mvcmd)

    # do printouts
    print('New samples:')
    for sample_name in newsamples:
        sample_dir = os.path.join(args.inputdir, sample_name)
        if os.path.exists(sample_dir):
            rootfiles = [f for f in os.listdir(sample_dir) if f.endswith('.root')]
            print(f'  - {sample_name}: {len(rootfiles)} files')
        else: print(f'  - {sample_name}: does not exist.')
    print('Original directories:')
    for sample_name, versiondict in samples.items():
        print(f'  - {sample_name}')
        for version_name, _ in versiondict.items():
            version_dir = os.path.join(args.inputdir, sample_name, version_name)
            rootfiles = glob.glob(os.path.join(version_dir, '*/*/*/*/*.root'))
            print(f'    - {version_name}: {len(rootfiles)} files')

    # remove original directories
    print('Remove original directories? (y/n)')
    go = six.moves.input()
    if go == 'y':
        for sample_name in samples.keys():
            sample_dir = os.path.join(args.inputdir, sample_name)
            rmcmd = f'rm -r {sample_dir}'
            print(rmcmd)
            if not args.test: os.system(rmcmd)
