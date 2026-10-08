#include <limits>

#include "gtest/gtest.h"
#include "roo_icons/filled/24/navigation.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows::material3 {
namespace {

constexpr Density kLevels[] = {Density::kDefault, Density::kMinus1,
                               Density::kMinus2,  Density::kMinus3,
                               Density::kMinus4,  Density::kMinus5};

class DensityOverrideTest : public testing::Test {
 protected:
  DensityOverrideTest()
      : device_(240, 320, raster_, roo_display::Argb4444()),
        display_(device_),
        material_(DefaultTheme().material3Theme()),
        theme_{MakeFrameworkTheme(material_), &material_},
        environment_(scheduler_, theme_),
        app_(&environment_, display_) {}

  ApplicationContext& context() { return app_.context(); }

  void changeShared(Density density) {
    material_.density = density;
    app_.root().requestLayoutDescending();
    app_.root().invalidateDescending();
    app_.refresh();
  }

  roo::byte raster_[240 * 320 * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Material3Theme material_;
  Theme theme_;
  Environment environment_;
  Application app_;
};

// Verifies inheritance is distinct from explicit zero and resolves live values.
TEST(DensityOverride, PolicyRepresentationAndResolution) {
  constexpr DensityOverride inherited;
  constexpr DensityOverride zero = DensityOverride::Explicit(Density::kDefault);
  static_assert(sizeof(DensityOverride) == 1);
  static_assert(inherited.isInherited());
  static_assert(!zero.isInherited());
  static_assert(inherited != zero);
  for (Density density : kLevels) {
    EXPECT_EQ(inherited.resolve(density), density);
    EXPECT_EQ(zero.resolve(density), Density::kDefault);
    EXPECT_EQ(DensityOverride::Explicit(density).resolve(Density::kDefault),
              density);
  }
}

// Verifies invalid explicit casts never turn into an inherited policy.
TEST_F(DensityOverrideTest, InvalidExplicitLevels) {
  List list(context());
  ListRow<HeadlineListItem> row(context(), "Row");
  for (int value : {-128, -6, 1, 127}) {
    Density invalid = static_cast<Density>(value);
#ifndef NDEBUG
    EXPECT_DEATH(DensityOverride::Explicit(invalid),
                 "invalid Material 3 density");
    EXPECT_DEATH(list.setDensity(invalid), "invalid Material 3 density");
    EXPECT_DEATH(row.setDensity(invalid), "invalid Material 3 density");
#else
    material_.density = Density::kMinus5;
    list.setDensity(invalid);
    row.setDensity(invalid);
    EXPECT_FALSE(list.densityOverride().isInherited());
    EXPECT_FALSE(row.densityOverride().isInherited());
    EXPECT_EQ(row.getSuggestedMinimumDimensions().height(), Scaled(56));
#endif
  }
}

// Verifies standalone overrides affect natural/preferred/measured geometry and
// layout, leave a sibling on shared density, and can restore live inheritance.
TEST_F(DensityOverrideTest, StandaloneRowsAndSiblingIsolation) {
  ListRow<HeadlineListItem> pinned(context(), "Pinned");
  ListRow<HeadlineListItem> sibling(context(), "Sibling");
  VerticalLayout layout(context());
  layout.add(pinned);
  layout.add(sibling);
  Task& task = app_.addTaskFullScreen(layout);
  app_.refresh();
  changeShared(Density::kMinus5);
  EXPECT_EQ(sibling.height(), Scaled(36));
  for (int step = 0; step <= 5; ++step) {
    pinned.setDensity(kLevels[step]);
    app_.refresh();
    EXPECT_EQ(pinned.height(), Scaled(56 - 4 * step));
    EXPECT_EQ(pinned.getSuggestedMinimumDimensions().height(),
              Scaled(56 - 4 * step));
    EXPECT_EQ(sibling.height(), Scaled(36));
    EXPECT_FALSE(pinned.densityOverride().isInherited());
  }
  pinned.setDensity(Density::kDefault);
  changeShared(Density::kMinus2);
  EXPECT_EQ(pinned.height(), Scaled(56));
  EXPECT_EQ(sibling.height(), Scaled(48));
  pinned.clearDensityOverride();
  app_.refresh();
  EXPECT_TRUE(pinned.densityOverride().isInherited());
  EXPECT_EQ(pinned.height(), Scaled(48));
  task.navigation().clear();
}

using Row = ListRow<InvokableListItemBase>;

class RowModel : public DynamicListModel<Row> {
 public:
  int elementCount() const override { return 30; }

  void prepare(Row& row) const override { row.item().setHeadline("Device"); }

  void bind(int, Row& row) const override {
    row.item().setHeadline("Device");
    row.item().setOnInvoked([]() {});
  }

  DynamicListSectionState sectionState() const override {
    return {true, DynamicListFocusTarget::kRowSurface};
  }
};

class Section : public DynamicList<Row> {
 public:
  using DynamicList<Row>::DynamicList;
  using DynamicListBase::rowStride;
  using ListLayout::poolCapacity;

