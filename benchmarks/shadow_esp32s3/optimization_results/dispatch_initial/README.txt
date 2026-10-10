Initial cache dispatch, before reusing nearest-edge geometry on cache misses.
The side-only cache repeated geometry work after a failed lookup and regressed
isolated pixel sampling. This directory preserves those screening measurements;
use the parent directory for the final optimized implementation and conclusions.
To reproduce the initial dispatch, replace shadow_lookup.cpp with the attached
shadow_lookup.cpp.txt on the optimization branch, then build the same PIO modes.
The two double_optimized runs here predate the final shared-geometry baseline;
use the parent directory for the final baseline measurements.
