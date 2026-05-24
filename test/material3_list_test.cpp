#include <string>
#include <type_traits>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/list/list.h"

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
  explicit TestWidget(const Environment& env, Dimensions dimensions = {})
      : BasicWidget(env), dimensions_(dimensions) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return dimensions_;
  }

 private:
  Dimensions dimensions_;
};

class TestListEntry : public ListEntry {
 public:
  explicit TestListEntry(const Environment& env) : ListEntry(env) {}

  using ListEntry::getChild;
  using ListEntry::getChildrenCount;
};

class TestList : public List {
 public:
  explicit TestList(const Environment& env) : List(env) {}

  using List::getChild;
  using List::getChildrenCount;
};

class TrackingListEntry : public ListEntry {
 public:
  TrackingListEntry(const Environment& env, bool& destroyed)
      : ListEntry(env), destroyed_(destroyed) {}

  ~TrackingListEntry() override { destroyed_ = true; }

 private:
  bool& destroyed_;
};

class MutableListItem : public ListItem {
 public:
  explicit MutableListItem(roo_display::StringView headline)
      : headline_(headline) {}

  roo_display::StringView headlineText() const override { return headline_; }

  void setHeadline(roo_display::StringView headline) { headline_ = headline; }

 private:
  roo_display::StringView headline_;
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

// Verifies that binding a standard item attaches exactly the stable slot
// widgets exposed by the item, and clearing the item detaches them again.
TEST(Material3List, ListEntryBindsAndClearsStableSlotChildren) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestWidget leading(env, Dimensions(20, 20));
  TestWidget trailing(env, Dimensions(12, 16));
  TestWidget body(env, Dimensions(80, 10));
  StandardListItem item(StandardListItemInit::ThreeLine(
      "Headline", "Supporting", "Overline", &leading, &trailing, &body));
  TestListEntry entry(env);

  entry.setItem(item);

  EXPECT_TRUE(entry.hasItem());
  EXPECT_EQ(&item, entry.item());
  EXPECT_EQ(3, entry.getChildrenCount());
  EXPECT_EQ(&leading, &entry.getChild(0));
  EXPECT_EQ(&trailing, &entry.getChild(1));
  EXPECT_EQ(&body, &entry.getChild(2));
  EXPECT_EQ(&entry, leading.parent());
  EXPECT_EQ(&entry, trailing.parent());
  EXPECT_EQ(&entry, body.parent());

  entry.clearItem();

  EXPECT_FALSE(entry.hasItem());
  EXPECT_EQ(nullptr, entry.item());
  EXPECT_EQ(0, entry.getChildrenCount());
  EXPECT_EQ(nullptr, leading.parent());
  EXPECT_EQ(nullptr, trailing.parent());
  EXPECT_EQ(nullptr, body.parent());
}

// Verifies that a measured and laid-out row positions leading and trailing
// widgets according to the default Material 3 row padding and alignment.
TEST(Material3List, ListEntryMeasuresAndLaysOutSlots) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestWidget leading(env, Dimensions(20, 20));
  TestWidget trailing(env, Dimensions(12, 16));
  StandardListItem item(
      StandardListItemInit::OneLine("Headline", &leading, &trailing));
  TestListEntry entry(env);
  entry.setItem(item);

  Dimensions measured =
      entry.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  EXPECT_EQ(180, measured.width());
  EXPECT_GE(measured.height(), Scaled(56));

  entry.layout(Rect(0, 0, measured.width() - 1, measured.height() - 1));

  EXPECT_EQ(Scaled(16), leading.offsetLeft());
  EXPECT_EQ((measured.height() - leading.height()) / 2, leading.offsetTop());
  EXPECT_EQ(180 - Scaled(16) - trailing.width(), trailing.offsetLeft());
  EXPECT_EQ((measured.height() - trailing.height()) / 2, trailing.offsetTop());
}

// Verifies that refreshFromItem rereads lightweight descriptor state while
// preserving the already attached slot widget identity.
TEST(Material3List, ListEntryRefreshesMutableTextWithoutReplacingSlots) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  MutableListItem item("A");
  TestListEntry entry(env);
  entry.setItem(item);

  Dimensions short_measure =
      entry.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  item.setHeadline("A much longer headline");
  entry.refreshFromItem();
  Dimensions long_measure =
      entry.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));

  EXPECT_GT(long_measure.width(), short_measure.width());
  EXPECT_EQ(0, entry.getChildrenCount());
}

// Verifies that List keeps borrowed entries borrowed, adopts unique_ptr rows,
// and detaches or destroys them correctly on clear().
TEST(Material3List, ListClearsBorrowedAndAdoptedEntriesWithCorrectOwnership) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestList list(env);
  TestListEntry borrowed(env);
  bool adopted_destroyed = false;
  auto adopted = std::make_unique<TrackingListEntry>(env, adopted_destroyed);
  TrackingListEntry* adopted_raw = adopted.get();

  list.add(borrowed);
  list.add(std::move(adopted));

  EXPECT_EQ(2, list.getChildrenCount());
  EXPECT_EQ(&borrowed, &list.getChild(0));
  EXPECT_EQ(adopted_raw, &list.getChild(1));
  EXPECT_EQ(&list, borrowed.parent());
  EXPECT_EQ(&list, adopted_raw->parent());

  list.clear();

  EXPECT_EQ(0, list.getChildrenCount());
  EXPECT_EQ(nullptr, borrowed.parent());
  EXPECT_TRUE(adopted_destroyed);
}

