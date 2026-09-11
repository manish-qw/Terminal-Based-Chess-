# Search Ablation Results

This document tracks the telemetry results of disabling specific search heuristics to validate their effectiveness.

| Configuration | Nodes | QNodes | BetaCutoffs | TTCutoffs | LMR Reductions | Null Cutoffs | Killer Hits |
|---|---|---|---|---|---|---|---|
| Baseline | 83,604 | 64,737 | 3,763 | 530 | 160 | 27 | 890 |
| No Null Move | 90,931 | 69,507 | 4,568 | 590 | 160 | 0 | 1,178 |
| No LMR | 91,179 | 68,491 | 3,912 | 483 | 0 | 27 | 928 |
| No Killer | 147,881 | 113,764 | 5,273 | 634 | 319 | 27 | 932 |
