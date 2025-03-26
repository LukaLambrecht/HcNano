# General command from here:
# https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production
# Era from here:
# https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Releases/NanoAODv12
# GT not mentioned on the page above...
# instead take it from the HPlusCharm_4FS sample on DAS
# Customization from here:
# https://github.com/hqucms/NanoTuples/tree/production/master

import os
import sys

def make_cmsdriver_command(inputfile,
        nentries = -1,
        outputfile = 'output.root',
        conditions = None,
        era = None,
        dtype = None,
        no_exec = False):
    # make the cmsDriver command
    cmd = 'cmsDriver.py config --step NANO'
    if dtype is not None: cmd += f' --{dtype}'
    if conditions is not None: cmd += f' --conditions {conditions}'
    if era is not None: cmd += f' --era {era}'
    cmd += ' --eventcontent NANOAODSIM --datatier NANOAODSIM'
    if no_exec: cmd += ' --no_exec'
    cmd += ' --customise PhysicsTools/HcNano/hcnano_cff.hcnano_customize'
    cmd += f' --filein {inputfile}'
    cmd += f' --fileout {outputfile}'
    cmd += f' -n {nentries}'
    return cmd

if __name__=='__main__':

    # read command line args
    inputfile = sys.argv[1]
    nentries = sys.argv[2]
    outputfile = sys.argv[3]

    # other arguments (hard-coded for now)
    conditions = '133X_mcRun3_2022_realistic_postEE_ForNanov13_v1'
    era = 'Run3'
    dtype = 'mc'
    no_exec = False

    # parse input file
    if inputfile.startswith('root://'): pass
    elif inputfile.startswith('/store/'):
        inputfile = f'root://cms-xrd-global.cern.ch//{inputfile}'
    else:
        inputfile = os.path.abspath(inputfile)
        inputfile = f'file:{inputfile}'
    print(f'Using parsed input file name: {inputfile}')

    # parse number of entries to process
    # (note: use -1 to process all events in the input file)
    nentries = int(nentries)

    # make the cmsDriver command
    cmd = make_cmsdriver_command(inputfile, nentries=nentries, outputfile=outputfile,
            conditions=conditions, era=era, dtype=dtype,
            no_exec=no_exec)

    # run the cmsDriver command
    os.system(cmd)
