"""Analyzer inputs for the full RCT/GCTB + HF/HGCAL -> MHH -> GCTS chain.

The smoke-test cfi remains unchanged and retains explicit zero-GCTB metadata.
"""

import FWCore.ParameterSet.Config as cms

from L1Trigger.L1CaloPhase2Analyzer.mhhGCTSTrainingNtuplizer_cfi import (
    mhhGCTSTrainingNtuplizer,
)


def _gcts_sources():
    tags = []
    for side, mhh in (
        ("Positive", "l1tPhase2L1MHHFromHFAndDMXPositive"),
        ("Negative", "l1tPhase2L1MHHFromHFAndDMXNegative"),
    ):
        tags.extend(cms.InputTag(mhh, "LinkOut{}".format(link)) for link in range(3))
        for card in range(1, 4):
            adapter = "l1tPhase2L1GCTB{}ToGCTS".format(card)
            tags.extend(
                cms.InputTag(adapter, "LinkOut{}{}".format(side, link))
                for link in range(3)
            )
    return cms.VInputTag(*tags)


mhhGCTSFullCaloTrainingNtuplizer = mhhGCTSTrainingNtuplizer.clone(
    gctsInputLinks=_gcts_sources(),
    gctsSideLinks=cms.VInputTag(
        *(cms.InputTag("l1tPhase2L1GCTSumFullCalo", "SumLinkOut{}".format(link))
          for link in range(6))
    ),
    gtLinks=cms.VInputTag(
        *(cms.InputTag("l1tPhase2L1GCTSumFullCalo", "LinkOut{}".format(link))
          for link in range(6))
    ),
    gctbPresent=cms.bool(True),
    gctbSourceMask=cms.uint32(7),
)
