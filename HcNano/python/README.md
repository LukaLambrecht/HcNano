# Python config files for CMSSW

The file `hcnano_cff.py` contains the configuration fragment that customizes the standard NanoAOD production.
Functionality includes:
- Adding more branches by running custom EDProducers.
- Removing unneeded branches from the output.
- Applying event selection by running custom EDFilters and only storing selected events in the output.
