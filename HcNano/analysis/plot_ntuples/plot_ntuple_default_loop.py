import os
import sys
import argparse


if __name__=='__main__':

    # read command line args
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--inputfiles', required=True, nargs='+')
    parser.add_argument('-o', '--outputdir', required=True)
    parser.add_argument('--weighted', default=False, action='store_true')
    parser.add_argument('--xsec', default=0, type=float)
    parser.add_argument('--lumi', default=0, type=float)
    args = parser.parse_args()

    # make configurations
    configs = {
      'event': {
        'variables': 'variables/variables_event.json',
        'genmatchbranch': None,
        'chargebranch1': None,
        'chargebranch2': None,
        'outputdir': 'event',
        'extrainfo': None
      },
      'event-gen': {
        'variables': 'variables/variables_event_gen.json',
        'genmatchbranch': None,
        'chargebranch1': None,
        'chargebranch2': None,
        'outputdir': 'event',
        'extrainfo': None
      },
      'Ds': {
        'variables': 'variables/variables_ds_selection.json',
        'genmatchbranch': 'DsMeson_hasFastGenmatch',
        'chargebranch1': 'DsMeson_KPlus_charge',
        'chargebranch2': 'DsMeson_KMinus_charge',
        'outputdir': 'Ds',
        'extrainfo': r"'$D_{s} \rightarrow \phi \pi \rightarrow K K \pi$'"
      },
      'Ds-gen': {
        'variables': 'variables/variables_ds_gen.json',
        'genmatchbranch': None,
        'chargebranch1': None,
        'chargebranch2': None,
        'outputdir': 'Ds',
        'extrainfo': r"'$D_{s} \rightarrow \phi \pi \rightarrow K K \pi$'"
      },
      'DStar': {
        'variables': 'variables/variables_dstar_selection.json',
        'genmatchbranch': 'DStarMeson_hasFastGenmatch',
        'chargebranch1': 'DStarMeson_Pi2_charge',
        'chargebranch2': 'DStarMeson_K_charge',
        'outputdir': 'DStar',
        'extrainfo': r"'$D* \rightarrow D^{0} \pi \rightarrow K \pi \pi$'"
      },
      'DStar-gen': {
        'variables': 'variables/variables_dstar_gen.json',
        'genmatchbranch': None,
        'chargebranch1': None,
        'chargebranch2': None,
        'outputdir': 'DStar',
        'extrainfo': r"'$D* \rightarrow D^{0} \pi \rightarrow K \pi \pi$'"
      }
    }

    # make lumi header in case of weighted events
    lumiheader = None
    if args.weighted:
        lumiheader = r"'$\sigma$ = " + "{:.1e}, ".format(args.xsec)
        lumiheader += r"$L$ = " + "{:.1e}'".format(args.lumi)

    for key, config in configs.items():

        variables = config['variables']
        genmatchbranch = config['genmatchbranch']
        chargebranch1 = config['chargebranch1']
        chargebranch2 = config['chargebranch2']
        thisoutputdir = os.path.join(args.outputdir, config['outputdir'])
        extrainfo = config['extrainfo']

        cmd = 'python3 plot_ntuple.py'
        cmd += ' -i {}'.format(' '.join(args.inputfiles))
        cmd += f' -v {variables}'
        cmd += f' -o {thisoutputdir}'
        cmd += ' --donormalize --dolog'
        if extrainfo is not None: cmd += f' --extrainfo {extrainfo}'
        if genmatchbranch is not None: cmd += f' --genmatchbranch {genmatchbranch}'
        if chargebranch1 is not None: cmd += f' --chargebranch1 {chargebranch1}'
        if chargebranch2 is not None: cmd += f' --chargebranch2 {chargebranch2}'
        cmd += ' --extracmstext Simulation'
        if args.weighted:
            cmd += ' --weighted'
            cmd += f' --xsec {args.xsec}'
            cmd += f' --lumi {args.lumi}'
            cmd += f' --lumiheader {lumiheader}'

        print(cmd)
        os.system(cmd)
