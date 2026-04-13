# Phase 2 L1 GCT Sum Analyzer

## Description

   Forked from https://github.com/rpsimeon34/L1CaloPhase2Analyzer, which is an adapted version of https://github.com/pallabidas/L1CaloPhase2Analyzer.git, branch 13_3_0_calojet, which in turn
   is adapted from: https://github.com/skkwan/phase2-l1Calo-analyzer.
   This branch is for the Phase-2 GCT Sum Card analyzer and validation flow. It runs the CMSSW GCT Sum emulator, writes the packed Sum input and GT-link output dumps, and compares those dumps against the firmware C++ replay. RCT and ML-on-RCT workflows are not part of this README.

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
   For GCT Sum Card emulator:
   ```
   git checkout -b digi-branch --track digi-repo/from-CMSSW_15_0_0_pre3
   git pull
   scram b -j 12
   ```

## Running the GCT Sum Validation on Test Vectors

   This runs the synthetic GCT Sum test-vector producer, the GCT Sum emulator, and the analyzer.  
   It writes `gctsum_testvectors.root`, which contains the decoded GT-link outputs in `l1TGCTSumAnalyzer/gctSumTree`.

   The current test setup runs a deterministic 6-event pattern cycle, repeated twice for a determinism check:
   - all-zero input
   - single positive-side EG-like object
   - positive-side EG + sums
   - dense positive-side occupancy
   - positive and negative eta populated
   - sparse hadron/tau-like pattern and a sum

   Run:   
   ```
   cd L1Trigger/L1CaloPhase2Analyzer/
   cmsRun test/test-gctsum-testvectors.py
   python3 test/check_gctsum_outputs.py
   ```
   
   A successful check should print:
   - `Test event count: PASSED`
   - `Test GT output link format (6 links, 9 words each): PASSED`
   - `Test all-zero vector minimal output: PASSED`
   - `Test single EG-like object propagates: PASSED`
   - `Test EG plus sums case: PASSED`
   - `Test dense positive-side EG/EGiso ordering: PASSED`
   - `Test positive/negative eta separation affects GT output: PASSED`
   - `Test hadron/tau-like pattern propagates: PASSED`
   - `Test deterministic repeat over second 6-event cycle: PASSED`
   - `Test non-zero word count deterministic: PASSED`
   - `Test decoded sum branches consistent with sum validity: PASSED`

   Notes:
   - `cmsRun` uses the emulator: the test-vector producer only provides the input links, the emulator processes them, and the analyzer reads the emulator outputs.
   - If execution feels slow, the main overhead is usually analyzer-side event printing rather than the GCT Sum emulator logic itself.
   - The NumPy warnings printed by `check_gctsum_outputs.py` are harmless environment warnings and can be ignored or suppressed.

## Running the GCT Sum MC-to-Firmware Comparison

   The `runComparison.bash` script runs the GCT Sum comparison chain used for MC input. It does not run any RCT analyzer workflow.

   The script does the following:
   - builds the CMSSW release
   - runs `test/test-gctsum-mc.py` unless `--skip-cmsrun` is passed
   - writes `gctsum_mc.root`, `gctsum_mc_sum_input.txt`, and `gctsum_mc_gt_output.txt` in this analyzer directory
   - converts the 32-link CMSSW Sum input dump to the 24 links used by the firmware replay
   - builds and runs the `SUM_IP` and `TO_GT_IP` C++ replay programs from `xF13P`
   - compares the CMSSW GT-link output against the replayed firmware output over 6 GT links

   Run from the analyzer directory:
   ```
   cd $CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer
   ./runComparison.bash
   ```

   To reuse existing `gctsum_mc_sum_input.txt` and `gctsum_mc_gt_output.txt` files without rerunning `cmsRun`:
   ```
   ./runComparison.bash --skip-cmsrun
   ```

   The main environment variables are:
   - `CMSSW_DIR`: CMSSW release directory. By default the script derives this from the analyzer location.
   - `INPUT_ROOT`: input ROOT file for `test/test-gctsum-mc.py`. The default is `file:///hdfs/store/user/rsimeon/MCFiles/001ebf5f-b83c-43fc-997f-c2e5ecf1f9dd.root`.
   - `MAX_EVENTS`: number of events for `cmsRun`; default is `100`.
   - `APXROOT`: directory containing the `xF13P` checkout; default is `$HOME/apx-fs-r2-gctsum_wip`.
   - `XFDIR`: `xF13P` directory; default is `$APXROOT/xF13P`.
   - `HLS_INCLUDE_DIR`: HLS include directory; default is the CMSSW CVMFS HLS 2019.08 include path.

   Example with overrides:
   ```
   CMSSW_DIR=$HOME/CMSSW_15_0_0_pre3 \
   APXROOT=/afs/hep.wisc.edu/home/mbileska/apx-fs-r2-gctsum_wip \
   MAX_EVENTS=100 \
   ./runComparison.bash
   ```
