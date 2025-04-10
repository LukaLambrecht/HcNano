# Custom NanoAOD producer for Hc analysis

This repository contains some NanoAOD customizations for an analysis of H boson production in association with charmed mesons.
Several types of charmed mesons (including the Ds and D\*) are reconstructed from pairs and triplets of individual tracks,
and the resulting branches are added on top of default NanoAOD.

This has the advantage that the remainder of the analysis can be carried out like a NanoAOD-based analysis
(including profiting from all corrections and calibrations that are either applied by default or otherwise easily available),
while still utilizing the more low-level (MiniAOD) features needed for individual meson reconstruction from tracks.

### How to set up
Download and run the installation script as follows:

```
wget https://raw.githubusercontent.com/LukaLambrecht/HcNano/refs/heads/main/setup.sh
bash setup.sh
```

This will set up the NanoAOD customizer with the following structure (as conventional for CMSSW modules):
`CMSSW_<version>/src/PhysicsTools/HcNano`.

The CMSSW version is currently hard-coded in the setup script.
In case another CMSSW version is needed, you can edit the setup script after downloading it but before running it.
This might be extended in the future to pick the correct CMSSW version based on some criteria.

### How to obtain a test file for quick testing and development
See instructions [here](https://github.com/LukaLambrecht/HcNano/blob/main/HcNano/testfiles/README.md).

### How to run
For quick testing on locally available files, go to `$CMSSW_BASE/src/PhysicsTools/HcNano/run` and use `python3 cmsrun.py`.
Run with the option `-h` to see a list of all available options.
At the time of writing, they are:
- inputfile: input `.root` file in MiniAOD format.
- nentries: number of entries to process (default: all entries in the input file)
- outputfile: name of the output `.root` file in NanoAOD format to produce.
- configname: name of the cmsRun config file to produce.
- dtype: argument to cmsDriver, use `mc` for simulation and `data` for data.
- era: argument to cmsDriver, usually just `Run3` for Run 3 conditions, more info below.
- globaltag: argument to cmsDriver, more info below. Can be either a valid `conditions` name or the path to a json file holding the correct global tags per year. See the `globaltags` subdirectory for some examples on correct formatting.
- year: data-taking year, used to extract the correct global tag in case a json file was provided above.
- no_exec: argument to cmsDriver. If specified, the cmsRun config file will be produced but not run.

Note: make sure to have done `cmsenv` in the CMSSW `src` directory containing the NanoAOD producer before running.

Note: while the exact `CMSSW` version, era, and global tag are not important for the custom charmed meson analyzers,
using the wrong one(s) might make the rest of the NanoAOD production fail or produce invalid output.

For running on multiple files in HTCondor job submission, see [here](https://github.com/LukaLambrecht/HcNano/tree/main/HcNano/run).
For submitting full datasets with CRAB: see [here](https://github.com/LukaLambrecht/HcNano/tree/main/HcNano/crab).

### Overview of how it works
The basics for producing custom NanoAOD as done here consist of the following steps:
- Build the correct cmsDriver command to produce central NanoAOD, as explained [here](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production). In this repository, the cmsDriver command is constructed [here](https://github.com/LukaLambrecht/HcNano/blob/main/HcNano/run/cmsdriver/cmsdriver.py).
- Modify the cmsDriver command with an additional argument of the form `--customise <some custom config file>.<some customization function>`. The customization function is defined [here](https://github.com/LukaLambrecht/HcNano/blob/f9a2ab225ea5553307351d787f7efd734038397b/HcNano/python/hcnano_cff.py#L111). An example of how to add a custom EDProducer to add additional output branches to the NanoAOD is [here](https://github.com/LukaLambrecht/HcNano/blob/d294725d481fa7be4343512d28b4347dc25720b4/HcNano/python/hcnano_cff.py#L5).
- The actual EDProducers to be run are defined [here](https://github.com/LukaLambrecht/HcNano/tree/main/HcNano/plugins).
- Apart from EDProducers, one can also add EDFilters to reduce the size of the output files by skipping events that do not meet some selection requirement. See an example EDFilter [here](https://github.com/LukaLambrecht/HcNano/blob/main/HcNano/plugins/NLeptonSelector.cc) and its configuration [here](https://github.com/LukaLambrecht/HcNano/blob/f9a2ab225ea5553307351d787f7efd734038397b/HcNano/python/hcnano_cff.py#L137).

### Notes on CMSSW version, eras, and global tags
It is not always very clear which combination of CMSSW version, era, and global tag to use in order to make the NanoAOD production run (and ideally produce the correct output).
It's mostly a matter of consulting experts and/or trial and error, but some guidelines and further references are given [here](https://github.com/LukaLambrecht/HcNano/tree/main/HcNano/run/globaltags).

### How to make modifications
Modify the producers in the `PhysicsTools/HcNano/plugins` directory.
If needed, also edit the corresponding headers in the `PhysicsTools/HcNano/interface` directory.
Then recompile with `scramv1 b`.

For modifications in which producers are being run,
the configuration file `PhysicsTools/Hcnano/python/hcnano_cff.py` should be modified accordingly.

Note: make sure to have done `cmsenv` in the CMSSW `src` directory containing the ntuplizer before recompiling.

### Current status
Correctly produces NanoAOD files with the required additional branches.
Additional branches are fully synchronized with an [earlier standalone analyzer](https://github.com/LukaLambrecht/HcAnalysis).
Also the skimming seems to work well, only selected events are written to the Events tree,
while at the same time the event counters in the Runs tree contain all events (before selection), as needed for normalization.
Verified to work both on simulation and data.

Known issues and things to do:
- Optimize storage space by removing unneeded branches from the NanoAOD output (partly done but to be optimized).
- Find correct global tag to use for different eras of interest.

### References
- Technical NanoAOD documentation [here](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Releases/NanoAODv12).
  - Especially instructions for [v12](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Releases/NanoAODv12).
  - And instructions for [private production](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production).
- Another example of custom NanoAOD [here](https://github.com/hqucms/NanoTuples/tree/production/master).
- Earlier standalone analyzer [here](https://github.com/LukaLambrecht/HcAnalysis).

For further analysis starting from NanoAOD files, see the following (just a few examples):
- For this analysis, the next step is probably going to be done with the [HcNtuplizer](https://github.com/LukaLambrecht/HcNtuplizer).
- General NanoAOD documentation on [this twiki page](https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookNanoAOD) and in [this tutorial](https://opendata.cern.ch/docs/cms-getting-started-nanoaod).
- Uproot documentation [here](https://uproot.readthedocs.io/en/latest/basic.html).
- Coffea tutorial [here](https://github.com/CoffeaTeam/coffea-casa-tutorials/blob/master/analyses/thq/analysis_tutorial.ipynb).
