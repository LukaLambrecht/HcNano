# CRAB configuration file

# More info: https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile

import os
import sys


# get modifiable parameters from the environment
# (set by the submit script)
dataset = os.environ['CRAB_DATASET']
(_, sample, version, tier) = dataset.split('/')
sample_short = '_'.join(sample.split('_')[:2])
version_short = '_'.join(version.split('_')[:1])
# (first value is just an empty string because dataset starts with /)
unitsPerJob = int(os.environ['CRAB_UNITSPERJOB'])
entriesPerUnit = int(os.environ['CRAB_ENTRIESPERUNIT'])
outputdir = os.environ['CRAB_OUTPUTDIR']
psetName = os.environ['CRAB_PSETNAME']
test = os.environ['CRAB_TEST']
test = (test=='True' or test=='true')

# define a name and a workarea for this CRAB workflow
requestName = sample_short + '_' + version_short
workArea = os.path.join(os.environ['CMSSW_BASE'],
             'src/PhysicsTools/HcNano/crab/crab_logs',
             sample, version)

# define an output directory
# note: the first part should always be /store/user/<username>
outLFNDirBase = '/store/user/' + os.environ['USER']
outLFNDirBase = os.path.join(outLFNDirBase, outputdir)

# printouts for checking
print('Building CRAB config with following parameters:')
print(f'  - dataset: {dataset}')
print(f'  - requestName: {requestName}')
print(f'  - workArea: {workArea}')
print(f'  - psetName: {psetName}')
print(f'  - outLFNDirBase: {outLFNDirBase}')
print(f'  - unitsPerJob: {unitsPerJob}')
print(f'  - entriesPerUnit: {entriesPerUnit}')

# in case of testing, exit here
if test:
    print('Parameter "test" was set to True, so exiting here.')
    sys.exit()

# set CRAB config
from CRABClient.UserUtilities import config

config = config()

# set the CRAB working directory (where log files will appear)
config.General.workArea = workArea
# set the folder within the CRAB working directory for a specific submission
config.General.requestName = requestName
config.General.transferOutputs = True
config.General.transferLogs = False

# set input dataset
config.Data.inputDataset = dataset
# set splitting
config.Data.splitting = "FileBased"
# set the number of units (usually files) per job
config.Data.unitsPerJob = unitsPerJob
# set the output directory
# note that /store/user/<username> is automatically translated by CRAB
# to a physical file location, depending on the storage site.
# for example using site T3_CH_CERNBOX, this path translates to
# /eos/user/<initial>/<username>
config.Data.outLFNDirBase = outLFNDirBase
config.Data.publication = False

# set the plugin name (do not change, only "Analysis" is allowed for this type of job)
config.JobType.pluginName = "Analysis"
# set the config file and its arguments
config.JobType.psetName = psetName
config.JobType.pyCfgParams = [f'{entriesPerUnit}']
# set the requested time limit and memory limit
config.JobType.maxJobRuntimeMin = 1315
config.JobType.maxMemoryMB = 3500
# set the number of requested cores
config.JobType.numCores = 1

# set the storage site
config.Site.storageSite = "T3_CH_CERNBOX"
