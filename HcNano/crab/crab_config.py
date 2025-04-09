# CRAB configuration file

# More info: https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile

# imports
import CRABClient
import os
import sys

# note: need to import CRABClient first in order to run this locally
#       (with python3 crab_config.py instead of crab submit crab_config.py)
#       for testing purposes (only printouts; does not actually run something).
#       see here: https://twiki.cern.ch/twiki/bin/view/CMSPublic/CMSCrabClient#Using_CRABClient_API

# get modifiable parameters from the environment
# (set by the submit script)
dataset = os.environ['CRAB_DATASET']
(_, sample, version, tier) = dataset.split('/')
sample_short = '_'.join(sample.split('_')[:2])
version_short = '_'.join(version.split('_')[:1])
outputdir = os.environ['CRAB_OUTPUTDIR']
psetName = os.environ['CRAB_PSETNAME']
splitting = os.environ['CRAB_SPLITTING']
unitsPerJob = int(os.environ['CRAB_UNITSPERJOB'])
totalUnits = int(os.environ['CRAB_TOTALUNITS'])

# define a name for this CRAB workflow
# (used for CRAB internal bookkeeping)
requestName = sample_short + '_' + version_short

# define a work area for this CRAB workflow
# (where the log files will appear)
# note: the requestName is automatically appended.
workArea = os.path.join(os.environ['CMSSW_BASE'],
             'src/PhysicsTools/HcNano/crab/crab_logs',
             sample, version)

# define an output directory
# note: the first part should always be /store/user/<username>
# note: sample and requestName are automatically appended.
outLFNDirBase = '/store/user/' + os.environ['USER']
outLFNDirBase = os.path.join(outLFNDirBase, outputdir, sample, version)

# printouts for checking
print('Building CRAB config with following parameters:')
print(f'  - dataset: {dataset}')
print(f'  - requestName: {requestName}')
print(f'  - workArea: {workArea}')
print(f'  - outLFNDirBase: {outLFNDirBase}')
print(f'  - psetName: {psetName}')
print(f'  - splitting: {splitting}')
print(f'  - unitsPerJob: {unitsPerJob}')
print(f'  - totalUnits: {totalUnits}')

# set CRAB config
from CRABClient.UserUtilities import config

config = config()

# set the CRAB working directory and workflow name
config.General.workArea = workArea
config.General.requestName = requestName
config.General.transferOutputs = True
config.General.transferLogs = False

# set input dataset
config.Data.inputDataset = dataset
# set splitting parameters
# see here for more info:
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/CRAB3ConfigurationFile
config.Data.splitting = splitting
config.Data.unitsPerJob = unitsPerJob
config.Data.totalUnits = totalUnits
# set the output directory
# note that /store/user/<username> is automatically translated by CRAB
# to a physical file location, depending on the storage site.
# for example using site T3_CH_CERNBOX, this path translates to
# /eos/user/<initial>/<username>
config.Data.outLFNDirBase = outLFNDirBase
config.Data.publication = False

# set the plugin name (do not change, only "Analysis" is allowed for this type of job)
config.JobType.pluginName = "Analysis"
# set the config file
config.JobType.psetName = psetName
# set the requested time limit and memory limit
config.JobType.maxJobRuntimeMin = 1315
config.JobType.maxMemoryMB = 2500
# set the number of requested cores
config.JobType.numCores = 1

# set the storage site
config.Site.storageSite = "T3_CH_CERNBOX"
