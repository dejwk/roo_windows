#include "roo_windows/containers/flex_layout.h"

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/slider/slider.h"
#include "roo_windows/widgets/divider.h"

namespace roo_windows {
namespace {

ApplicationContext MakeContext(Environment& context) {
  return ApplicationContext(context.scheduler(), context.theme(),
                            context.keyboardColorTheme());
}

class TestFlexLayout : public FlexLayout {
 public:
  explicit TestFlexLayout(ApplicationContext& context,
                          FlexDirection direction = FlexDirection::kRow)
      : FlexLayout(context, direction) {}

  using FlexLayout::add;
};

class TestFullWidthFlexLayout : public TestFlexLayout {
 public:
  explicit TestFullWidthFlexLayout(
      ApplicationContext& context,
      FlexDirection direction = FlexDirection::kRow)
      : TestFlexLayout(context, direction) {}

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

class TestWrapContentWidget : public BasicWidget {
 public:
  TestWrapContentWidget(ApplicationContext& context, Dimensions dims)
      : BasicWidget(context), dims_(dims) {}

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }

 private:
  Dimensions dims_;
};

class TestPaddedColumnFlexLayout : public TestFlexLayout {
 public:
  explicit TestPaddedColumnFlexLayout(ApplicationContext& context)
      : TestFlexLayout(context, FlexDirection::kColumn) {}
};

// A widget that reports an exact preferred size larger than what the parent
// is able to provide. Mimics e.g. a long TextLabel whose natural width exceeds
// the available cross-axis space in a padded container.
class TestExactSizeWidget : public BasicWidget {
 public:
  TestExactSizeWidget(ApplicationContext& context, Dimensions dims)
      : BasicWidget(context), dims_(dims) {}

  Dimensions getSuggestedMinimumDimensions() const override { return dims_; }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::ExactWidth(dims_.width()),
                         PreferredSize::ExactHeight(dims_.height()));
  }

 private:
  Dimensions dims_;
};

// Verifies that when align-content stretches a single line's cross size, a
// stretched flex child is sized to the post-stretch line cross size during
// onLayout (regression test for the case where the child was sized to the
// pre-stretch cross size).
TEST(FlexLayout, StretchUsesFinalLineCrossSizeAfterAlignContent) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestFlexLayout parent(context, FlexDirection::kRow);
  parent.setAlignItems(AlignItems::kStretch);
  parent.setAlignContent(AlignContent::kStretch);

  // A flex child reports wrap-content preferred size on both axes.
  TestFlexLayout child(context, FlexDirection::kRow);
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

// Verifies that a padded column FlexLayout correctly inset its match-parent
// children by the configured padding on both sides, so a divider/slider
// children land at offsetLeft==padding and span (width - 2*padding).
TEST(FlexLayout, PaddedColumnShrinksMatchParentWidthChildren) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  TestFlexLayout parent(context, FlexDirection::kColumn);
  parent.setPadding(Padding(12));

  HorizontalDivider divider(context);
  material3::Slider slider(context, material3::SliderRange{}, 0.5f);

  parent.add(divider, {.flex_grow = 0, .flex_shrink = 0});
  parent.add(slider, {.flex_grow = 0, .flex_shrink = 1});

  constexpr int16_t kParentWidth = 120;
  constexpr int16_t kParentHeight = 80;
  parent.measure(WidthSpec::Exactly(kParentWidth),
                 HeightSpec::Exactly(kParentHeight));
  parent.layout(Rect(0, 0, kParentWidth - 1, kParentHeight - 1));

  EXPECT_EQ(12, divider.offsetLeft());
  EXPECT_EQ(kParentWidth - 24, divider.width());

  EXPECT_EQ(12, slider.offsetLeft());
  EXPECT_EQ(kParentWidth - 24, slider.width());
}

