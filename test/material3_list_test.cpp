#include "roo_windows/material3/list/list.h"

#include <string>
#include <type_traits>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace material3 {
namespace {

static_assert(std::is_base_of<Container, ListEntry>::value,
              "ListEntry must keep the fixed-child Container boundary");
static_assert(std::is_base_of<Container, List>::value,
              "List must remain a public widget container");
static_assert(!std::is_base_of<FlexLayout, List>::value,
              "List must not leak FlexLayout in its public type");
static_assert(std::is_convertible<List*, Widget*>::value,
              "List must remain composable as a Widget");
static_assert(!std::is_convertible<List*, FlexLayout*>::value,
              "List callers must not be able to use FlexLayout APIs");
static_assert(std::is_base_of<ListItem, StandardListItem>::value,
              "StandardListItem must remain a ListItem descriptor");

class TestWidget : public BasicWidget {
 public:
  explicit TestWidget(const Environment& env) : BasicWidget(env) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(0, 0);
  }
};

// Verifies that the public policy structs start from the Phase 1 design
// defaults and stay compact enough to be stored inline.
TEST(Material3List, PolicyDefaultsMatchDesign) {
  ListTextPolicy text_policy;
  EXPECT_EQ(TextOverflowPolicy::kTruncate, text_policy.overflow);
  EXPECT_EQ(1, text_policy.max_lines);

  ListSelectionPolicy selection_policy;
  EXPECT_EQ(SelectionMode::kNone, selection_policy.mode);
  EXPECT_EQ(SelectionAffordance::kNone, selection_policy.affordance);
  EXPECT_EQ(AffordancePlacement::kTrailing, selection_policy.placement);
  EXPECT_TRUE(selection_policy.selection_follows_press);

  ListDividerPolicy divider_policy;
  EXPECT_EQ(DividerMode::kNone, divider_policy.mode);
  EXPECT_EQ(0, divider_policy.start_inset);
  EXPECT_EQ(0, divider_policy.end_inset);
  EXPECT_TRUE(divider_policy.suppress_between_selected);

  ListEntryVisualContext visual_context;
  EXPECT_EQ(ListVariant::kExpressive, visual_context.variant);
  EXPECT_EQ(ListStyle::kStandard, visual_context.style);
  EXPECT_EQ(ListItemPosition::kSingle, visual_context.position);
  EXPECT_FALSE(visual_context.selected);
  EXPECT_TRUE(visual_context.enabled);
  EXPECT_FALSE(visual_context.pressed);
  EXPECT_FALSE(visual_context.focused);
  EXPECT_FALSE(visual_context.hovered);
  EXPECT_FALSE(visual_context.show_divider);

  EXPECT_LE(sizeof(ListTextPolicy), 2U);
  EXPECT_LE(sizeof(ListSelectionPolicy), 4U);
  EXPECT_LE(sizeof(ListDividerPolicy), 4U);
  EXPECT_LE(sizeof(DividerInsetHint), 2U);
  EXPECT_LE(sizeof(ListEntryVisualContext), 12U);
}

// Verifies that the convenience init builders populate only descriptor data,
// without requiring any row widget behavior.
TEST(Material3List, StandardListItemInitPresetsPopulateDescriptorData) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestWidget leading(env);
  TestWidget trailing(env);
  TestWidget body(env);

  StandardListItemInit one_line =
      StandardListItemInit::OneLine("WiFi", &leading, &trailing);
  EXPECT_EQ("WiFi", std::string(one_line.headline));
  EXPECT_TRUE(one_line.supporting.empty());
  EXPECT_EQ(&leading, one_line.leading);
  EXPECT_EQ(&trailing, one_line.trailing);
  EXPECT_EQ(nullptr, one_line.body);
  EXPECT_EQ(1, one_line.headline_policy.max_lines);
  EXPECT_EQ(VerticalVisualAlignment::kMiddle, one_line.leading_alignment);

  StandardListItemInit two_line =
      StandardListItemInit::TwoLine("Pump", "Auto", &leading, &trailing);
  EXPECT_EQ("Pump", std::string(two_line.headline));
  EXPECT_EQ("Auto", std::string(two_line.supporting));
  EXPECT_EQ(1, two_line.supporting_policy.max_lines);
  EXPECT_FALSE(two_line.prefer_top_text_alignment);

  StandardListItemInit three_line = StandardListItemInit::ThreeLine(
      "Filter", "Last cleaned today", "Status", &leading, &trailing, &body);
  EXPECT_EQ("Status", std::string(three_line.overline));
  EXPECT_EQ(&body, three_line.body);
  EXPECT_EQ(1, three_line.supporting_policy.max_lines);
  EXPECT_EQ(VerticalVisualAlignment::kTop, three_line.leading_alignment);
  EXPECT_EQ(VerticalVisualAlignment::kTop, three_line.trailing_alignment);
  EXPECT_TRUE(three_line.prefer_top_text_alignment);
}

