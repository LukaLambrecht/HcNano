#####################################
# Tools for handling input datasets #
#####################################


import os
import sys


def get_files( datasetname,
               location=None,
               runnb=None,
               privateprod=False,
               redirector='root://cms-xrd-global.cern.ch/',
               maxfiles=None,
               verbose=True ):
  ### get a list of input files from a dataset name
  # input arguments:
  # - datasetname: can be one of the following:
  #   - name of a dataset on DAS.
  #   - name of a single file on DAS.
  #     (warning: no check is done to verify that the file actually exists on DAS).
  #   - name of a locally accessible directory with root files.
  #   - name of a locally accessible single root file.
  #   - list of any of the above.
  # - location: choose from 'das', 'local' or None, with the following behaviour:
  #   - 'das': use a DAS query to find the files in a dataset on DAS,
  #            and prefix the retrieved file names using the specified redirector.
  #   - 'local': get all root files in the specified folder on the local filesystem.
  #   - None: choose automatically between 'das' and 'local' via a simple os.path.exists check.
  # - runnb: select only files for the given run number (ignored for local files).
  # - privateprod: whether the dataset on DAS was privately produced (ignored for local files).
  # - redirector: redirector used to prefix remote files (ignored for local files).
  # - maxfiles: return only specified number of first files.
  # - verbose: print information.
  # output:
  #   a list of (formatted) file paths.
  # note: the output is always a list, even if the input was just a single file.
  # note: the DAS client requires a valid proxy to run.

  # check if the specified dataset is actually a list
  # and if so, run recursively on each element in the list
  if isinstance(datasetname, list):
    filenames = []
    for name in datasetname:
      filenames += get_files(name,
                      location=location,
                      runnb=runnb,
                      privateprod=privateprod,
                      redirector=redirector,
                      maxfiles=maxfiles,
                      verbose=verbose)
    return filenames

  # strip spurious characters from dataset name
  datasetname = datasetname.strip(' \t\n,;')
  
  # determine whether dataset is locally accessible or not
  if location is None:
    if os.path.exists(datasetname):
      location = 'local'
      if verbose: print('Dataset {} was found locally.'.format(datasetname))
    else:
      location = 'das'
      if verbose: print('Dataset {} was not found locally, will use DAS.'.format(datasetname))

  # handle the case of a single file (either locally or on DAS)
  if datasetname.endswith('.root'):
    if location=='das':
      redirector = redirector.rstrip('/')+'/'
      filenames = [redirector+datasetname]
    elif location=='local':
      filenames = [os.path.abspath(datasetname)]
    else:
      raise Exception('ERROR: location "{}" not recognized'.format(location))
    if verbose:
      print('Dataset {} is interpreted as a single root file.'.format(datasetname))
      print('Using following fromatting: {}'.format(filenames))
    return filenames

  # make a list of input files based on provided directory or dataset name
  if location=='das':
      # make the DAS client command
      dasquery = 'file dataset={}'.format(datasetname)
      if privateprod: dasquery += ' instance=prod/phys03'
      if runnb is not None: dasquery += ' run={}'.format(runnb)
      if verbose: print('Running DAS client with following query: {}...'.format(dasquery))
      dascmd = "dasgoclient -query '{}' --limit 0".format(dasquery)
      # run the DAS client command
      dasstdout = os.popen(dascmd).read().strip(' \t\n')
      # check for DAS errors
      if 'X509_USER_PROXY' in dasstdout:
        msg = 'ERROR: DAS returned a proxy error:\n'+dasstdout
        raise Exception(msg)
      # format the files
      dasfiles = sorted([el.strip(' \t') for el in dasstdout.split('\n')])
      # do printouts
      if verbose:
        print('DAS client ready; found following files ({}):'.format(len(dasfiles)))
        for f in dasfiles: print('  - {}'.format(f))
      # prefix files with redirector
      filenames = [redirector+f for f in dasfiles]
  elif location=='local':
    # read all root files in the given directory
    datasetname = os.path.abspath(datasetname)
    filenames = ([os.path.join(datasetname,f) for f in os.listdir(datasetname) if f.endswith('.root')])
    if verbose:
      print('Found following local files ({}):'.format(len(filenames)))
      for f in filenames: print('  - {}'.format(f))
  else:
    raise Exception('ERROR: location "{}" not recognized'.format(location))

  # check number of retrieved files
  if len(filenames)==0:
    print('WARNING (in get_files): no files found, returning empty list.')
    return filenames

  # limit number of files to return
  if( maxfiles is not None and maxfiles>0 and maxfiles<len(filenames) ):
    print('WARNING (in get_files): returning only {} out of {} files.'.format(maxfiles, len(filenames)))
    filenames = filenames[:maxfiles]

  # return the result
  return filenames
