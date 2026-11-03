#!/usr/bin/env python3
"""Validate the raw MHH/GCTS products in a CMSSW EDM output file.

Run this under the same CMSSW 20 runtime and AVX2-capable worker used for the
integration job.  The script deliberately reads the raw vector<uint64_t>
products rather than an analyzer ntuple, so it can validate the handoff before
the CMSSW-20 ntuplizer is implemented.
"""

import argparse
import math
import sys


SOURCE_WIDTH = 12
SIDE_WIDTH = 16
WORDS_PER_LINK = 9


class ValidationError(RuntimeError):
    pass


def signed(value, width):
    value &= (1 << width) - 1
    sign = 1 << (width - 1)
    return value - (1 << width) if value & sign else value


def decode_source_sums(words):
    word0, word1 = words[0], words[1]
    return {
        "ex": signed(word0, SOURCE_WIDTH),
        "ey": signed(word0 >> 12, SOURCE_WIDTH),
        "htx": signed(word0 >> 24, SOURCE_WIDTH),
        "hty": signed(word0 >> 36, SOURCE_WIDTH),
        "et": word1 & 0xFFF,
        "ht": (word1 >> 12) & 0xFFF,
        "nobj": (word1 >> 24) & 0xFFF,
    }


def decode_side_sums(words):
    word0, word1 = words[0], words[1]
    return {
        "ex": signed(word0, SIDE_WIDTH),
        "ey": signed(word0 >> 16, SIDE_WIDTH),
        "htx": signed(word0 >> 32, SIDE_WIDTH),
        "hty": signed(word0 >> 48, SIDE_WIDTH),
        "et": word1 & 0xFFFF,
        "ht": (word1 >> 16) & 0xFFFF,
        "nobj": (word1 >> 32) & 0xFFFF,
    }


def vector_phi(x_value, y_value):
    """Bit-exact integer CORDIC used by sums ABI v2."""
    angles = (
        1073741824,
        633866811,
        334917815,
        170009512,
        85334662,
        42708931,
        21359677,
        10680490,
        5340327,
        2670173,
        1335088,
        667544,
        333772,
        166886,
        83443,
        41722,
        20861,
        10430,
        5215,
        2608,
        1304,
        652,
        326,
        163,
    )
    x_work = x_value << 24
    y_work = y_value << 24
    angle = 0
    if x_work < 0:
        angle = -(1 << 32) if y_work < 0 else (1 << 32)
        x_work = -x_work
        y_work = -y_work
    for index, cordic_angle in enumerate(angles):
        old_x = x_work
        if y_work > 0:
            x_work += y_work >> index
            y_work -= old_x >> index
            angle += cordic_angle
        elif y_work < 0:
            x_work -= y_work >> index
            y_work += old_x >> index
            angle -= cordic_angle
    code = (abs(angle) + 524288) >> 20
    if angle < 0:
        code = -code
    return code & 0x1FFF


def pack_vector_sum(x_value, y_value, scalar):
    magnitude = min(0xFFFF, math.isqrt((x_value * x_value + y_value * y_value) << 8))
    phi = vector_phi(-x_value, -y_value)
    scalar_gt = min(0xFFFF, scalar << 4)
    return 1 | (magnitude << 1) | (phi << 17) | (scalar_gt << 30)


def pack_scalar16(value):
    return 1 | (min(0xFFFF, value) << 1)


def require(condition, message):
    if not condition:
        raise ValidationError(message)


def get_words(event, handle_class, module, instance, process, event_label):
    handle = handle_class("std::vector<unsigned long>")
    event.getByLabel((module, instance, process), handle)
    require(handle.isValid(), f"{event_label}: missing {module}:{instance}:{process}")
    words = [int(word) for word in handle.product()]
    require(
        len(words) == WORDS_PER_LINK,
        f"{event_label}: {module}:{instance} has {len(words)} words, expected {WORDS_PER_LINK}",
    )
    return words


def all_zero(words):
    return not any(words)