  Row* row(int index) { return static_cast<Row*>(materializedRow(index)); }
};

class WrappingList : public List {
 public:
  using List::List;

  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }
};

// Verifies list changes refresh static rows and the detached prototype without
// caller refresh, preserve focus/selection, and apply policy to recycled rows.
TEST_F(DensityOverrideTest, MixedListPrototypePoolAndRestoredInheritance) {
  RowModel model;
  Section section(context(), model);
  Row header(context(), "Header");
  WrappingList list(context());
  list.setVariant(ListVariant::kBaseline);
  ListSelectionPolicy selection;
  selection.mode = SelectionMode::kSingle;
  list.setSelectionPolicy(selection);
  header.setDensity(Density::kMinus5);
  list.add(header);
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Task& task = app_.addTaskFullScreen(scroll);
  app_.refresh();
  EXPECT_EQ(header.height(),
            Scaled(56));  // Owner supersedes standalone policy.
  ASSERT_TRUE(list.select(section, 1));
  Row* focused = section.row(1);
  ASSERT_NE(focused, nullptr);
  ASSERT_TRUE(focused->requestFocus());
  size_t original_capacity = section.poolCapacity();
  for (int step = 0; step <= 5; ++step) {
    list.setDensity(kLevels[step]);
    app_.refresh();
    EXPECT_EQ(header.height(), Scaled(56 - 4 * step));
    EXPECT_EQ(section.rowStride(), Scaled(56 - 4 * step));
    EXPECT_EQ(section.row(1), focused);
    EXPECT_TRUE(focused->isFocused());
    EXPECT_EQ(list.selection().index, 1);
  }
  EXPECT_GT(section.poolCapacity(), original_capacity);
  scroll.scrollTo(0, -500);
  app_.refresh();
  for (int index = section.first(); index <= section.last(); ++index) {
    ASSERT_NE(section.row(index), nullptr);
    EXPECT_EQ(section.row(index)->height(), Scaled(36));
    EXPECT_EQ(section.row(index)->densityOverride(), list.densityOverride());
  }
  list.setDensity(Density::kDefault);
  changeShared(Density::kMinus2);
  EXPECT_EQ(header.height(), Scaled(56));
  EXPECT_EQ(section.rowStride(), Scaled(56));
  list.clearDensityOverride();
  app_.refresh();
  EXPECT_EQ(header.height(), Scaled(48));
  EXPECT_EQ(section.rowStride(), Scaled(48));
  EXPECT_EQ(list.offsetTop(), -500);
  changeShared(Density::kMinus5);
  EXPECT_EQ(section.rowStride(), Scaled(36));
  task.navigation().clear();
}

// Verifies an owner override compacts row whitespace without changing a nested
// button's independent application density or explicit parent constraints.
TEST_F(DensityOverrideTest, SlotControlsAndExactConstraints) {
  Button button(context(), "Save");
  StandardListItem item(StandardListItemInit::OneLine("Row", &button));
  ListEntry row(context());
  row.setItem(item);
  row.setDensity(Density::kMinus5);
  EXPECT_EQ(
      row.measure(WidthSpec::Exactly(220), HeightSpec::Unspecified(0)).height(),
      Scaled(48) + button.getMargins().top() + button.getMargins().bottom());
  // The 40dp button keeps its own margins plus 4dp row padding per edge.
  EXPECT_EQ(button.getNaturalDimensions().height(), Scaled(40));
  EXPECT_EQ(
      row.measure(WidthSpec::Exactly(220), HeightSpec::Exactly(12)).height(),
      12);
}

// Verifies menu rows are explicitly zero by default, can opt into live shared
// density, and honor explicit choices in both visual families at every level.
TEST_F(DensityOverrideTest, MenuDefaultInheritanceAndExplicitLevels) {
  StandardMenuItem item(StandardMenuItemInit{"Menu item", {}});
  MenuEntry row(context());
  row.setMenuItem(item);
  material_.density = Density::kMinus5;
  EXPECT_FALSE(MenuPolicy{}.density.isInherited());
  EXPECT_FALSE(row.densityOverride().isInherited());
  EXPECT_EQ(
      row.measure(WidthSpec::Exactly(200), HeightSpec::Unspecified(0)).height(),
      std::max<YDim>(Scaled(56),
                     text_style_body_large().lineHeight() + Scaled(20)));
  row.clearDensityOverride();
  EXPECT_TRUE(row.densityOverride().isInherited());
  EXPECT_EQ(
      row.measure(WidthSpec::Exactly(200), HeightSpec::Unspecified(0)).height(),
      std::max<YDim>(Scaled(36),
                     text_style_body_large().lineHeight() + 2 * Scaled(4)));
  for (ListVariant variant :
       {ListVariant::kBaseline, ListVariant::kExpressive}) {
    ListEntryVisualContext visual = row.visualContext();
    visual.variant = variant;
    row.setVisualContext(visual);
    for (int step = 0; step <= 5; ++step) {
      row.setDensity(kLevels[step]);
      int padding = Scaled(
          std::max(4, (variant == ListVariant::kBaseline ? 8 : 10) - 2 * step));
      YDim expected =
          std::max<YDim>(Scaled(56 - 4 * step),
                         text_style_body_large().lineHeight() + 2 * padding);
      EXPECT_EQ(row.getSuggestedMinimumDimensions().height(), expected);
      EXPECT_EQ(row.measure(WidthSpec::Exactly(200), HeightSpec::Unspecified(0))
                    .height(),
                expected);
      row.layout(Rect(0, 0, 199, expected - 1));
      EXPECT_EQ(row.height(), expected);
      EXPECT_EQ(
          row.measure(WidthSpec::Exactly(7), HeightSpec::Exactly(9)).height(),
          9);
    }
  }
}

// Verifies root refresh reaches already-open inherited menu rows, updating
// geometry and hit positions while preserving the allocated presentation
// bounds.
TEST_F(DensityOverrideTest, OpenInheritedMenuRefreshesWithSharedTheme) {
  Button source(context(), "Menu");
  Task& task = app_.addTaskFullScreen(source);
  app_.refresh();
  MenuRow<StandardMenuItem> first(context(), StandardMenuItemInit{"First", {}});
  MenuRow<StandardMenuItem> second(context(),
                                   StandardMenuItemInit{"Second", {}});
  MenuGroup group(context());
  group.add(first);
  group.add(second);
  Menu menu(context());
  menu.addGroup(group);
  MenuPolicy policy;
  policy.density = DensityOverride{};
  menu.setPolicy(policy);
  ASSERT_EQ(menu.showFromRect(task, Rect(20, 20, 59, 39)),
            MenuShowResult::kShown);
  app_.refresh();
  EXPECT_EQ(first.height(), Scaled(56));
  changeShared(Density::kMinus5);
  EXPECT_EQ(first.height(), Scaled(36));
  EXPECT_EQ(second.height(), Scaled(36));
  EXPECT_EQ(second.offsetTop(), Scaled(36) + Scaled(2));
  std::vector<Widget*> path;
  EXPECT_TRUE(first.fillTouchTargetPath(5, first.height() - 1, path));
  EXPECT_FALSE(first.fillTouchTargetPath(5, first.height(), path));
  changeShared(Density::kDefault);
  EXPECT_EQ(first.height(), Scaled(56));
  EXPECT_EQ(second.offsetTop(), Scaled(56) + Scaled(2));
  menu.dismissChain();
  task.navigation().clear();
}

class TallMenuSlot : public Widget {
 public:
  explicit TallMenuSlot(ApplicationContext& context) : Widget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return {Scaled(24), Scaled(96)};
  }
};

