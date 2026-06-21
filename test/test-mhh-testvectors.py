import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

process = cms.Process("MHH", eras.Phase2C17I13M9)

process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tMHHTestVectorProducer_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tPhase2L1MHHEmulator_cfi")
process.load("L1Trigger.L1CaloPhase2Analyzer.l1TMHHAnalyzer_cfi")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(16))
process.source = cms.Source(
    "EmptySource",
    firstRun=cms.untracked.uint32(1),
    firstEvent=cms.untracked.uint32(1),
)

input_tags = cms.VInputTag(
    *[cms.InputTag("mhhTestVectorProducer", f"LinkIn{i}") for i in range(16)]
)
output_tags = cms.VInputTag(
    *[cms.InputTag("phase2L1MHHEmulator", f"LinkOut{i}") for i in range(6)]
)

process.phase2L1MHHEmulator.inputLinks = input_tags
process.l1TMHHAnalyzer.inputLinks = input_tags
process.l1TMHHAnalyzer.outputLinks = output_tags
process.l1TMHHAnalyzer.inputDumpFile = cms.untracked.string("mhh_testvectors_input.txt")
process.l1TMHHAnalyzer.outputDumpFile = cms.untracked.string("mhh_testvectors_cmssw_output.txt")

process.TFileService = cms.Service(
    "TFileService",
    fileName=cms.string("mhh_testvectors.root"),
)

process.mhhPath = cms.Path(
    process.mhhTestVectorProducer
    * process.phase2L1MHHEmulator
    * process.l1TMHHAnalyzer
)
process.schedule = cms.Schedule(process.mhhPath)
process.options.numberOfThreads = cms.untracked.uint32(1)
process.options.numberOfStreams = cms.untracked.uint32(1)
process.options.wantSummary = cms.untracked.bool(True)
