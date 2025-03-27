# Functions for retrieving the global tag from a json file


import os
import sys
import json


def get_globaltag(gtfile, year):
    '''
    Get the global tag
    Input arguments:
    - gtfile: json file with global tag definitions
    - year: data-taking year to get the global tag for
    '''

    # file existence check
    if not os.path.exists(gtfile):
        msg = f'Global tag json file {gtfile} does not exist.'
        raise Exception(msg)

    # read json file
    with open(gtfile, 'r') as f:
        gtdict = json.load(f)
    
    # get requested year
    return gtdict[year]["gt"]
