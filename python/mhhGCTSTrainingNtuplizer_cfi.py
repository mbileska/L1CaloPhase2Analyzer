import math

import FWCore.ParameterSet.Config as cms


def _links(module, prefix, count):
    return [cms.InputTag(module, f"{prefix}{index}") for index in range(count)]


mhhGCTSTrainingNtuplizer = cms.EDAnalyzer(
    "MHHGCTSTrainingNtuplizer",
    # Ordering is positive eta first, negative eta second.
    hfLinks=cms.VInputTag(
        *(
            _links("l1tPhase2L1HFMHHLinkProducerPositive", "LinkIn", 3)
            + _links("l1tPhase2L1HFMHHLinkProducerNegative", "LinkIn", 3)
        )
    ),
    dmxLinks=cms.VInputTag(
        *(
            _links("l1tPhase2L1DMXEmulatorPositive", "LinkIn", 16)
            + _links("l1tPhase2L1DMXEmulatorNegative", "LinkIn", 16)
        )
    ),
    mhhLinks=cms.VInputTag(
        *(
            _links("l1tPhase2L1MHHFromHFAndDMXPositive", "LinkOut", 6)
            + _links("l1tPhase2L1MHHFromHFAndDMXNegative", "LinkOut", 6)
        )
    ),
    # Empty means: derive the current smoke input as MHH links 0..2 followed
    # by nine zero placeholder links on each side. Supply all 24 tags once
    # real GCTB products are connected.
    gctsInputLinks=cms.VInputTag(),
    gctsSideLinks=cms.VInputTag(
        *_links("l1tPhase2L1GCTSumFromMHHSmoke", "SumLinkOut", 6)
    ),
    gtLinks=cms.VInputTag(
        *_links("l1tPhase2L1GCTSumFromMHHSmoke", "LinkOut", 6)
    ),
    genJets=cms.InputTag("ak4GenJetsNoNu"),
    genMET=cms.InputTag("genMetTrue"),
    gctbPresent=cms.bool(False),
    gctbSourceMask=cms.uint32(0),
    sourceEtLSB=cms.double(0.5),
    gtEtLSB=cms.double(1.0 / 32.0),
    gtEtaLSB=cms.double(0.0005),
    gtPhiLSB=cms.double(math.pi / 4096.0),
    dataset=cms.string("UNKNOWN"),
    sourceLFN=cms.string("UNKNOWN"),
    inputEDM=cms.string("UNKNOWN"),
    cmsswRelease=cms.string("CMSSW_20_0_0_patch2"),
    globalTag=cms.string("UNKNOWN"),
    cmsswCommit=cms.string("UNKNOWN"),
    analyzerCommit=cms.string("UNKNOWN"),
    emulatorConfigurationHash=cms.string("UNKNOWN"),
    ntupleConfigurationHash=cms.string("UNKNOWN"),
)
