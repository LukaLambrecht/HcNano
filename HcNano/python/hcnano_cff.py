import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import Var


def add_ds_gen_producer(process, name='GenDsMeson'):
    process.DsMesonGenProducer = cms.EDProducer("DsMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.DsMesonGenTask = cms.Task(process.DsMesonGenProducer)
    process.schedule.associate(process.DsMesonGenTask)
    process.NANOAODSIMoutput.outputCommands.append("keep *_DsMesonGenProducer_*_*")

def add_ds_producer(process, name='DsMeson'):
    process.DsMesonProducer = cms.EDProducer("DsMesonProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.DsMesonTask = cms.Task(process.DsMesonProducer)
    process.schedule.associate(process.DsMesonTask)
    process.NANOAODSIMoutput.outputCommands.append("keep *_DsMesonProducer_*_*")

def add_dstar_gen_producer(process, name='GenDStarMeson'):
    process.DStarMesonGenProducer = cms.EDProducer("DStarMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.DStarMesonGenTask = cms.Task(process.DStarMesonGenProducer)
    process.schedule.associate(process.DStarMesonGenTask)
    process.NANOAODSIMoutput.outputCommands.append("keep *_DStarMesonGenProducer_*_*")

def add_dstar_producer(process, name='DStarMeson'):
    process.DStarMesonProducer = cms.EDProducer("DStarMesonProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles"),
        packedPFCandidatesToken = cms.InputTag("packedPFCandidates"),
        lostTracksToken = cms.InputTag("lostTracks")
    )
    process.DStarMesonTask = cms.Task(process.DStarMesonProducer)
    process.schedule.associate(process.DStarMesonTask)
    process.NANOAODSIMoutput.outputCommands.append("keep *_DStarMesonProducer_*_*")

def add_dzero_gen_producer(process, name='GenDZeroMeson'):
    process.DZeroMesonGenProducer = cms.EDProducer("DZeroMesonGenProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.DZeroMesonGenTask = cms.Task(process.DZeroMesonGenProducer)
    process.schedule.associate(process.DZeroMesonGenTask)
    process.NANOAODSIMoutput.outputCommands.append("keep *_DZeroMesonGenProducer_*_*")

def add_cfragmentation_producer(process, name='cFragmentation'):
    process.cFragmentationProducer = cms.EDProducer("cFragmentationProducer",
        name = cms.string(name),
        genParticlesToken = cms.InputTag("prunedGenParticles")
    )
    process.cFragmentationTask = cms.Task(process.cFragmentationProducer)
    process.schedule.associate(process.cFragmentationTask)
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

    # add custom producers
    add_ds_gen_producer(process)
    add_ds_producer(process)
    add_dstar_gen_producer(process)
    add_dstar_producer(process)
    add_dzero_gen_producer(process)
    add_cfragmentation_producer(process)

    return process
