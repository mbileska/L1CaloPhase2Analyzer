import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing
from Configuration.StandardSequences.Eras import eras

options = VarParsing.VarParsing("analysis")
options.outputFile = "mhh_replay.root"
options.maxEvents = 16
options.register(
    "inputFile",
    "mhh_testvectors_input.txt",
    VarParsing.VarParsing.multiplicity.singleton,
    VarParsing.VarParsing.varType.string,
    "MHH table with a row counter followed by 16 hexadecimal link words",
)
options.parseArguments()

process = cms.Process("MHHREPLAY", eras.Phase2C17I13M9)
process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tMHHFileInputProducer_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tPhase2L1MHHEmulator_cfi")
process.load("L1Trigger.L1CaloPhase2Analyzer.l1TMHHAnalyzer_cfi")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))
process.source = cms.Source(
    "EmptySource",
    firstRun=cms.untracked.uint32(1),
    firstEvent=cms.untracked.uint32(1),
)

process.mhhFileInputProducer.inputFile = cms.string(options.inputFile)
input_tags = cms.VInputTag(
    *[cms.InputTag("mhhFileInputProducer", f"LinkIn{i}") for i in range(16)]
)
output_tags = cms.VInputTag(
    *[cms.InputTag("phase2L1MHHEmulator", f"LinkOut{i}") for i in range(6)]
)
process.phase2L1MHHEmulator.inputLinks = input_tags
process.l1TMHHAnalyzer.inputLinks = input_tags
process.l1TMHHAnalyzer.outputLinks = output_tags
process.l1TMHHAnalyzer.inputDumpFile = cms.untracked.string("mhh_replay_input.txt")
process.l1TMHHAnalyzer.outputDumpFile = cms.untracked.string("mhh_replay_cmssw_output.txt")

process.TFileService = cms.Service("TFileService", fileName=cms.string(options.outputFile))
process.replayPath = cms.Path(
    process.mhhFileInputProducer
    * process.phase2L1MHHEmulator
    * process.l1TMHHAnalyzer
)
process.schedule = cms.Schedule(process.replayPath)
process.options.numberOfThreads = cms.untracked.uint32(1)
process.options.numberOfStreams = cms.untracked.uint32(1)
