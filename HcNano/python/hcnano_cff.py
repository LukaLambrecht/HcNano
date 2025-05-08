import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import Var

import os
import sys
import json


def add_nlepton_selector(process, nleptons=0, dtype='mc'):
    process.NLeptonSelector = cms.EDFilter("NLeptonSelector",
        minNLeptons = cms.int32(nleptons),
        muonsToken = cms.InputTag("slimmedMuons"),
        electronsToken = cms.InputTag("slimmedElectrons")
    )
    
    # modify the process.nanoAOD_step to insert the filter
    # note: this assumes the process.nanoAOD_step was defined in the CMSSW config
    #       before the customization function.
    orig_nanoaod_sequence = process.nanoAOD_step._seq
    process.nanoAOD_step = cms.Path(
      process.NLeptonSelector
      * orig_nanoaod_sequence
    )
    
    # also need to modify the output module to store only events
    # that passed the process.nanoAOD_step (including the filter as above).
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.SelectEvents = cms.untracked.PSet(
      SelectEvents = cms.vstring("nanoAOD_step")
    )
    
    # need to put the genWeightsTable in a separate Path,
    # else only events passing the filter are contributing
    # to the genEventSumw and genEventCount branches in the Runs tree.
    if dtype=='mc':
        # note: errors occur if multiple filters try to set the genWeightsPath,
        # so simply check if it was already set before.
        if not hasattr(process, 'genWeightsPath'):
            process.genWeightsPath = cms.Path(process.genWeightsTable)
            process.schedule.append(process.genWeightsPath)


def add_trigger_selector(process, dtype='mc', year=None):
    
    # parse year
    # (to be updated as needed)
    year = '2018-UL'
    if year is None:
        msg = 'Year must be provided for the trigger selector.'
        raise Exception(msg)
    triggeryear = year
    triggeryear = triggeryear.split('-')[0]

    # read trigger names
    triggerfile = os.path.join(os.path.dirname(__file__), 'triggers.json')
    with open(triggerfile, 'r') as f:
        triggers = json.load(f)
    triggers = triggers[triggeryear]

    # make selector
    process.TriggerSelector = cms.EDFilter("TriggerSelector",
        triggerNames = cms.vstring(*triggers),
        triggersToken = cms.InputTag("TriggerResults::HLT")
    )
    
    # modify the process.nanoAOD_step to insert the filter
    # note: this assumes the process.nanoAOD_step was defined in the CMSSW config
    #       before the customization function.
    orig_nanoaod_sequence = process.nanoAOD_step._seq
    process.nanoAOD_step = cms.Path(
      process.TriggerSelector
      * orig_nanoaod_sequence
    )
    
    # also need to modify the output module to store only events
    # that passed the process.nanoAOD_step (including the filter as above).
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.SelectEvents = cms.untracked.PSet(
      SelectEvents = cms.vstring("nanoAOD_step")
    )
    
    # need to put the genWeightsTable in a separate Path,
    # else only events passing the filter are contributing
    # to the genEventSumw and genEventCount branches in the Runs tree.
    if dtype=='mc':
        # note: errors occur if multiple filters try to set the genWeightsPath,
        # so simply check if it was already set before.
        if not hasattr(process, 'genWeightsPath'):
            process.genWeightsPath = cms.Path(process.genWeightsTable)
            process.schedule.append(process.genWeightsPath)


def add_ds_gen_producer(process, name='GenDsMeson', dtype='mc'):
    process.DsMesonGenProducer = cms.EDProducer("DsMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DsMesonGenProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_DsMesonGenProducer_*_*")