// Verifies that wrapping a padded full-width column in a SimpleScrollablePanel
// keeps the panel's content at the panel width and still applies the column's
// padding to its match-parent-width children.
TEST(FlexLayout,
     ScrollablePanelRespectsNestedPaddedColumnWhenContentFillsWidth) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  auto content = std::make_unique<TestFullWidthFlexLayout>(
      context, FlexDirection::kColumn);
  TestFullWidthFlexLayout* content_ptr = content.get();
  content_ptr->setPadding(Padding(12));

  auto divider = std::make_unique<HorizontalDivider>(context);
  HorizontalDivider* divider_ptr = divider.get();
  auto slider = std::make_unique<material3::Slider>(
      context, material3::SliderRange{}, 0.5f);
  material3::Slider* slider_ptr = slider.get();

  content_ptr->add(std::move(divider), {.flex_grow = 0, .flex_shrink = 0});
  content_ptr->add(std::move(slider), {.flex_grow = 0, .flex_shrink = 1});

  SimpleScrollablePanel panel(context, std::move(content));

  constexpr int16_t kPanelWidth = 120;
  constexpr int16_t kPanelHeight = 80;
  panel.measure(WidthSpec::Exactly(kPanelWidth),
                HeightSpec::Exactly(kPanelHeight));
  panel.layout(Rect(0, 0, kPanelWidth - 1, kPanelHeight - 1));

  EXPECT_EQ(0, content_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth, content_ptr->width());

  EXPECT_EQ(12, divider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, divider_ptr->width());

  EXPECT_EQ(12, slider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, slider_ptr->width());
}

// Verifies that a wide exact-size sibling does not cause match-parent-width
// children to expand beyond the scrollable panel's viewport: padded
// divider/slider widths stay clamped to the panel width minus padding.
TEST(FlexLayout,
     ScrollablePanelKeepsMatchParentSiblingsBoundedWithWideExactSibling) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  auto content = std::make_unique<TestFullWidthFlexLayout>(
      context, FlexDirection::kColumn);
  TestFullWidthFlexLayout* content_ptr = content.get();
  content_ptr->setPadding(Padding(12));

  auto divider = std::make_unique<HorizontalDivider>(context);
  HorizontalDivider* divider_ptr = divider.get();
  auto slider = std::make_unique<material3::Slider>(
      context, material3::SliderRange{}, 0.5f);
  material3::Slider* slider_ptr = slider.get();

  auto wide_child =
      std::make_unique<TestWrapContentWidget>(context, Dimensions(240, 20));
  content_ptr->add(std::move(divider), {.flex_grow = 0, .flex_shrink = 0});
  content_ptr->add(std::move(slider), {.flex_grow = 0, .flex_shrink = 1});
  content_ptr->add(std::move(wide_child), {.flex_grow = 0, .flex_shrink = 0});

  SimpleScrollablePanel panel(context, std::move(content));

  constexpr int16_t kPanelWidth = 120;
  constexpr int16_t kPanelHeight = 80;
  panel.measure(WidthSpec::Exactly(kPanelWidth),
                HeightSpec::Exactly(kPanelHeight));
  panel.layout(Rect(0, 0, kPanelWidth - 1, kPanelHeight - 1));

  EXPECT_EQ(kPanelWidth, content_ptr->width());
  EXPECT_EQ(12, divider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, divider_ptr->width());
  EXPECT_EQ(12, slider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, slider_ptr->width());
}

// Verifies that a nested row containing a wide exact-size child does not
// expand match-parent siblings beyond the scrollable panel's viewport: the
// outer column padding still bounds divider/slider/row widths.
TEST(FlexLayout,
     ScrollablePanelKeepsMatchParentSiblingsBoundedWithWideNestedRow) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  auto content = std::make_unique<TestFullWidthFlexLayout>(
      context, FlexDirection::kColumn);
  TestFullWidthFlexLayout* content_ptr = content.get();
  content_ptr->setPadding(Padding(12));

  auto divider = std::make_unique<HorizontalDivider>(context);
  HorizontalDivider* divider_ptr = divider.get();
  auto slider = std::make_unique<material3::Slider>(
      context, material3::SliderRange{}, 0.5f);
  material3::Slider* slider_ptr = slider.get();

  auto row = std::make_unique<TestPaddedColumnFlexLayout>(context);
  TestPaddedColumnFlexLayout* row_ptr = row.get();
  row_ptr->setPadding(Padding(12));

  auto wide_child =
      std::make_unique<TestWrapContentWidget>(context, Dimensions(240, 20));
  row_ptr->add(std::move(wide_child), {.flex_grow = 0, .flex_shrink = 0});
  content_ptr->add(std::move(divider), {.flex_grow = 0, .flex_shrink = 0});
  content_ptr->add(std::move(slider), {.flex_grow = 0, .flex_shrink = 1});
  content_ptr->add(std::move(row), {.flex_grow = 0, .flex_shrink = 0});

  SimpleScrollablePanel panel(context, std::move(content));

  constexpr int16_t kPanelWidth = 120;
  constexpr int16_t kPanelHeight = 100;
  panel.measure(WidthSpec::Exactly(kPanelWidth),
                HeightSpec::Exactly(kPanelHeight));
  panel.layout(Rect(0, 0, kPanelWidth - 1, kPanelHeight - 1));

  EXPECT_EQ(kPanelWidth, content_ptr->width());
  EXPECT_EQ(12, divider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, divider_ptr->width());
  EXPECT_EQ(12, slider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, slider_ptr->width());
  EXPECT_EQ(12, row_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, row_ptr->width());
}

