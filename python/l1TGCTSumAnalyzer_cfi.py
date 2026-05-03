import FWCore.ParameterSet.Config as cms

l1TGCTSumAnalyzer = cms.EDAnalyzer(
    "L1TGCTSumAnalyzer",
    folderName = cms.untracked.string("GCTSumAnalyzer"),
    debug = cms.untracked.bool(False),

    sumInputDumpFile = cms.untracked.string(
        "/afs/hep.wisc.edu/home/mbileska/apx-fs-r2-gctsum/xF13P/AlgoSRC/SUM_IP/dummy_sum_input.txt"
    ),
    cmsswSumOutputDumpFile = cms.untracked.string(
        "/afs/hep.wisc.edu/home/mbileska/apx-fs-r2-gctsum/xF13P/AlgoSRC/SUM_IP/cmssw_sum_output.txt"
    ),
    cmsswGtOutputDumpFile = cms.untracked.string(
        "/afs/hep.wisc.edu/home/mbileska/apx-fs-r2-gctsum/xF13P/AlgoSRC/TO_GT_IP/cmssw_gt_output.txt"
    ),

    inputLinks = cms.VInputTag(
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
    ),

    sumOutputLinks = cms.VInputTag(
        cms.InputTag("phase2L1GCTSumEmulator", "SumLinkOut0"),
        cms.InputTag("phase2L1GCTSumEmulator", "SumLinkOut1"),
        cms.InputTag("phase2L1GCTSumEmulator", "SumLinkOut2"),
        cms.InputTag("phase2L1GCTSumEmulator", "SumLinkOut3"),
        cms.InputTag("phase2L1GCTSumEmulator", "SumLinkOut4"),
        cms.InputTag("phase2L1GCTSumEmulator", "SumLinkOut5"),
    ),

    outputLinks = cms.VInputTag(
        cms.InputTag("phase2L1GCTSumEmulator", "LinkOut0"),
        cms.InputTag("phase2L1GCTSumEmulator", "LinkOut1"),
        cms.InputTag("phase2L1GCTSumEmulator", "LinkOut2"),
        cms.InputTag("phase2L1GCTSumEmulator", "LinkOut3"),
        cms.InputTag("phase2L1GCTSumEmulator", "LinkOut4"),
        cms.InputTag("phase2L1GCTSumEmulator", "LinkOut5"),
    ),
)
