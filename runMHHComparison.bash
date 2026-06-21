#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: ./runMHHComparison.bash [--skip-cmsrun]

Runs the deterministic MHH CMSSW analyzer validation, replays the exact input
through the MHH firmware C++ from mhh-clean, and compares every output word.

Environment overrides:
  CMSSW_DIR        CMSSW release directory (derived from this package).
  APXROOT          apx-fs-r2-gctsum_wip checkout (sibling of CMSSW_DIR).
  MHH_REF          firmware Git ref (default: mhh-clean).
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
MHH_REF="${MHH_REF:-mhh-clean}"
HLS_INCLUDE_DIR="${HLS_INCLUDE_DIR:-}"

[[ -d "$CMSSW_DIR/src" ]] || { echo "Invalid CMSSW_DIR: $CMSSW_DIR" >&2; exit 1; }
git -C "$APXROOT" rev-parse --verify "${MHH_REF}^{commit}" >/dev/null

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
  cmsRun test/test-mhh-testvectors.py
fi
python3 test/check_mhh_outputs.py mhh_testvectors.root

for file in mhh_testvectors_input.txt mhh_testvectors_cmssw_output.txt; do
  [[ -f "$file" ]] || { echo "Missing analyzer output: $script_dir/$file" >&2; exit 1; }
done

build_dir="$(mktemp -d "${TMPDIR:-/tmp}/mhh-cmssw-compare.XXXXXX")"
trap 'rm -rf "$build_dir"' EXIT
git -C "$APXROOT" archive "$MHH_REF" \
  targets/xF13P/AlgoSRC/MHH_IP targets/xF13P/testing/host_compat | tar -x -C "$build_dir"
mhh_dir="$build_dir/targets/xF13P/AlgoSRC/MHH_IP"

compile=(g++ -O2 -std=c++17 -Wno-unknown-pragmas -I"$mhh_dir")
link=()
if [[ -n "$HLS_INCLUDE_DIR" && -f "$HLS_INCLUDE_DIR/ap_int.h" ]]; then
  compile+=(-I"$HLS_INCLUDE_DIR")
elif [[ -n "${XILINX_HLS:-}" && -f "$XILINX_HLS/include/ap_int.h" ]]; then
  compile+=(-I"$XILINX_HLS/include")
elif [[ -f /usr/include/systemc ]] && ldconfig -p 2>/dev/null | grep -q libsystemc; then
  compile+=(-I"$build_dir/targets/xF13P/testing/host_compat" -Dmain=sc_main)
  link+=(-lsystemc)
  export SYSTEMC_DISABLE_COPYRIGHT_MESSAGE=1
else
  echo "No Xilinx ap_int.h or SystemC host backend found" >&2
  exit 1
fi

"${compile[@]}" \
  test/mhh_firmware_replay.cpp \
  "$mhh_dir/algo_top.cpp" \
  "$mhh_dir/bitonicSort32.cpp" \
  "${link[@]}" \
  -o "$build_dir/mhh_firmware_replay"

"$build_dir/mhh_firmware_replay" \
  mhh_testvectors_input.txt \
  "$build_dir/mhh_firmware_output.txt"
python3 test/compare_firmware_tables.py \
  mhh_testvectors_cmssw_output.txt \
  "$build_dir/mhh_firmware_output.txt"