// Reproduces the structure of the material3 slider example: a scrollable
// panel that contains a padded full-width column, whose direct children
// include nested column FlexLayouts (each containing a slider).
// Verifies the layout used by the Material 3 slider example: a scrollable
// panel hosts a padded full-width column whose nested SliderRow columns
// inherit the correct content area, with each slider clamped to the panel
// width minus the combined padding of both the outer column and its row.
TEST(FlexLayout, SliderExampleNestedColumnsHaveCorrectPadding) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);

  auto content = std::make_unique<TestFullWidthFlexLayout>(
      context, FlexDirection::kColumn);
  TestFullWidthFlexLayout* content_ptr = content.get();
  content_ptr->setPadding(Padding(12));

  auto top_divider = std::make_unique<HorizontalDivider>(context);
  HorizontalDivider* top_divider_ptr = top_divider.get();
  content_ptr->add(std::move(top_divider), {.flex_grow = 0, .flex_shrink = 0});

  // Three nested column FlexLayouts (like SliderRow), each with its own
  // padding and a Slider child reporting MatchParentWidth preferred size.
  material3::Slider* slider_ptrs[3] = {nullptr, nullptr, nullptr};
  TestFlexLayout* row_ptrs[3] = {nullptr, nullptr, nullptr};
  for (int i = 0; i < 3; ++i) {
    auto row =
        std::make_unique<TestFlexLayout>(context, FlexDirection::kColumn);
    row_ptrs[i] = row.get();
    row->setPadding(Padding(12, 8));
    auto slider = std::make_unique<material3::Slider>(
        context, material3::SliderRange{}, 0.5f);
    slider_ptrs[i] = slider.get();
    row_ptrs[i]->add(std::move(slider), {.flex_grow = 0, .flex_shrink = 1});
    content_ptr->add(std::move(row), {.flex_grow = 0, .flex_shrink = 0});
  }

  auto footer_divider = std::make_unique<HorizontalDivider>(context);
  HorizontalDivider* footer_divider_ptr = footer_divider.get();
  content_ptr->add(std::move(footer_divider),
                   {.flex_grow = 0, .flex_shrink = 0});

  // A label that reports an exact preferred width larger than the
  // available cross-axis space. This emulates a long TextLabel from the
  // slider example.
  constexpr int16_t kPanelWidth = 320;
  constexpr int16_t kPanelHeight = 240;
  auto wide_label = std::make_unique<TestExactSizeWidget>(
      context, Dimensions(kPanelWidth + 40, 20));
  content_ptr->add(std::move(wide_label), {.flex_grow = 0, .flex_shrink = 0});

  SimpleScrollablePanel panel(context, std::move(content));

  panel.measure(WidthSpec::Exactly(kPanelWidth),
                HeightSpec::Exactly(kPanelHeight));
  panel.layout(Rect(0, 0, kPanelWidth - 1, kPanelHeight - 1));

  EXPECT_EQ(kPanelWidth, content_ptr->width());

  // Top-level divider should sit inside the content's padding.
  EXPECT_EQ(12, top_divider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, top_divider_ptr->width());

  EXPECT_EQ(12, footer_divider_ptr->offsetLeft());
  EXPECT_EQ(kPanelWidth - 24, footer_divider_ptr->width());

  for (int i = 0; i < 3; ++i) {
    EXPECT_EQ(12, row_ptrs[i]->offsetLeft()) << "row " << i << " offsetLeft";
    EXPECT_EQ(kPanelWidth - 24, row_ptrs[i]->width())
        << "row " << i << " width";

    // Slider inside the row should sit inside the row's own padding (12 each
    // side), so absolute offset is 12 + 12 = 24 and width is panel - 48.
    EXPECT_EQ(12, slider_ptrs[i]->offsetLeft())
        << "slider " << i << " offsetLeft";
    EXPECT_EQ(kPanelWidth - 48, slider_ptrs[i]->width())
        << "slider " << i << " width";
  }
}

}  // namespace
}  // namespace roo_windows
