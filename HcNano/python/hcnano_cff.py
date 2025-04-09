import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import Var


def add_nlepton_selector(process, nleptons):
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
    process.NANOAODSIMoutput.SelectEvents = cms.untracked.PSet(
      SelectEvents = cms.vstring("nanoAOD_step")
    )
    # need to put the genWeightsTable in a separate Path,
    # else only events passing the filter are contributing
    # to the genEventSumw and genEventCount branches in the Runs tree.
    process.genWeightsPath = cms.Path(process.genWeightsTable)
    process.schedule.append(process.genWeightsPath)

def add_ds_gen_producer(process, name='GenDsMeson'):
    process.DsMesonGenProducer = cms.EDProducer("DsMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DsMesonGenProducer
    )
    process.NANOAODSIMoutput.outputCommands.append("keep *_DsMesonGenProducer_*_*")

def add_ds_producer(process, name='DsMeson'):
    process.DsMesonProducer = cms.EDProducer("DsMesonProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DsMesonProducer
    )
    process.NANOAODSIMoutput.outputCommands.append("keep *_DsMesonProducer_*_*")

def add_dstar_gen_producer(process, name='GenDStarMeson'):
    process.DStarMesonGenProducer = cms.EDProducer("DStarMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DStarMesonGenProducer
    )
    process.NANOAODSIMoutput.outputCommands.append("keep *_DStarMesonGenProducer_*_*")

def add_dstar_producer(process, name='DStarMeson'):
    process.DStarMesonProducer = cms.EDProducer("DStarMesonProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DStarMesonProducer
    )
    process.NANOAODSIMoutput.outputCommands.append("keep *_DStarMesonProducer_*_*")

def add_dzero_gen_producer(process, name='GenDZeroMeson'):
    process.DZeroMesonGenProducer = cms.EDProducer("DZeroMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.DZeroMesonGenProducer
    )
    process.NANOAODSIMoutput.outputCommands.append("keep *_DZeroMesonGenProducer_*_*")

def add_cfragmentation_producer(process, name='cFragmentation'):
    process.cFragmentationProducer = cms.EDProducer("cFragmentationProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.nanoAOD_step = cms.Path(
      process.nanoAOD_step._seq
      * process.cFragmentationProducer
    )
    process.NANOAODSIMoutput.outputCommands.append("keep *_cFragmentationProducer_*_*")


def hcnano_customize(process):
    
    # set fake name for CRAB
    # (not sure what this does exactly, but recommended here:
    # https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production)
    process.NANOAODSIMoutput.fakeNameForCrab = cms.untracked.bool(True)

    # disable IMT
    # (not sure what this does exactly, but recommended here:
    # https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production)
    process.add_(cms.Service("InitRootHandlers", EnableIMT=cms.untracked.bool(False)))

    # set report frequency
    # (as recommended here:
    # https://gitlab.cern.ch/cms-nanoAOD/nanoaod-doc/-/wikis/Instructions/Private%20production)
    process.MessageLogger.cerr.FwkReport.reportEvery = 1

    # toggle summary
    # (ad done e.g. here:
    # https://github.com/hqucms/NanoTuples/tree/production/master)
    process.options.wantSummary = cms.untracked.bool(True)

    # do event selection to reduce size of output
    # (experimental)
    add_nlepton_selector(process, 4)

    # add custom producers
    add_ds_gen_producer(process)
    add_ds_producer(process)
    add_dstar_gen_producer(process)
    add_dstar_producer(process)
    add_dzero_gen_producer(process)
    add_cfragmentation_producer(process)

    # remove unneeded output
    # note: can give errors if the main table for a given object is dropped
    #       while keeping one or more extension tables for that object!
    #       it is currently not clear how to get a list of all main tables
    #       with their corresponding extension tables,
    #       just try to deduce it from the central NanoAOD config files.
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_HTXSCategoryTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_beamSpotTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_boostedTau*_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_fatJet*_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_customFatJetExtTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_finalJetsAK8ConstituentsTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_subJet*_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_subjet*_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_btvSubJetMCExtTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_customSubJetsExtTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_genProtonTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_subGenJet*_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_genSubJet*_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_genVisTauTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_isoTrackTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_lheInfoTable_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_lowPtElectron*_*_*")
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_softActivityJet*_*_*")    
    process.NANOAODSIMoutput.outputCommands.append("drop nanoaodFlatTable_tau*_*_*")

    return process
