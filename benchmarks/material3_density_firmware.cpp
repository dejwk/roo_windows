// Linked-image acceptance fixture: the complete runtime density example remains
// selectable at all six levels. The build flag changes only its initial choice;
// this measures configuration cost, not the cost of introducing the feature.
#include "../examples/material3/theme/density/density.ino"

#ifndef ROO_WINDOWS_DENSITY_BENCHMARK_LEVEL
#define ROO_WINDOWS_DENSITY_BENCHMARK_LEVEL 0
#endif

static_assert(ROO_WINDOWS_DENSITY_BENCHMARK_LEVEL >= -5 &&
              ROO_WINDOWS_DENSITY_BENCHMARK_LEVEL <= 0);

namespace {
// Runs after the included example has constructed its theme, application, and
// widgets, but before setup starts the display/scheduler. No upload is
// performed.
struct InitialDensity {
  InitialDensity() {
    density_example::ApplySetting(ROO_WINDOWS_DENSITY_BENCHMARK_LEVEL, material,
                                  app.root());
  }
} initial_density;
}  // namespace
