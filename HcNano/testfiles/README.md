# Copy files from DAS to local

Use the following command: `xrdcp root://cms-xrd-global.cern.ch/<path to file on DAS>`.
Note: this needs a proxy to work. Create one with `voms-proxy-init --voms cms`.

More info on [this TWiki](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookXrootdService)


### Signal samples

Use this DAS [query](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2F*HPlusCharm*%2F*Run3*%2FMINIAODSIM).


### Background samples

ggH
- 2022-preEE: [das](https://cmsweb.cern.ch/das/request?input=dataset%3D%2FGluGluHToGG_M-125_TuneCP5_13p6TeV_powheg-pythia8%2FRun3Summer22MiniAODv4-BSzpz35_130X_mcRun3_2022_realistic_v5-v2%2FMINIAODSIM&instance=prod/global)
- 2022-postEE: [das](https://cmsweb.cern.ch/das/request?input=dataset%3D%2FGluGluHToGG_M-125_TuneCP5_13p6TeV_powheg-pythia8%2FRun3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2%2FMINIAODSIM&instance=prod/global)
- 2023-preBPIX: ?
- 2023-postBPIX: [das](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2FGluGluHToGG_M-125_TuneCP5_13p6TeV_powheg-pythia8%2FRun3Summer23*%2FMINIAODSIM)


# Case study on 2018 simulation and data

No need to copy files, just read them remotely.

### Signal sample (H+c, H->4L)
Use this DAS [query](https://cmsweb.cern.ch/das/request?instance=prod/global&input=file+dataset%3D%2FHPlusCharm_4FS_MuRFScaleDynX0p50_HToZZTo4L_M125_TuneCP5_13TeV_amcatnloFXFX_JHUGenV7011_pythia8%2FRunIISummer20UL18MiniAODv2-106X_upgrade2018_realistic_v16_L1v1-v2%2FMINIAODSIM)

### Data
For example: [DoubleMuon, Run2018D](https://cmsweb.cern.ch/das/request?instance=prod/global&input=file+dataset%3D%2FDoubleMuon%2FRun2018D-UL2018_MiniAODv2-v1%2FMINIAOD)
