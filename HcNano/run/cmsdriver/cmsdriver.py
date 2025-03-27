# Functions for making the correct cmsDriver command for NanoAOD production

# References:
# General command from here:
# https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production
# Customization similar to this example:
# https://github.com/hqucms/NanoTuples/tree/production/master


def make_nano_cmsdriver(inputfile,
        configname = 'config',
        nentries = -1,
        outputfile = 'output.root',
        conditions = None,
        era = None,
        dtype = None,
        no_exec = False):

    # make the cmsDriver command for standard NanoAOD
    cmd = f'cmsDriver.py {configname} --step NANO'
    if dtype is not None: cmd += f' --{dtype}'
    if conditions is not None: cmd += f' --conditions {conditions}'
    if era is not None: cmd += f' --era {era}'
    cmd += ' --eventcontent NANOAODSIM --datatier NANOAODSIM'
    if no_exec: cmd += ' --no_exec'
    cmd += f' --filein {inputfile}'
    cmd += f' --fileout {outputfile}'
    cmd += f' -n {nentries}'
    
    # do customization
    cmd += ' --customise PhysicsTools/HcNano/hcnano_cff.hcnano_customize'

    # return the cmsDriver command
    return cmd