def parse_event_ids(value):
    if not value:
        return None
    return [int(item) for item in value.split(",") if item.strip()]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", help="MHH/GCTS EDM ROOT file")
    parser.add_argument("--process", default="MHHGCTS", help="EDM process name")
    parser.add_argument("--expected-events", type=int)
    parser.add_argument(
        "--expected-event-ids",
        type=parse_event_ids,
        help="comma-separated event numbers in expected file order",
    )
    parser.add_argument(
        "--expect-zero-gctb",
        action="store_true",
        help="assert that GCTS side sums equal MHH because all GCTB triplets are placeholders",
    )
    parser.add_argument(
        "--require-nonzero",
        action="store_true",
        help="require at least one nonzero MHH and final-GT payload in the file",
    )
    args = parser.parse_args()

    try:
        from DataFormats.FWLite import Events, Handle
    except ImportError as error:
        print(f"ERROR: initialize the CMSSW 20 runtime before running this validator: {error}", file=sys.stderr)
        return 2

    event_numbers = []
    saw_nonzero_mhh = False
    saw_nonzero_gt = False

    try:
        for event_index, event in enumerate(Events(args.input)):
            auxiliary = event.eventAuxiliary()
            event_label = (
                f"entry {event_index} run={auxiliary.run()} "
                f"lumi={auxiliary.luminosityBlock()} event={auxiliary.event()}"
            )
            event_numbers.append(int(auxiliary.event()))

            hf = {}
            dmx = {}
            mhh = {}
            for side in ("Positive", "Negative"):
                hf[side] = [
                    get_words(
                        event,
                        Handle,
                        f"l1tPhase2L1HFMHHLinkProducer{side}",
                        f"LinkIn{link}",
                        args.process,
                        event_label,
                    )
                    for link in range(3)
                ]
                dmx[side] = [
                    get_words(
                        event,
                        Handle,
                        f"l1tPhase2L1DMXEmulator{side}",
                        f"LinkIn{link}",
                        args.process,
                        event_label,
                    )
                    for link in range(16)
                ]
                mhh[side] = [
                    get_words(
                        event,
                        Handle,
                        f"l1tPhase2L1MHHFromHFAndDMX{side}",
                        f"LinkOut{link}",
                        args.process,
                        event_label,
                    )
                    for link in range(6)
                ]

                for reserved_link in range(3, 6):
                    require(
                        all_zero(mhh[side][reserved_link]),
                        f"{event_label}: MHH {side} LinkOut{reserved_link} is not reserved-zero",
                    )
                require(
                    all_zero(mhh[side][2][2:]),
                    f"{event_label}: MHH {side} sums link has nonzero reserved words 2..8",
                )

            side_outputs = [
                get_words(
                    event,
                    Handle,
                    "l1tPhase2L1GCTSumFromMHHSmoke",
                    f"SumLinkOut{link}",
                    args.process,
                    event_label,
                )
                for link in range(6)
            ]
            gt_outputs = [
                get_words(
                    event,
                    Handle,
                    "l1tPhase2L1GCTSumFromMHHSmoke",
                    f"LinkOut{link}",
                    args.process,
                    event_label,
                )
                for link in range(6)
            ]

            for sum_link in (2, 5):
                require(
                    all_zero(side_outputs[sum_link][2:]),
                    f"{event_label}: GCTS SumLinkOut{sum_link} has nonzero reserved words 2..8",
                )
            require(
                all_zero(gt_outputs[5][6:]),
                f"{event_label}: final GCTS LinkOut5 words 6..8 are not zero",
            )

            positive_mhh = decode_source_sums(mhh["Positive"][2])
            negative_mhh = decode_source_sums(mhh["Negative"][2])
            positive_side = decode_side_sums(side_outputs[2])
            negative_side = decode_side_sums(side_outputs[5])

            if args.expect_zero_gctb:
                require(
                    positive_side == positive_mhh,
                    f"{event_label}: positive GCTS side sums do not equal zero-GCTB MHH input: "
                    f"GCTS={positive_side}, MHH={positive_mhh}",
                )
                require(
                    negative_side == negative_mhh,
                    f"{event_label}: negative GCTS side sums do not equal zero-GCTB MHH input: "
                    f"GCTS={negative_side}, MHH={negative_mhh}",
                )

            total_ex = positive_side["ex"] + negative_side["ex"]
            total_ey = positive_side["ey"] + negative_side["ey"]
            total_htx = positive_side["htx"] + negative_side["htx"]
            total_hty = positive_side["hty"] + negative_side["hty"]
            total_et = positive_side["et"] + negative_side["et"]
            total_ht = positive_side["ht"] + negative_side["ht"]
            total_nobj = positive_side["nobj"] + negative_side["nobj"]

            expected_met = pack_vector_sum(total_ex, total_ey, total_et)
            expected_mht = pack_vector_sum(total_htx, total_hty, total_ht)
            expected_nobj = pack_scalar16(total_nobj)
            require(
                gt_outputs[5][3] == expected_met,
                f"{event_label}: MET/ET word mismatch: observed=0x{gt_outputs[5][3]:016x}, "
                f"expected=0x{expected_met:016x}",
            )
            require(
                gt_outputs[5][4] == expected_mht,
                f"{event_label}: MHT/HT word mismatch: observed=0x{gt_outputs[5][4]:016x}, "
                f"expected=0x{expected_mht:016x}",
            )
            require(
                gt_outputs[5][5] == expected_nobj,
                f"{event_label}: NObj word mismatch: observed=0x{gt_outputs[5][5]:016x}, "
                f"expected=0x{expected_nobj:016x}",
            )

            saw_nonzero_mhh |= any(any(words) for words in mhh["Positive"] + mhh["Negative"])
            saw_nonzero_gt |= any(any(words) for words in gt_outputs)

        if args.expected_events is not None:
            require(
                len(event_numbers) == args.expected_events,
                f"file has {len(event_numbers)} events, expected {args.expected_events}",
            )
        if args.expected_event_ids is not None:
            require(
                event_numbers == args.expected_event_ids,
                f"event sequence {event_numbers} does not match {args.expected_event_ids}",
            )
        if args.require_nonzero:
            require(saw_nonzero_mhh, "no nonzero MHH payload found")
            require(saw_nonzero_gt, "no nonzero final GCTS payload found")
    except ValidationError as error:
        print(f"FAIL: {error}", file=sys.stderr)
        return 1

    print(
        f"PASS: validated {len(event_numbers)} events; all raw links have nine words, "
        "reserved fields are zero, and final sums match ABI v2"
    )
    if args.expect_zero_gctb:
        print("PASS: GCTB-placeholder mode is explicit and GCTS side sums equal the MHH sums")
    return 0


if __name__ == "__main__":
    sys.exit(main())
