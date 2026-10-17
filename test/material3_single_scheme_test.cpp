#include "gtest/gtest.h"
#include "roo_windows/material3/schemes/indigo_san_marino.h"

namespace roo_windows::material3::schemes {
namespace {

// Verifies direct access works without the catalog; the linked binary is also
// inspected to ensure it retains no unrelated preset symbols.
TEST(Material3SingleSchemeTest, UsesOnlyIndigoSanMarino) {
  const ColorSchemePair& pair = IndigoSanMarino();
  EXPECT_EQ(roo_display::Color(0xFF515B92), pair.light.primary);
  EXPECT_EQ(roo_display::Color(0xFFBAC3FF), pair.dark.primary);
  static_assert(sizeof(ColorSchemePair) == 66 * sizeof(roo_display::Color),
                "Preset pairs must store only their resolved colors.");
}

}  // namespace
}  // namespace roo_windows::material3::schemes
