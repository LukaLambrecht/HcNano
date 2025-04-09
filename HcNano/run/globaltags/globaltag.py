# Functions for retrieving the global tag from a json file


import os
import sys
import json


def get_globaltag(gtfile, year=None, dtype=None):
    '''
    Get the global tag
    Input arguments:
    - gtfile: json file with global tag definitions
    - year: data-taking year to get the global tag for
    - dtype: data type; either "mc" or "data"
    '''

    # check arguments
    if year is None: raise Exception('Data-taking year must be provided.')
    if dtype is None: raise Exception('Data type must be provided.')

    # file existence check
    if not os.path.exists(gtfile):
        msg = f'Global tag json file {gtfile} does not exist.'
        raise Exception(msg)

    # read json file
    with open(gtfile, 'r') as f:
        gtdict = json.load(f)
    
    # get requested year
    globaltag = gtdict[year][dtype].get("conditions", None)
    era = gtdict[year][dtype].get("era", None)
    return {'globaltag': globaltag, 'era': era}
