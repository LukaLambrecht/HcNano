# Tools for reading sample lists

def read_samplelists(samplelists, verbose=True):
    datasets = []
    if isinstance(samplelists, str): samplelists = [samplelists]
    for samplelist in samplelists:
        with open(samplelist, 'r') as f:
            lines = f.readlines()
        lines = [l.strip(' \t\n') for l in lines]
        lines = [l for l in lines if not l.startswith('#')]
        lines = [l for l in lines if len(l)>0]
        samples = lines[:]
        datasets += samples
    if verbose:
        print(f'Read following datasets from provided samplelist ({len(datasets)}):')
        for d in datasets: print(f'  - {d}')
    return datasets
