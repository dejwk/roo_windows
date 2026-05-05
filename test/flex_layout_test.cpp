#include "roo_windows/containers/flex_layout.h"

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class TestFlexLayout : public FlexLayout {
 public:
  explicit TestFlexLayout(const Environment& env,
                          FlexDirection direction = FlexDirection::kRow)
      : FlexLayout(env, direction) {}

  using FlexLayout::add;
};

TEST(FlexLayout, StretchUsesFinalLineCrossSizeAfterAlignContent) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  TestFlexLayout parent(env, FlexDirection::kRow);
  parent.setAlignItems(AlignItems::kStretch);
  parent.setAlignContent(AlignContent::kStretch);

  // A flex child reports wrap-content preferred size on both axes.
  TestFlexLayout child(env, FlexDirection::kRow);
  FlexLayout::Params params;
  params.flex_grow = 1;
  parent.add(child, params);

  constexpr int16_t kParentWidth = 120;
  constexpr int16_t kParentHeight = 60;
  parent.measure(WidthSpec::Exactly(kParentWidth),
                 HeightSpec::Exactly(kParentHeight));
  parent.layout(Rect(0, 0, kParentWidth - 1, kParentHeight - 1));

  // Regression: line cross size can grow during align-content, and stretch
  // must size the child to that final cross size during onLayout.
  EXPECT_EQ(kParentHeight, child.height());
  EXPECT_EQ(0, child.offsetTop());
}

}  // namespace
}  // namespace roo_windows
