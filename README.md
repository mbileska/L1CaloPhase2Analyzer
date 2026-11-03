# Phase-2 GCTSum and MHH Analyzer Validation

This package validates the CMSSW GCTSum and HF/HGCAL mixer (MHH) emulators.
It records the packed link payloads, decodes useful ROOT branches, and compares
CMSSW output against the authoritative firmware C++ models.

## CMSSW 20 MHH/GCTS simulated-MC workflow

The reproducible Wisconsin workflow for running the MHH-to-GCTS chain on
Phase2Fall26 `GEN-SIM-DIGI-RAW`, validating the ten-event reference output,
and scaling to one input file per Condor job is documented in
[`docs/CMSSW20_MHH_GCTS_MC_WORKFLOW.md`](docs/CMSSW20_MHH_GCTS_MC_WORKFLOW.md).

That workflow requires CMSSW `20_0_0_patch2`.  The analyzer implementation on
this `15_0_0_pre3_calojet` branch has **not** yet been ported to CMSSW 20 and
must not be assumed to decode the sums-ABI-v2 products correctly.  The
standalone EDM validator in `test/validate_mhh_gcts_edm.py` is ABI-v2 aware.

## Is the APX firmware checkout required?

No, not for ordinary CMSSW event production.  Once the clean firmware-derived
MHH/GCTS headers and producers have been committed in the CMSSW area, this
runtime path is self-contained:

```text
Phase-2 MC -> CMSSW HF/DMX adapters -> CMSSW MHH -> CMSSW GCTS -> EDM/ntuple ROOT
```

Producing those ROOT files requires the compatible CMSSW release and branch,
the CMSSW configuration, the input MC, and any desired analyzer or validator.
It does **not** require a live checkout of
`apx-fs-r2-gctsum_wip/targets/xF13P`.

The dependency by task is:

| Task | Needs `apx-fs-r2-gctsum_wip/targets/xF13P`? |
| --- | --- |
| Run the CMSSW MHH/GCTS chain on MC | No |
| Write the replay EDM or training ntuple | No |
| Run `test/validate_mhh_gcts_edm.py` on an EDM file | No |
| Run the analyzer's ROOT-only diagnostic checkers | No |
| Run `runComparison.bash` or `runMHHComparison.bash` for exact CMSSW/firmware parity | Yes |
| Rebuild/update the emulator from the authoritative firmware C++ | Yes |
| Run HLS, RTL, TMUX6, scouting, link-distribution, or FPGA tests | Yes |

The APX repository remains the authoritative firmware source and is strongly
recommended for release validation.  Record the APX branch and commit used to
derive the CMSSW code even when the repository is not present in production.
The sibling-checkout layout below applies to the parity drivers, not to a
normal `cmsRun` production job.

## Checkout Layout

The default scripts assume the CMSSW release and APX firmware repository are
siblings. This is the cluster layout:

```text
work/
  CMSSW_15_0_0_pre3/
    src/L1Trigger/L1CaloPhase2Analyzer/
  apx-fs-r2-gctsum_wip/
    targets/xF13P/
```

Override `CMSSW_DIR` or `APXROOT` when using another layout. The scripts read
firmware with `git archive`; they do not switch or modify the APX working tree.
`runComparison.bash` defaults to `gcts-clean`, and `runMHHComparison.bash`
defaults to `mhh-clean`.

## Environment And Build

CMSSW 15.0.0 pre-releases require a compatible CMS runtime. On the cluster,
enter the usual CMS container and then run:

```bash
cd /path/to/CMSSW_15_0_0_pre3/src
cmsenv
scram b -j 12
```

The integrated CMSSW branch is `from-CMSSW_15_0_0_pre3`; this Analyzer checkout
uses `15_0_0_pre3_calojet`. The separate `emulator-gcts-mhh-clean` CMSSW branch
contains only the two firmware-derived emulators, without Analyzer `.cfi` or
input/replay producer files.

## GCTSum Test Vectors

