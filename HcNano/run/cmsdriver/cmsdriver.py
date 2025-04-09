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

    # check dtype
    if dtype is None:
        msg = 'Argument dtype is required'
        msg += ' to make the cmsDriver command.'
        raise Exception(msg)
    if dtype not in ['mc', 'data']:
       msg = f'Dtype "{dtype}" not recognized.'
       raise Exception(msg)

    # make the cmsDriver command for standard NanoAOD
    cmd = f'cmsDriver.py {configname} --step NANO'
    cmd += f' --{dtype}'
    if conditions is not None: cmd += f' --conditions {conditions}'
    if era is not None: cmd += f' --era {era}'
    if dtype=='mc': cmd += ' --eventcontent NANOAODSIM --datatier NANOAODSIM'
    else: cmd += ' --eventcontent NANOAOD --datatier NANOAOD'
    if no_exec: cmd += ' --no_exec'
    cmd += f' --filein {inputfile}'
    cmd += f' --fileout {outputfile}'
    cmd += f' -n {nentries}'
    
    # do customization
    cmd += f' --customise PhysicsTools/HcNano/hcnano_cff.hcnano_customize_{dtype}'

    # return the cmsDriver command
    return cmd
