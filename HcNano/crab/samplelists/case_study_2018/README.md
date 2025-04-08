# Samples for 2018 case study

Motivation: the HPlusCharm samples with H decaying to 4L seem to be currently only available for Run II (UL).
See for example [this DAS query](https://cmsweb.cern.ch/das/request?view=list&limit=50&instance=prod%2Fglobal&input=dataset%3D%2FHPlusCharm_4FS_*_*HToZZTo4L*%2F*%2FMINIAODSIM): `dataset=/HPlusCharm_4FS_*_*HToZZTo4L*/*/MINIAODSIM`.

So we have three options:
- Switch to H decaying to two photons (samples for 2022 + 2023 available).
- Generate new samples.
- Work with Run II UL.

Starting a new Run II analysis is probably not a good idea,
but at least we can do a case study with 2018 data and simulation
to test the workflow of custom NanoAOD production and the subsequent ntuplizing and analysis,
without having to generate new samples right away.
