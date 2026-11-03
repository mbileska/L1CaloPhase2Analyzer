"""Overlay the proven MHH/GCTS integration config with campaign retention.

The base configuration path is supplied through MHH_GCTS_BASE_CFG.  Its
VarParsing options remain authoritative; this overlay only extends the EDM
output commands so the upstream HCAL/HGCAL inputs used by the adapters are
preserved alongside every packed emulator stage.
"""

import os


base_config = os.environ.get("MHH_GCTS_BASE_CFG")
if not base_config:
    raise RuntimeError("MHH_GCTS_BASE_CFG is not set")
if not os.path.isfile(base_config):
    raise RuntimeError(f"base MHH/GCTS configuration does not exist: {base_config}")

with open(base_config, "rb") as source:
    exec(compile(source.read(), base_config, "exec"), globals())

if not hasattr(process, "output"):
    raise RuntimeError(f"base configuration {base_config} has no process.output")

process.output.outputCommands.extend(
    [
        "keep *_simHcalTriggerPrimitiveDigis_*_*",
        "keep *_l1tHGCalTowerProducer_HGCalTowerProcessor_*",
        "keep *_l1tHGCalBackEndLayer2Producer_HGCalBackendLayer2Processor3DClustering_*",
    ]
)
