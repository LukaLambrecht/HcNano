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
        no_exec = False,
        year = None):

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
    # note: syntax seems more complicated than needed,
    #       but this seems to be required because of CMSSW quirks...
    customize_commands = []
    if dtype is not None: customize_commands.append(f'process.__dict__[\'dtype\'] = \'{dtype}\'')
    if year is not None: customize_commands.append(f'process.__dict__[\'year\'] = \'{year}\'')
    customize_commands.append('from PhysicsTools.HcNano.hcnano_cff import hcnano_customize')
    customize_commands.append('process = hcnano_customize(process)')
    cmd += ' --customise_commands="{}"'.format('; '.join(customize_commands))

    # return the cmsDriver command
    return cmd
