import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing


options = VarParsing("analysis")
options.maxEvents = -1
options.register(
    "dataset",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Dataset name stored in the training tree",
)
options.register(
    "sourceLFN",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Original Phase2Fall26 MC LFN/PFN stored in the training tree",
)
options.register(
    "inputEDM",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Intermediate emulator EDM path stored in the training tree",
)
options.register(
    "ntupleFile",
    "mhh_gcts_training.root",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "TFileService output (not decorated by VarParsing maxEvents)",
)
options.register(
    "cmsswRelease",
    "CMSSW_20_0_0_patch2",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "CMSSW release stored in the training tree",
)
options.register(
    "globalTag",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "GlobalTag stored in the training tree",
)
options.register(
    "cmsswCommit",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "CMSSW emulator git commit stored in the training tree",
)
options.register(
    "analyzerCommit",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "L1CaloPhase2Analyzer git commit stored in the training tree",
)
options.register(
    "emulatorConfigurationHash",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Emulator cmsRun configuration SHA-256 stored in the training tree",
)
options.register(
    "ntupleConfigurationHash",
    "UNKNOWN",
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Ntuplizer cmsRun configuration SHA-256 stored in the training tree",
)
options.parseArguments()

process = cms.Process("TRAINING")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("L1Trigger.L1CaloPhase2Analyzer.mhhGCTSTrainingNtuplizer_cfi")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(options.inputFiles),
)
process.options = cms.untracked.PSet(
    numberOfThreads=cms.untracked.uint32(1),
    numberOfStreams=cms.untracked.uint32(1),
    wantSummary=cms.untracked.bool(True),
)
process.MessageLogger.cerr.FwkReport.reportEvery = 100

process.mhhGCTSTrainingNtuplizer.dataset = cms.string(options.dataset)
process.mhhGCTSTrainingNtuplizer.sourceLFN = cms.string(options.sourceLFN)
process.mhhGCTSTrainingNtuplizer.inputEDM = cms.string(options.inputEDM)
process.mhhGCTSTrainingNtuplizer.cmsswRelease = cms.string(options.cmsswRelease)
process.mhhGCTSTrainingNtuplizer.globalTag = cms.string(options.globalTag)
process.mhhGCTSTrainingNtuplizer.cmsswCommit = cms.string(options.cmsswCommit)
process.mhhGCTSTrainingNtuplizer.analyzerCommit = cms.string(options.analyzerCommit)
process.mhhGCTSTrainingNtuplizer.emulatorConfigurationHash = cms.string(
    options.emulatorConfigurationHash
)
process.mhhGCTSTrainingNtuplizer.ntupleConfigurationHash = cms.string(
    options.ntupleConfigurationHash
)

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string(options.ntupleFile),
)
process.training = cms.Path(process.mhhGCTSTrainingNtuplizer)
