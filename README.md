# Custom NanoAOD producer for Hc analysis

This repository contains some NanoAOD customizations for an analysis of H boson production in association with charmed mesons.
Several types of charmed mesons (including the Ds and D\*) are reconstructed from pairs and triplets of individual tracks,
and the resulting branches are added on top of default NanoAOD.

### How to setup
Download and run the installation script as follows:

```
wget https://raw.githubusercontent.com/LukaLambrecht/HcNano/refs/heads/main/setup.sh
bash setup.sh
```

### How to obtain a test file for quick testing and development
See instructions [here](https://github.com/LukaLambrecht/HcNano/blob/main/HcNano/testfiles/README.md).

### How to run
For quick testing on locally available files, go to `$CMSSW_BASE/src/PhysicsTools/HcNano/run` and run the following command:
```
python3 cmsrun.py <path to input file> <number of entries> <name of output file>
```
where both the input file and output file should be `.root` files,
and the number of entries is an integer number of entries to process
(use -1 to process all entries in the provided input file).
The input file should be in MiniAOD format, while the output file is in NanoAOD format.

Note: make sure to have done `cmsenv` in the CMSSW `src` directory containing the ntuplizer before running.

Note: you might need to modify the `cmsDriver` command (created inside `cmsrun.py`),
depending on the `CMSSW` version, era, and global tag that were used to produce the input MiniAOD file.
While the exact `CMSSW` version, era, and global tag are not important for the custom charmed meson analyzers,
using the wrong one(s) might make the rest of the NanoAOD production fail.

Running on multiple files, submitting HTCondor jobs, and submitting CRAB tasks: to be implemented.

### How to make modifications
Modify the producers in the `PhysicsTools/HcNano/plugins` directory.
If needed, also edit the corresponding headers in the `PhysicsTools/HcNano/interface` directory.
Then recompile with `scramv1 b`.

For modifications in which producers are being run,
the configuration file `PhysicsTools/Hcnano/python/hcnano_cff.py` should be modified accordingly.

Note: make sure to have done `cmsenv` in the CMSSW `src` directory containing the ntuplizer before recompiling.

### Current status
Correctly produces NanoAOD files with the required additional branches.
Additional branches are fully synchronized with [earlier standalone analyzer](https://github.com/LukaLambrecht/HcAnalysis).

Known issues and things to do:
- Remove unneeded output to save disk space (currently the NanoAOD files are O(10) times larger than earlier standalone ntuples).
- Might not work on data, need to implement some switches.
- Find correct global tag to use for different eras of interest.
- Implement HTCondor and CRAB submission scripts.

### References
- General NanoAOD documentation [here](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Releases/NanoAODv12).
  - Especially instructions for [v12](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Releases/NanoAODv12).
  - And instructions for [private production](https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production).
- Another example of custom NanoAOD [here](https://github.com/hqucms/NanoTuples/tree/production/master).
- Earlier standalone analyzer [here](https://github.com/LukaLambrecht/HcAnalysis)
