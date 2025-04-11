# Global tags

The official global tags for Run-3 MC productions are listed [here](https://twiki.cern.ch/twiki/bin/view/CMSPublic/GTsRun3#Global_tag_for_2023_Run3_MC_prod).

Specifically for NanoAOD production, they might be listed [here](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Releases/NanoAODv12) (or similar pages for other versions).

Another way to retrieve the global tag is to find an already existing sample on DAS for the period of interest and click `Configs`.

In case these methods give unclear or contradictory information, best to check with experts...


## Notes for 2018 comparison study

Offcial global tags listed [here](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Releases/NanoAODv9).
For simulation: `106X_upgrade2018_realistic_v16_L1v1`, for data: `106X_dataRun2_v35`.
This is confirmed by DAS, e.g. [this query](https://cmsweb.cern.ch/das/request?instance=prod/global&input=config+dataset%3D%2FGluGluHToZZTo4L_M125_TuneCP5_13TeV_powheg2_JHUGenV7011_pythia8%2FRunIISummer20UL18NanoAODv9-106X_upgrade2018_realistic_v16_L1v1-v1%2FNANOAODSIM) for simulation
and [this query](https://cmsweb.cern.ch/das/request?instance=prod/global&input=config+dataset%3D%2FEGamma%2FRun2018A-UL2018_MiniAODv2_NanoAODv9-v1%2FNANOAOD) for data.

But downstream in the HcNtuplizer repository, the central NanoAOD datasets are the ones with a `GT36` tag.
They have global tag `106X_dataRun2_v36` (see e.g. [this DAS query](https://cmsweb.cern.ch/das/request?instance=prod/global&input=config+dataset%3D%2FEGamma%2FRun2018A-UL2018_MiniAODv2_NanoAODv9_GT36-v1%2FNANOAOD)),
so switch to that one (presumably matters only very little).
This only applies to data, not to simulation.

Another problem is that the output depends not only on the era and global tag parameters, but also on the CMSSW version.
For example, the central NanoAOD (v9) samples for 2018 (both simulation and data, and regardless of `v35` or `v36`) have a branch named `Electron.mvaFallV2Iso` (which is used in HcNtuplizer).
But with my custom processing, I get `Electron.mvaIso` (which seems to correspond to newer NanoAOD versions, starting from v10), regardless of the specific global tag.
Testing in `CMSSW_10_6_30` proves to be difficult, as one needs a `cmssw-el7` container and weird unintellegible CMSSW config file reading errors pop up (while the same config works perfectly fine in newer CMSSW versions).

Conclusion: process 2018 samples with global tag that matches the central NanoAOD production, but there will still be differences because of the CMSSW version.
Comparison between central and custom NanoAOD will be approximate at best (but still useful).