def add_ds_producer(process, name='DsMeson', dtype='mc'):
    process.DsMesonProducer = cms.EDProducer("DsMesonProducer",
        name = cms.string(name),
        dtype = cms.string(dtype),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DsMesonProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_DsMesonProducer_*_*")

def add_dstar_gen_producer(process, name='GenDStarMeson', dtype='mc'):
    process.DStarMesonGenProducer = cms.EDProducer("DStarMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DStarMesonGenProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_DStarMesonGenProducer_*_*")

def add_dstar_producer(process, name='DStarMeson', dtype='mc'):
    process.DStarMesonProducer = cms.EDProducer("DStarMesonProducer",
        name = cms.string(name),
        dtype = cms.string(dtype),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DStarMesonProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_DStarMesonProducer_*_*")

def add_dzero_gen_producer(process, name='GenDZeroMeson', dtype='mc'):
    process.DZeroMesonGenProducer = cms.EDProducer("DZeroMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DZeroMesonGenProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_DZeroMesonGenProducer_*_*")

def add_cfragmentation_producer(process, name='cFragmentation', dtype='mc'):
    process.cFragmentationProducer = cms.EDProducer("cFragmentationProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.cFragmentationProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_cFragmentationProducer_*_*")

def add_btodstar_gen_producer(process, name='GenBHadron', dtype='mc'):
    process.BToDStarMesonGenProducer = cms.EDProducer("BToDStarMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.BToDStarMesonGenProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_BToDStarMesonGenProducer_*_*")

def add_htodstar_gen_producer(process, name='GenHToDStarMeson', dtype='mc'):
    process.HToDStarMesonGenProducer = cms.EDProducer("HToDStarMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.HToDStarMesonGenProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_HToDStarMesonGenProducer_*_*")

def add_htodstar_producer(process, name='HToDStarMeson', dtype='mc'):
    process.HToDStarMesonProducer = cms.EDProducer("HToDStarMesonProducer",
        name = cms.string(name),
        dtype = cms.string(dtype),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.HToDStarMesonProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_HToDStarMesonProducer_*_*")

def add_htods_gen_producer(process, name='GenHToDsMeson', dtype='mc'):
    process.HToDsMesonGenProducer = cms.EDProducer("HToDsMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.HToDsMesonGenProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_HToDsMesonGenProducer_*_*")

def add_htods_producer(process, name='HToDsMeson', dtype='mc'):
    process.HToDsMesonProducer = cms.EDProducer("HToDsMesonProducer",
        name = cms.string(name),
        dtype = cms.string(dtype),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.HToDsMesonProducer
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_HToDsMesonProducer_*_*")

def add_debugger(process, name='Dbugger', dtype='mc'):
    process.Dbugger = cms.EDProducer("Dbugger",
        name = cms.string(name),
        dtype = cms.string(dtype),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.Dbugger
    )
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    outputmodule.outputCommands.append("keep *_Dbugger_*_*")


def hcnano_customize(process):

    # get data type and year from process
    # (not standard; must be set manually e.g. with --customize_commands in cmsDriver)
    dtype = 'mc'
    if hasattr(process, 'dtype'): dtype = process.dtype
    else: print(f'WARNING: process has no attribute dtype; assuming {dtype}.')
    year = None
    if hasattr(process, 'year'): year = process.year
    else: print(f'WARNING: process has not attribute year; assuming {year}.')
    print(f'INFO from hcnano_customize: using dtype {dtype} and year {year}.')

    # set output module for later use
    outputmodule = process.NANOAODSIMoutput if dtype=='mc' else process.NANOAODoutput
    
    # set fake name for CRAB
    # (not sure what this does exactly, but recommended here:
    # https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production)
    outputmodule.fakeNameForCrab = cms.untracked.bool(True)

    # disable IMT
    # (not sure what this does exactly, but recommended here:
    # https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production)
    process.add_(cms.Service("InitRootHandlers", EnableIMT=cms.untracked.bool(False)))

    # set report frequency
    # (as recommended here:
    # https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production)
    process.MessageLogger.cerr.FwkReport.reportEvery = 100

    # toggle summary
    # (ad done e.g. here:
    # https://github.com/hqucms/NanoTuples/tree/production/master)
    process.options.wantSummary = cms.untracked.bool(True)

    # do event selection to reduce size of output
    #add_trigger_selector(process, dtype=dtype, year=year)
    #add_nlepton_selector(process, nleptons=4, dtype=dtype)

    # add custom producers
    if dtype=='mc':
        #add_ds_gen_producer(process, dtype=dtype)
        #add_dstar_gen_producer(process, dtype=dtype)
        #add_dzero_gen_producer(process, dtype=dtype)
        #add_cfragmentation_producer(process, dtype=dtype)
        #add_btodstar_gen_producer(process, dtype=dtype) # temp for investigating H+b sample
        add_htodstar_gen_producer(process, dtype=dtype) # temp for investigating alternative signal
        add_htods_gen_producer(process, dtype=dtype) # temp for investigating alternative signal
    #add_ds_producer(process, dtype=dtype)
    #add_dstar_producer(process, dtype=dtype)
    add_htodstar_producer(process, dtype=dtype) # temp for investigating alternative signal
    add_htods_producer(process, dtype=dtype) # temp for investigating alternative signal
    
    # temp: add debugger
    #add_debugger(process, dtype=dtype)

    # remove unneeded output
    # note: can give errors if the main table for a given object is dropped
    #       while keeping one or more extension tables for that object!
    #       it is currently not clear how to get a list of all main tables
    #       with their corresponding extension tables,
    #       just try to deduce it from the central NanoAOD config files.
    outputmodule.outputCommands.append("drop nanoaodFlatTable_HTXSCategoryTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_beamSpotTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_boostedTau*_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_fatJet*_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_customFatJetExtTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_finalJetsAK8ConstituentsTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_subJet*_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_subjet*_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_btvSubJetMCExtTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_customSubJetsExtTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_genProtonTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_subGenJet*_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_genSubJet*_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_genVisTauTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_isoTrackTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_lheInfoTable_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_lowPtElectron*_*_*")
    outputmodule.outputCommands.append("drop nanoaodFlatTable_softActivityJet*_*_*")    
    outputmodule.outputCommands.append("drop nanoaodFlatTable_tau*_*_*")

    return process
