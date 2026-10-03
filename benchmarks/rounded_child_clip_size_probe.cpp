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
[[gnu::used]] char sizeof_clipper_state[sizeof(internal::ClipperState)];
[[gnu::used]] char sizeof_rounded_clip[sizeof(internal::RoundedClip)];
[[gnu::used]] char sizeof_rounded_output[sizeof(internal::RoundedClipOutput)];
[[gnu::used]] char sizeof_rounded_overlay[sizeof(internal::RoundedOverlay)];
[[gnu::used]] char
    sizeof_rounded_decoration[sizeof(internal::RoundedDecoration)];
[[gnu::used]] char sizeof_rounded_arena[sizeof(internal::RoundedPaintState)];