// Verifies that StandardListItem is construction-time configured and mirrors
// the stable borrowed widgets from its init descriptor.
TEST(Material3List, StandardListItemMirrorsInitDescriptor) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestWidget leading(env);
  TestWidget trailing(env);

  StandardListItemInit init =
      StandardListItemInit::TwoLine("Pool pump", "Auto", &leading, &trailing);
  init.divider_inset_hint.start_inset = 12;
  init.divider_inset_hint.end_inset = 4;
  StandardListItem item(init);

  EXPECT_EQ("Pool pump", std::string(item.headlineText()));
  EXPECT_EQ("Auto", std::string(item.supportingText()));
  EXPECT_TRUE(item.overlineText().empty());
  EXPECT_EQ(1, item.headlinePolicy().max_lines);
  EXPECT_EQ(1, item.supportingPolicy().max_lines);
  EXPECT_EQ(&leading, item.leading());
  EXPECT_EQ(&trailing, item.trailing());
  EXPECT_EQ(nullptr, item.body());
  EXPECT_EQ(&leading, item.leadingWidget());
  EXPECT_EQ(&trailing, item.trailingWidget());
  EXPECT_EQ(12, item.dividerInsetHint().start_inset);
  EXPECT_EQ(4, item.dividerInsetHint().end_inset);
  EXPECT_FALSE(item.preferTopTextAlignment());
}

// Verifies that the Phase 1 widgets can be constructed with safe default
// state before row layout, child binding, and expansion behavior are added.
TEST(Material3List, BaselineClassesConstructWithSafeDefaults) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);

  ListEntry entry(env);
  EXPECT_FALSE(entry.hasItem());
  EXPECT_EQ(nullptr, entry.item());
  EXPECT_EQ(ListVariant::kExpressive, entry.visualContext().variant);
  EXPECT_EQ(ListItemPosition::kSingle, entry.visualContext().position);

  ListEntryVisualContext selected_context;
  selected_context.selected = true;
  selected_context.position = ListItemPosition::kFirst;
  entry.setVisualContext(selected_context);
  EXPECT_TRUE(entry.visualContext().selected);
  EXPECT_EQ(ListItemPosition::kFirst, entry.visualContext().position);

  ExpandablePanel panel(env);
  EXPECT_FALSE(panel.isExpanded());
  EXPECT_FALSE(panel.isAnimating());
  panel.clearContent();
  panel.setExpanded(false);
  panel.setAnimationDuration(180);

  List list(env);
  WidgetRef borrowed_list(list);
  EXPECT_EQ(&list, borrowed_list.get());
  list.clear();
}

// Verifies the checked Phase 1 size budget using pointer-size-aware limits so
// the host build still catches accidental inline state growth.
TEST(Material3List, BaselineTypesStayWithinPhaseOneSizeBudget) {
  constexpr size_t kStandardItemBudget =
      3 * sizeof(roo_display::StringView) + 4 * sizeof(void*) + 16;
  constexpr size_t kListEntryBudget =
      sizeof(Container) + 4 * sizeof(void*) + 16;
  constexpr size_t kExpandablePanelBudget =
      sizeof(Container) + sizeof(WidgetRef) + 8;
    constexpr size_t kListBudget = sizeof(Container) + sizeof(void*) * 8 + 16;

  EXPECT_LE(sizeof(StandardListItem), kStandardItemBudget);
  EXPECT_LE(sizeof(ListEntry), kListEntryBudget);
  EXPECT_LE(sizeof(ExpandablePanel), kExpandablePanelBudget);
  EXPECT_LE(sizeof(List), kListBudget);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows