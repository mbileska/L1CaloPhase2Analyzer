#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./runComparison.bash [--skip-cmsrun]

Runs the GCTSum CMSSW analyzer, replays SUM_IP and TO_GT_IP from gcts-clean,
and compares all six final GT links word-for-word.

Environment overrides:
  CMSSW_DIR        CMSSW release directory (derived from this package).
  INPUT_ROOT       MC ROOT file passed to test/test-gctsum-mc.py.
  MAX_EVENTS       cmsRun event count (default: 100).
  APXROOT          apx-fs-r2-gctsum_wip checkout (sibling of CMSSW_DIR).
  GCTSUM_REF       firmware Git ref (default: gcts-clean).
  HLS_INCLUDE_DIR  directory containing Xilinx ap_int.h.
EOF
}

run_cmsrun=1
case "${1:-}" in
  "") ;;
  --skip-cmsrun) run_cmsrun=0 ;;
  -h|--help) usage; exit 0 ;;
  *) echo "Unknown option: $1" >&2; usage >&2; exit 2 ;;
esac

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
default_cmssw_dir="$(cd "$script_dir/../../.." && pwd)"
CMSSW_DIR="${CMSSW_DIR:-$default_cmssw_dir}"
APXROOT="${APXROOT:-$(dirname "$CMSSW_DIR")/apx-fs-r2-gctsum_wip}"
GCTSUM_REF="${GCTSUM_REF:-gcts-clean}"
INPUT_ROOT="${INPUT_ROOT:-file:///hdfs/store/user/rsimeon/MCFiles/001ebf5f-b83c-43fc-997f-c2e5ecf1f9dd.root}"
MAX_EVENTS="${MAX_EVENTS:-100}"
HLS_INCLUDE_DIR="${HLS_INCLUDE_DIR:-}"

[[ -d "$CMSSW_DIR/src" ]] || { echo "Invalid CMSSW_DIR: $CMSSW_DIR" >&2; exit 1; }
git -C "$APXROOT" rev-parse --verify "${GCTSUM_REF}^{commit}" >/dev/null

if ! command -v cmsenv >/dev/null 2>&1; then
  set +u
  source /cvmfs/cms.cern.ch/cmsset_default.sh
  set -u
fi

cd "$CMSSW_DIR/src"
cmsenv
scram b -j "${BUILD_JOBS:-8}"

cd "$script_dir"
if [[ "$run_cmsrun" -eq 1 ]]; then
  cmsRun test/test-gctsum-mc.py \
    inputFiles="$INPUT_ROOT" \
    outputFile=gctsum_mc.root \
    sumInputDumpFile=gctsum_mc_sum_input.txt \
    sumOutputDumpFile=gctsum_mc_sum_output.txt \
    gtOutputDumpFile=gctsum_mc_gt_output.txt \
    maxEvents="$MAX_EVENTS"
fi

for file in gctsum_mc_sum_input.txt gctsum_mc_gt_output.txt; do
  [[ -f "$file" ]] || { echo "Missing analyzer output: $script_dir/$file" >&2; exit 1; }
done

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/gctsum-cmssw-compare.XXXXXX")"
trap 'rm -rf "$build_dir"' EXIT
git -C "$APXROOT" archive "$GCTSUM_REF" targets/xF13P | tar -x -C "$build_dir"
xfdir="$build_dir/targets/xF13P"

sum_in24="$build_dir/gctsum_mc_sum_input_24links.txt"
sumip_out="$build_dir/gctsum_mc_sumip_firmware.txt"
togt_out="$build_dir/gctsum_mc_togt_firmware.txt"

awk '
BEGIN {
  printf "WordCnt"
  for (i = 0; i < 12; ++i) printf "    POS_%d", i
  for (i = 0; i < 12; ++i) printf "    NEG_%d", i
  printf "\n#BeginData\n"
}
$1 ~ /^[0-9a-fA-F]+$/ {
  printf "%s", $1
  for (i = 2; i <= 13; ++i)  printf "    %s", $i
  for (i = 18; i <= 29; ++i) printf "    %s", $i
  printf "\n"
}
' gctsum_mc_sum_input.txt > "$sum_in24"

compile=(g++ -O2 -std=c++17 -Wno-unknown-pragmas)
link=()
if [[ -n "$HLS_INCLUDE_DIR" && -f "$HLS_INCLUDE_DIR/ap_int.h" ]]; then
  compile+=(-I"$HLS_INCLUDE_DIR")
elif [[ -n "${XILINX_HLS:-}" && -f "$XILINX_HLS/include/ap_int.h" ]]; then
  compile+=(-I"$XILINX_HLS/include")
elif [[ -f /usr/include/systemc ]] && ldconfig -p 2>/dev/null | grep -q libsystemc; then
  compile+=(-I"$xfdir/testing/host_compat" -Dmain=sc_main)
  link+=(-lsystemc)
  export SYSTEMC_DISABLE_COPYRIGHT_MESSAGE=1
else
  echo "No Xilinx ap_int.h or SystemC host backend found" >&2
  exit 1
fi

"${compile[@]}" \
  -I"$xfdir/AlgoSRC/SUM_IP" \
  test/gctsum_sum_firmware_replay.cpp \
  "$xfdir/AlgoSRC/SUM_IP/algo_top.cpp" \
  "$xfdir/AlgoSRC/SUM_IP/bitonicSort32.cpp" \
  "${link[@]}" -o "$build_dir/gctsum_sum_firmware_replay"

"${compile[@]}" \
  -I"$xfdir/AlgoSRC/TO_GT_IP" \
  test/gctsum_togt_firmware_replay.cpp \
  "$xfdir/AlgoSRC/TO_GT_IP/algo_top.cpp" \
  "${link[@]}" -o "$build_dir/gctsum_togt_firmware_replay"

"$build_dir/gctsum_sum_firmware_replay" "$sum_in24" "$sumip_out"
"$build_dir/gctsum_togt_firmware_replay" "$sumip_out" "$togt_out"
python3 test/compare_firmware_tables.py gctsum_mc_gt_output.txt "$togt_out" GCTSum
