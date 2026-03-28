import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

process = cms.Process("GCTSUMALEX", eras.Phase2C17I13M9)

process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.Geometry.GeometryExtendedRun4D110Reco_cff')
process.load('Configuration.Geometry.GeometryExtendedRun4D110_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, '131X_mcRun4_realistic_v6', '')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(10)
)

process.source = cms.Source(
    "EmptySource",
    firstRun = cms.untracked.uint32(1),
    firstEvent = cms.untracked.uint32(1)
)

process.MessageLogger.cerr.threshold = cms.untracked.string('INFO')
process.MessageLogger.cerr.FwkReport = cms.untracked.PSet(
    reportEvery = cms.untracked.int32(1)
)
process.MessageLogger.cerr.default = cms.untracked.PSet(
    limit = cms.untracked.int32(1000000)
)

process.load("L1Trigger.L1CaloTrigger.l1tGCTSumTestVectorProducer_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tPhase2L1GCTSumEmulator_cfi")
process.load("L1Trigger.L1CaloPhase2Analyzer.l1TGCTSumAnalyzer_cfi")

process.gctSumTestVectorProducer.patternMode = cms.string("alexander_ptsort")

process.phase2L1GCTSumEmulator.inputLinks = cms.VInputTag(
    cms.InputTag("gctSumTestVectorProducer", "LinkIn0"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn1"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn2"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn3"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn4"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn5"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn6"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn7"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn8"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn9"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn10"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn11"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn12"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn13"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn14"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn15"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn16"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn17"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn18"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn19"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn20"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn21"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn22"),
    cms.InputTag("gctSumTestVectorProducer", "LinkIn23"),
)

process.TFileService = cms.Service(
    "TFileService",
    fileName = cms.string("gctsum_testvectors.root")
)

process.GCTSUM = cms.Path(
    process.gctSumTestVectorProducer *
    process.phase2L1GCTSumEmulator *
    process.l1TGCTSumAnalyzer
)

process.schedule = cms.Schedule(process.GCTSUM)

process.options.numberOfThreads = cms.untracked.uint32(1)
process.options.numberOfStreams = cms.untracked.uint32(0)
process.options.wantSummary = cms.untracked.bool(True)
