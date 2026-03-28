# Phase 2 L1 Calo Analyzer

## Description

   Forked from https://github.com/rpsimeon34/L1CaloPhase2Analyzer, which is an adapted version of https://github.com/pallabidas/L1CaloPhase2Analyzer.git, branch 13_3_0_calojet, which in turn
   is adapted from: https://github.com/skkwan/phase2-l1Calo-analyzer.
   This repo is for running the Phase-2 calo emulator, in particular checking the GCT Sum card emulator via deterministic test vectors. For the complete RCT flow check out https://github.com/rpsimeon34/L1CaloPhase2Analyzer, where the new, firmware-based emulator is compared to the old emulator (this is WIP for the GCT Sum).

## Prerequisites to Running

   CMSSW_15_0_0_pre3 is meant to be run with an el8 OS. If you are running on a non-el8 machine, you can
   use an el8 container. For example, one can do the following. Go to the project root directory (the
   same directory as the one in which you will run `cmsrel`), and create a file called `.bashrc` with the
   following contents:
   ```
   unset SCRAM_ARCH
   source /cvmfs/cms.cern.ch/cmsset_default.sh
   ```
   and then run the following commands in the command line:
   ```
   export IMAGE=cms:rhel8-x86_64

   singularity exec -B ${PWD}:/srv -B /etc/condor -B /etc/grid-security/certificates \
   -B /cvmfs -B /hdfs -B /scratch -B /afs --pwd /srv /cvmfs/singularity.opensciencegrid.org/cmssw/${IMAGE} \
   /bin/bash --rcfile /srv/.bashrc
   ```

   It is recommended to put the last code block into a file called `shell`. Then, before proceeding, run
   `./shell` in the command line. This must be done every time before using the
   code in this repository. Note that you can run `exit` to exit the container.

## Setup (do only once)

   ```
   cmsrel CMSSW_15_0_0_pre3
   cd CMSSW_15_0_0_pre3/src
   cmsenv
   git cms-init
   git cms-addpkg L1Trigger/L1TCalorimeter
   git cms-addpkg L1Trigger/L1CaloTrigger
   git cms-addpkg DataFormats/L1TCalorimeterPhase2
   cd L1Trigger
   git clone https://github.com/mbileska/L1CaloPhase2Analyzer.git -b 15_0_0_pre3_calojet
   cd ../
   git remote add digi-repo https://github.com/mbileska/cmssw.git
   git fetch digi-repo
   ```
   The remaining lines depend on whether you want to run the RCT emulator or the ML on RCT setup producer.

   For GCT Sum Card emulator:
   ```
   git checkout -b digi-branch --track digi-repo/from-CMSSW_15_0_0_pre3
   git pull
   scram b -j 12
   ```

## To run the GCT Sum Card emulator on test vectors

   This runs the synthetic GCT Sum test-vector producer, the GCT Sum emulator, and the analyzer.  
   It writes `gctsum_testvectors.root`, which contains the decoded GT-link outputs in `l1TGCTSumAnalyzer/gctSumTree`.

   For the current Sasha/Alexander pT-sorting test vector, the expected result is:
   - 6 valid EG objects
   - 0 valid EGIso objects
   - 6 valid Jet objects
   - 0 valid Tau objects
   - 4 valid Sum words
   - identical output in every event
   - EG and Jet `hwPt` sorted in descending order

   Run:   
   ```
   cd L1Trigger/L1CaloPhase2Analyzer/
   cmsRun test/test-gctsum-testvectors.py
   python3 test/check_gctsum_outputs.py
   ```
   
   A successful check should print:
   - `nEgValid unique: {6}`
   - `nEgiValid unique: {0}`
   - `nJetValid unique: {6}`
   - `nTauValid unique: {0}`
   - `nSumValid unique: {4}`
   - `All LinkOut0 identical: True`
   - `All LinkOut2 identical: True`
   - `All LinkOut3 identical: True`
   - `EG sorted descending: True`
   - `Jet sorted descending: True`

   The NumPy warnings printed by `check_gctsum_outputs.py` are harmless environment warnings and can be ignored.