This is the fast CMSSW behavior suite. It runs 10 deterministic patterns twice:
zero input, object propagation and ordering, both eta sides, gamma and hadronic
stitching boundaries, no-stitch cases, source-dependent sum hypotheses, final
GT packet mapping, and deterministic repetition.

```bash
cd "$CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer"
cmsRun test/test-gctsum-testvectors.py
python3 test/check_gctsum_outputs.py gctsum_testvectors.root
```

The checker must exit zero and print `PASSED` for every contract. The ROOT tree
is `l1TGCTSumAnalyzer/gctSumTree`. Each event stores all 24 inputs, six SUM_IP
outputs, six TO_GT outputs, and decoded candidates and sums.

## GCTSum Firmware Parity

The full comparison uses MC input and compares all six GT links, all nine
64-bit words per link, for every event. It builds the CMSSW release, runs the
Analyzer, archives `targets/xF13P` from `gcts-clean`, compiles `SUM_IP` and
`TO_GT_IP`, replays the Analyzer input, and performs the exact comparison.

```bash
cd "$CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer"
./runComparison.bash
```

Reuse existing Analyzer dumps without rerunning `cmsRun`:

```bash
./runComparison.bash --skip-cmsrun
```

Useful overrides:

```bash
APXROOT=/cluster/path/apx-fs-r2-gctsum_wip \
INPUT_ROOT=file:/cluster/path/input.root \
MAX_EVENTS=1000 \
GCTSUM_REF=gcts-clean \
HLS_INCLUDE_DIR=/path/to/hls/include \
./runComparison.bash
```

Success means the firmware comparator reports no mismatch. This is the
bit-for-bit test; the ROOT checker above is the easier diagnostic when it fails.

## MHH Test Vectors And Firmware Parity

The MHH driver runs both validation layers in one command. Its eight-pattern
cycle is repeated twice and covers zero input, HF-only and HGCAL-only objects,
gamma boundary stitching, no-stitch phi separation, hadronic stitching, signed
Ex/Ey plus scalar sum aggregation, ignored reserved inputs, zero reserved
outputs, and determinism.

```bash
cd "$CMSSW_BASE/src/L1Trigger/L1CaloPhase2Analyzer"
./runMHHComparison.bash
```

The script performs these checks:

1. `cmsRun test/test-mhh-testvectors.py` writes the ROOT tree and link tables.
2. `test/check_mhh_outputs.py` checks decoded physics and link contracts.
3. The exact MHH C++ from `mhh-clean` replays the 16 Analyzer input links.
4. `test/compare_firmware_tables.py` compares every word on all six output links.

Expected final messages are:

```text
MHH analyzer validation: all 16 deterministic events passed
MHH CMSSW/firmware comparison: 16 events, all 6 links match
```

Reuse existing ROOT and text output with:

```bash
./runMHHComparison.bash --skip-cmsrun
```

Override `MHH_REF`, `APXROOT`, `CMSSW_DIR`, or `HLS_INCLUDE_DIR` as needed.

## File Replay

`GCTSumFileInputProducer` and `MHHFileInputProducer` feed saved link tables back
into CMSSW. MHH tables have one row-counter column followed by 16 payload
columns, grouped as nine rows per event; set `payloadOffset=0`. GCTSum replay
supports the 32-column board dump through its positive/negative column offsets.

Replay the deterministic MHH table and rerun the same ROOT checks with:

```bash
cmsRun test/test-mhh-replay.py \
  inputFile=mhh_testvectors_input.txt \
  outputFile=mhh_replay.root \
  maxEvents=16
python3 test/check_mhh_outputs.py mhh_replay.root
```

For a release-quality validation, run the fast ROOT check, the corresponding
firmware parity driver, and the standalone firmware suite on the APX branch:

```bash
# In an APX checkout on gcts-clean
./targets/xF13P/testing/run_gctsum_verification.sh

# In an APX checkout on mhh-clean
./targets/xF13P/testing/run_mhh_verification.sh
```

The standalone suites isolate firmware regressions; the Analyzer comparisons
then test CMSSW packing, emulator logic, output mapping, and file I/O together.
