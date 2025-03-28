# Run the NanoAOD production

### Quick tests
For quick testing on locally available files, use `python3 cmsrun.py`.
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
Also make sure to have recompiled the plugins (using `scramv1 b` in the `HcNano` directory) if there were any modifications.

Note: while the exact `CMSSW` version, era, and global tag are not important for the custom charmed meson analyzers,
using the wrong one(s) might make the rest of the NanoAOD production fail or produce invalid output.
See the [main README](https://github.com/LukaLambrecht/HcNano/blob/main/README.md) for some more information.

### Running with HTCondor
Use `python3 submit_condor.py`.
Run with the option `-h` to see a list of all available options.
They are mostly similar to the options for `cmsrun.py`. Some noteworthy differences/extensions:
- samplelist: samplelist (in simple `.txt` format) listing the datasets to process. Each dataset can be either remote (use the dataset name as shown on DAS), or locally accessible (use the path to the directory containing the `.root` files).
- proxy: provide the full path to a valid proxy (created with `voms-proxy-init --voms cms` and copied to some non-temporary directory); needed for remote file finding and reading, but not for running on locally accessible files.

### Running with CRAB
For submitting full datasets with CRAB: see [here](https://github.com/LukaLambrecht/HcNano/tree/main/HcNano/crab).