// Verifies tall content sets menu floors while shortcut, badge, checkmark and
// trailing icon slots retain space; compact rows obey exact parent constraints.
TEST_F(DensityOverrideTest, MenuTallSlotsAndTrailingContentFloors) {
  TallMenuSlot slot(context());
  StandardMenuItem item(
      StandardMenuItemInit{"", {}, &slot, StandardMenuItemFlags::kSelectable});
  item.setShortcut("Ctrl+S");
  item.setBadgeValue(23);
  MenuEntry row(context());
  row.setMenuItem(item);
  for (ListVariant variant :
       {ListVariant::kBaseline, ListVariant::kExpressive}) {
    ListEntryVisualContext visual = row.visualContext();
    visual.variant = variant;
    row.setVisualContext(visual);
    for (int step = 0; step <= 5; ++step) {
      row.setDensity(kLevels[step]);
      int padding = Scaled(
          std::max(4, (variant == ListVariant::kBaseline ? 8 : 10) - 2 * step));
      EXPECT_EQ(row.measure(WidthSpec::Exactly(220), HeightSpec::Unspecified(0))
                    .height(),
                Scaled(96) + 2 * padding);
    }
  }
  StandardMenuItem trailing_only(StandardMenuItemInit{
      "", {}, nullptr, StandardMenuItemFlags::kSelectable});
  trailing_only.setShortcut("Ctrl+S");
  trailing_only.setBadgeValue(23);
  trailing_only.setTrailingIcon(&ic_filled_24_navigation_chevron_right());
  row.setMenuItem(trailing_only);
  row.setDensity(Density::kMinus5);
  YDim expected =
      std::max({static_cast<YDim>(Scaled(36)),
                static_cast<YDim>(Scaled(24) + 2 * Scaled(4)),
                text_style_label_large().lineHeight() + 2 * Scaled(4)});
  EXPECT_EQ(
      row.measure(WidthSpec::Exactly(220), HeightSpec::Unspecified(0)).height(),
      expected);
  row.layout(Rect(0, 0, 219, expected - 1));
}

}  // namespace
}  // namespace roo_windows::material3
