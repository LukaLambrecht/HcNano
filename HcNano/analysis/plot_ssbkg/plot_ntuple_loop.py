import os
import sys
import argparse


if __name__=='__main__':

    # read command line args
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfiles', required=True, nargs='+')
    parser.add_argument('-o', '--outputdir', required=True)
    args = parser.parse_args()

    configs = {
      'Ds': {
        'variables': 'variables/variables_ds.json',
        'genmatchbranch': 'DsMeson_hasFastGenmatch',
        'chargebranch1': 'DsMeson_KPlus_charge',
        'chargebranch2': 'DsMeson_KMinus_charge',
        'outputdir': 'Ds',
        'extrainfo': r"'$D_{s} \rightarrow \phi \pi \rightarrow K K \pi$'"
      },
      'DStar': {
        'variables': 'variables/variables_dstar.json',
        'genmatchbranch': 'DStarMeson_hasFastGenmatch',
        'chargebranch1': 'DStarMeson_K_charge',
        'chargebranch2': 'DStarMeson_Pi2_charge',
        'outputdir': 'DStar',
        'extrainfo': r"'$D* \rightarrow D^{0} \pi \rightarrow K \pi \pi$'"
      },
    }

    for key, config in configs.items():

        variables = config['variables']
        thisoutputdir = os.path.join(args.outputdir, config['outputdir'])
        extrainfo = config['extrainfo']

        cmd = 'python3 plot_ntuple.py'
        cmd += ' -i {}'.format(' '.join(args.inputfiles))
        cmd += f' -v {variables}'
        cmd += f' -o {thisoutputdir}'
        cmd += ' --donormalize --dolog'
        if extrainfo is not None: cmd += f' --extrainfo {extrainfo}'
        cmd += f' --genmatchbranch {config["genmatchbranch"]}'
        cmd += f' --chargebranch1 {config["chargebranch1"]}'
        cmd += f' --chargebranch2 {config["chargebranch2"]}'
        cmd += ' --extracmstext Simulation'

        print(cmd)
        os.system(cmd)
