# General instructions

It is not per se needed to download files to a locally accessible location,
as `cmsrun.py` (see [here](https://github.com/LukaLambrecht/HcNano/tree/main/HcNano/run))
can handle local as well as remote files as its input.

But in some cases, it might be convenient to run on a local file
(e.g. for speed or to rule out some network issue).
In that case, one can download any file retrievable from DAS to a local location,
using the following command:

```
xrdcp root://cms-xrd-global.cern.ch/<path to file on DAS>`
```

Note: this needs a proxy to work. Create one with `voms-proxy-init --voms cms`.

More info on this command can be found on [this TWiki](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookXrootdService)


# Early Run 3 samples

### Signal samples (H+c, H -> gamma gamma)
Use this DAS [query](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2F*HPlusCharm*%2F*Run3*%2FMINIAODSIM).

Note: these samples are H+c, with H decaying to a pair of photons.
The analogous samples with H decaying to 4 leptons do not seem to have been produced yet for Run 3, only for Run 2.


### Background samples
- ggH, 2022-postEE: [DAS](https://cmsweb.cern.ch/das/request?input=dataset%3D%2FGluGluHToGG_M-125_TuneCP5_13p6TeV_powheg-pythia8%2FRun3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2%2FMINIAODSIM&instance=prod/global)


# Case study on 2018 simulation and data
To test the analysis strategy with H decaying to four leptons,
we use 2018 simulation and data, as for Run 3 only samples with H decaying to a pair of photons have been produced.

### Signal sample (H+c, H -> 4L)
Use this DAS [query](https://cmsweb.cern.ch/das/request?instance=prod/global&input=file+dataset%3D%2FHPlusCharm_4FS_MuRFScaleDynX0p50_HToZZTo4L_M125_TuneCP5_13TeV_amcatnloFXFX_JHUGenV7011_pythia8%2FRunIISummer20UL18MiniAODv2-106X_upgrade2018_realistic_v16_L1v1-v2%2FMINIAODSIM)

### Data
For example: [DoubleMuon, Run2018D](https://cmsweb.cern.ch/das/request?instance=prod/global&input=file+dataset%3D%2FDoubleMuon%2FRun2018D-UL2018_MiniAODv2-v1%2FMINIAOD)
