# Run plot_ntuple_loop.py on a number of predefined ntuples.

import os
import sys

if __name__=='__main__':

    ntuples = [
      {
        'file': '/eos/user/l/llambrec/hcanalysis_nanoaod_2022_test/HPlusCharm_4FS_MuRFScaleDynX0p50_HTo2G_M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2/output_batch*.root',
        'xsec': 0.00009013 * 1000, # cross-section for 2018 H->4l sample, see ntuplizer
        'lumi': 61.75,
        'outputdir': 'output_test_Hc'
      },
      {
        'file': '/eos/user/l/llambrec/hcanalysis_nanoaod_2022_test/HPlusBottom_5FS_MuRFScaleDynX0p50_HTo2G_M-125_TuneCP5_13p6TeV_amcatnloFXFX-pythia8_Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2/output_batch*.root',
        'xsec': 0.0006654 * 1000, # cross-section for 2018 H->4l sample, see ntuplizer
        'lumi': 61.75,
        'outputdir': 'output_test_Hb'
      },
      {
        'file': '/eos/user/l/llambrec/hcanalysis_nanoaod_2022_test/GluGluHtoZZto4L_M-125_TuneCP5_13p6TeV_powheg2-JHUGenV752-pythia8_Run3Summer22EEMiniAODv4-130X_mcRun3_2022_realistic_postEE_v6-v2/output.root',
        'xsec': 0.01334 * 1000, # cross-section for 2018 H->4l sample, see ntuplizer
        'lumi': 61.75,
        'outputdir': 'output_test_ggH'
      }
    ]

    cmds = []
    for ntuple in ntuples:
        cmd = 'python3 plot_ntuple_loop.py'
        cmd += f' -i {ntuple["file"]}'
        cmd += f' -o {ntuple["outputdir"]}'
        if 'xsec' in ntuple.keys() and 'lumi' in ntuple.keys():
            cmd += ' --weighted'
            cmd += f' --xsec {ntuple["xsec"]}'
            cmd += f' --lumi {ntuple["lumi"]}'
        cmds.append(cmd)

    for cmd in cmds:
        print(cmd)
        os.system(cmd)
