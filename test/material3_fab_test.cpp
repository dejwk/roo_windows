#include <type_traits>

#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/fab/fab.h"

namespace roo_windows::material3 {
namespace {

constexpr size_t AlignUp(size_t n, size_t alignment) {
  return (n + alignment - 1) / alignment * alignment;
}

static_assert(sizeof(FloatingActionButton) <=
              AlignUp(sizeof(SurfaceWidget) + sizeof(const MonoIcon*) + 1,
                      alignof(FloatingActionButton)));
static_assert(sizeof(ExtendedFloatingActionButton) <=
              AlignUp(sizeof(SurfaceWidget) + sizeof(roo::string_view) +
                          sizeof(const MonoIcon*) + 1,
                      alignof(ExtendedFloatingActionButton)));
static_assert(!std::is_constructible<FloatingActionButton, ApplicationContext&,
                                     const MonoIcon*>::value);
static_assert(
    !std::is_constructible<ExtendedFloatingActionButton, ApplicationContext&,
                           roo::string_view, const MonoIcon*>::value);

TEST(Material3Fab, Defaults) {
  roo_scheduler::SchedulingService scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  const MonoIcon& icon = ic_outlined_24_action_done();
  FloatingActionButton fab(context, icon);
  ExtendedFloatingActionButton extended(context, "Done", icon);
  EXPECT_EQ(&icon, &fab.icon());
  EXPECT_EQ(FabSize::kRegular, fab.size());
  EXPECT_EQ(FabColorStyle::kPrimaryContainer, fab.colorStyle());
  EXPECT_EQ(ExtendedFabSize::kSmall, extended.size());
  EXPECT_TRUE(extended.expanded());
  EXPECT_TRUE(fab.isClickable());
  EXPECT_TRUE(extended.isClickable());
}

}  // namespace
}  // namespace roo_windows::material3
