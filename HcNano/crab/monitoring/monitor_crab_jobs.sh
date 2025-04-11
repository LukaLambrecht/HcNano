#!/bin/bash

# run by adding something like this in the acrontab file:
# 0 */4 * * * lxplus.cern.ch cd <path to here>; cmsenv; bash monitor_crab_jobs.sh >> <some log file> 2>&1


# print date and time for bookkeeping
echo "Running monitor_crab_jobs.sh"
date

# this seems to be needed for scram and crab commands to be available
source /cvmfs/cms.cern.ch/cmsset_default.sh

# copy proxy
cp x509up_u116295 /tmp/

# run the monitoring
python3 monitor_crab_jobs.py \
-i ../crab_logs \
-d /eos/user/l/llambrec/hcanalysis_nanoaod_2018_sim_new/monitor_crab_jobs/index.html \
-r