// Verifies that List recomputes first, middle, last, and single row position
// roles as rows are added and list-level policies change.
TEST(Material3List, ListPropagatesPositionVariantStyleAndSegmentedGap) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestList list(env);
  TestListEntry first(env);
  TestListEntry second(env);
  TestListEntry third(env);
  StandardListItem first_item(StandardListItemInit::OneLine("First"));
  StandardListItem second_item(StandardListItemInit::OneLine("Second"));
  StandardListItem third_item(StandardListItemInit::OneLine("Third"));
  first.setItem(first_item);
  second.setItem(second_item);
  third.setItem(third_item);

  list.add(first);
  EXPECT_EQ(ListItemPosition::kSingle, first.visualContext().position);

  list.add(second);
  EXPECT_EQ(ListItemPosition::kFirst, first.visualContext().position);
  EXPECT_EQ(ListItemPosition::kLast, second.visualContext().position);

  list.setVariant(ListVariant::kBaseline);
  list.setStyle(ListStyle::kSegmented);
  list.add(third);

  EXPECT_EQ(ListVariant::kBaseline, first.visualContext().variant);
  EXPECT_EQ(ListStyle::kSegmented, first.visualContext().style);
  EXPECT_EQ(ListItemPosition::kFirst, first.visualContext().position);
  EXPECT_EQ(ListItemPosition::kMiddle, second.visualContext().position);
  EXPECT_EQ(ListItemPosition::kLast, third.visualContext().position);

  Dimensions measured =
      list.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  list.layout(Rect(0, 0, measured.width() - 1, measured.height() - 1));

  EXPECT_EQ(0, first.offsetTop());
  EXPECT_EQ(first.height() + Scaled(8), second.offsetTop());
  EXPECT_EQ(second.offsetTop() + second.height() + Scaled(8),
            third.offsetTop());
}

// Verifies that selection policy resolves the list-owned selected state and
// divider visibility from stored per-entry selection hints.
TEST(Material3List, ListResolvesSelectionAndDividerContext) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestList list(env);
  TestListEntry first(env);
  TestListEntry second(env);
  TestListEntry third(env);
  StandardListItem first_item(StandardListItemInit::OneLine("One"));
  StandardListItem second_item(StandardListItemInit::OneLine("Two"));
  StandardListItem third_item(StandardListItemInit::OneLine("Three"));
  first.setItem(first_item);
  second.setItem(second_item);
  third.setItem(third_item);

  ListEntryVisualContext selected_context;
  selected_context.selected = true;
  first.setVisualContext(selected_context);
  second.setVisualContext(selected_context);

  list.add(first);
  list.add(second);
  list.add(third);

  EXPECT_FALSE(first.visualContext().selected);
  EXPECT_FALSE(second.visualContext().selected);

  ListSelectionPolicy selection_policy;
  selection_policy.mode = SelectionMode::kMultiple;
  list.setSelectionPolicy(selection_policy);

  EXPECT_TRUE(first.visualContext().selected);
  EXPECT_TRUE(second.visualContext().selected);
  EXPECT_FALSE(third.visualContext().selected);

  ListDividerPolicy divider_policy;
  divider_policy.mode = DividerMode::kInset;
  divider_policy.suppress_between_selected = true;
  list.setDividerPolicy(divider_policy);

  EXPECT_FALSE(first.visualContext().show_divider);
  EXPECT_TRUE(second.visualContext().show_divider);
  EXPECT_FALSE(third.visualContext().show_divider);

  selection_policy.mode = SelectionMode::kSingle;
  list.setSelectionPolicy(selection_policy);

  EXPECT_TRUE(first.visualContext().selected);
  EXPECT_FALSE(second.visualContext().selected);
  EXPECT_TRUE(first.visualContext().show_divider);
}

// Verifies that segmented lists use list-owned gaps only while divider mode is
// disabled, and collapse back to contiguous row stacking when dividers are on.
TEST(Material3List, ListGapResolutionDependsOnDividerPolicy) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TestList list(env);
  TestListEntry first(env);
  TestListEntry second(env);
  StandardListItem first_item(StandardListItemInit::OneLine("One"));
  StandardListItem second_item(StandardListItemInit::OneLine("Two"));
  first.setItem(first_item);
  second.setItem(second_item);
  list.setStyle(ListStyle::kSegmented);
  list.add(first);
  list.add(second);

  Dimensions with_gap =
      list.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));

  ListDividerPolicy divider_policy;
  divider_policy.mode = DividerMode::kFullWidth;
  list.setDividerPolicy(divider_policy);
  Dimensions with_divider =
      list.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));

  EXPECT_EQ(with_gap.height(), with_divider.height() + Scaled(8));
  EXPECT_TRUE(first.visualContext().show_divider);
  EXPECT_FALSE(second.visualContext().show_divider);
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