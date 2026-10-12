// Compile-only ESP32-S3 ABI probe. Inspect sizeof_* symbols with nm -S.
#include "benchmarks/standalone_abi_logging_stub.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/damage_region.h"
#include "roo_windows/core/widget.h"

[[gnu::used]] char sizeof_widget[sizeof(roo_windows::Widget)];
[[gnu::used]] char sizeof_container[sizeof(roo_windows::Container)];
[[gnu::used]] char
    sizeof_clipper_output[sizeof(roo_windows::internal::ClipperOutput)];
[[gnu::used]] char
    sizeof_damage_region[sizeof(roo_windows::internal::DamageRegion)];
