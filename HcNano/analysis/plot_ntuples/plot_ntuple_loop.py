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
      'event': {
        'variables': 'variables/variables_event.json',
        'genmatchbranch': None,
        'outputdir': 'event',
        'extrainfo': None
      },
      'event-gen': {
        'variables': 'variables/variables_event_gen.json',
        'genmatchbranch': None,
        'outputdir': 'event',
        'extrainfo': None
      },
      'Ds': {
        'variables': 'variables/variables_ds_selection.json',
        'genmatchbranch': 'DsMeson_hasFastGenmatch',
        'outputdir': 'Ds',
        'extrainfo': r"'$D_{s} \rightarrow \phi \pi \rightarrow K K \pi$'"
      },
      'Ds-gen': {
        'variables': 'variables/variables_ds_gen.json',
        'genmatchbranch': None,
        'outputdir': 'Ds',
        'extrainfo': r"'$D_{s} \rightarrow \phi \pi \rightarrow K K \pi$'"
      },
      'DStar': {
        'variables': 'variables/variables_dstar_selection.json',
        'genmatchbranch': 'DStarMeson_hasFastGenmatch',
        'outputdir': 'DStar',
        'extrainfo': r"'$D* \rightarrow D^{0} \pi \rightarrow K \pi \pi$'"
      },
      'DStar-gen': {
        'variables': 'variables/variables_dstar_gen.json',
        'genmatchbranch': None,
        'outputdir': 'DStar',
        'extrainfo': r"'$D* \rightarrow D^{0} \pi \rightarrow K \pi \pi$'"
      }
    }

    for key, config in configs.items():

        variables = config['variables']
        genmatchbranch = config['genmatchbranch']
        thisoutputdir = os.path.join(args.outputdir, config['outputdir'])
        extrainfo = config['extrainfo']

        cmd = 'python3 plot_ntuple.py'
        cmd += ' -i {}'.format(' '.join(args.inputfiles))
        cmd += f' -v {variables}'
        cmd += f' -o {thisoutputdir}'
        cmd += ' --donormalize --dolog'
        if extrainfo is not None: cmd += f' --extrainfo {extrainfo}'
        if genmatchbranch is not None: cmd += f' --genmatchbranch {genmatchbranch}'
        cmd += ' --extracmstext Simulation'

        print(cmd)
        os.system(cmd)
