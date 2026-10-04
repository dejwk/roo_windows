// Target ABI probe: compile only, then inspect sizeof_* symbols using nm -S.
// Define ROO_WINDOWS_STANDALONE_ABI_PROBE to avoid a platform logging backend.
#if defined(ROO_WINDOWS_STANDALONE_ABI_PROBE)
#include "benchmarks/standalone_abi_logging_stub.h"
#endif

#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"

using namespace roo_windows;

[[gnu::used]] char sizeof_widget[sizeof(Widget)];
[[gnu::used]] char sizeof_container[sizeof(Container)];
[[gnu::used]] char
    sizeof_clipper_state[sizeof(roo_windows::internal::ClipperState)];
[[gnu::used]] char
    sizeof_rounded_clip[sizeof(roo_windows::internal::RoundedClip)];
[[gnu::used]] char
    sizeof_rounded_output[sizeof(roo_windows::internal::RoundedClipOutput)];
[[gnu::used]] char
    sizeof_rounded_overlay[sizeof(roo_windows::internal::RoundedOverlay)];
[[gnu::used]] char
    sizeof_rounded_decoration[sizeof(roo_windows::internal::RoundedDecoration)];
[[gnu::used]] char
    sizeof_rounded_arena[sizeof(roo_windows::internal::RoundedPaintState)];

[[gnu::used]] char
    sizeof_masked_exclusion[sizeof(roo_windows::internal::MaskedExclusion)];
[[gnu::used]] char
    sizeof_exclusion_union[sizeof(roo_windows::internal::ExclusionUnion)];
[[gnu::used]] char
    sizeof_exclusion_filter[sizeof(roo_windows::internal::ExclusionFilter)];
