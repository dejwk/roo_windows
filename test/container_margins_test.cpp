#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/blit_cache_container.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/holder.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/stacked_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/child_layout.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/dialog/dialog_scaffold.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/menu/menu_surface.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"
#include "roo_windows/material3/navigation_rail/navigation_rail.h"
#include "roo_windows/material3/tabs/tabs.h"

namespace roo_windows {
namespace {

class Probe : public Widget {
 public:
  explicit Probe(ApplicationContext& context, XDim width = 20, YDim height = 10)
      : Widget(context), natural(width, height) {}

  Margins getMargins() const override { return margins; }
  Dimensions getSuggestedMinimumDimensions() const override { return natural; }
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::WrapContentWidth(),
            PreferredSize::WrapContentHeight()};
  }
  Margins margins = Margins(3, 5);
  Dimensions natural;
  Dimensions measured;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    measured = Dimensions(width.resolveSize(natural.width()),
                          height.resolveSize(natural.height()));
    return measured;
  }
};

template <typename Base>
class WithMargins : public Base {
 public:
  using Base::Base;
  Margins getMargins() const override { return Margins(3, 5); }
};

class ContainerMargins : public testing::Test {
 protected:
  ContainerMargins()
      : env(scheduler),
        context(env.scheduler(), env.theme(), env.keyboardColorTheme()) {}

  void layout(Widget& widget, XDim width, YDim height) {
    widget.measure(WidthSpec::Exactly(width), HeightSpec::Exactly(height));
    widget.layout(Rect(0, 0, width - 1, height - 1));
  }

  void expectMeasured(const Probe& widget, XDim width, YDim height) {
    EXPECT_EQ(width, widget.measured.width());
    EXPECT_EQ(height, widget.measured.height());
  }

  roo_scheduler::Scheduler scheduler;
  Environment env;
  ApplicationContext context;
};

// Verifies loose constraints include margins, exact constraints inset the
// child, and exhausted/negative margins cannot revive an empty slot.
TEST_F(ContainerMargins, MeasurementAndEmptySlots) {
  Probe child(context);
  Dimensions size = MeasureChildWithMargins(child, WidthSpec::AtMost(100),
                                            HeightSpec::Unspecified(0));
  EXPECT_EQ(26, size.width());
  EXPECT_EQ(20, size.height());
  expectMeasured(child, 20, 10);
  MeasureChildWithMargins(child, WidthSpec::Exactly(4), HeightSpec::Exactly(6));
  expectMeasured(child, 0, 0);
  LayoutChildWithMargins(child, Rect(0, 0, 3, 5));
  EXPECT_TRUE(child.parent_bounds().empty());
  child.margins = Margins(-2, -3);
  MeasureChildWithMargins(child, WidthSpec::Exactly(20),
                          HeightSpec::Exactly(10));
  expectMeasured(child, 24, 16);
  LayoutChildWithMargins(child, Rect(0, 0, 19, 9));
  EXPECT_EQ(Rect(-2, -3, 21, 12), child.parent_bounds());
  LayoutChildWithMargins(child, Rect(0, 0, -1, -1));
  EXPECT_TRUE(child.parent_bounds().empty());
}

// Verifies overlap and cached wrappers use the same margin-inclusive dimensions
// for loose measurement and the same inset for exact placement.
TEST_F(ContainerMargins, StackedAndCachedWrappers) {
  for (bool cached : {false, true}) {
    Probe child(context);
    StackedLayout stack(context);
    BlitCacheContainer cache(context);
    Widget& wrapper = cached ? static_cast<Widget&>(cache) : stack;
    if (cached)
      cache.setChild(child);
    else
      stack.add(child);
    Dimensions size =
        wrapper.measure(WidthSpec::AtMost(100), HeightSpec::AtMost(80));
    EXPECT_EQ(26, size.width());
    EXPECT_EQ(20, size.height());
    layout(wrapper, 100, 80);
    expectMeasured(child, 94, 70);
    EXPECT_EQ(Rect(3, 5, 96, 74), child.parent_bounds());
    child.setVisibility(Visibility::kGone);
    layout(wrapper, 100, 80);
    EXPECT_TRUE(child.parent_bounds().empty());
  }
}

// Verifies page measurement and the active page wrapper agree about margins.
TEST_F(ContainerMargins, HorizontalPages) {
  Probe page(context);
  HorizontalPageHost host(context);
  host.addPage(page);
  Dimensions size =
      host.measure(WidthSpec::AtMost(100), HeightSpec::AtMost(80));
  EXPECT_EQ(26, size.width());
  EXPECT_EQ(20, size.height());
  layout(host, 100, 80);
  expectMeasured(page, 94, 70);
  EXPECT_EQ(Rect(3, 5, 96, 74), page.parent_bounds());
}

// Verifies the established generic layouts retain their margin handling.
TEST_F(ContainerMargins, ExistingGenericLayouts) {
  for (int kind = 0; kind < 5; ++kind) {
    Probe child(context);
    AlignedLayout aligned(context);
    HorizontalLayout horizontal(context);
    VerticalLayout vertical(context);
    FlexLayout flex(context);
    Holder holder(context);
    Widget* parent = nullptr;
    switch (kind) {
      case 0:
        aligned.add(child);
        parent = &aligned;
        break;
      case 1:
        horizontal.add(child);
        parent = &horizontal;
        break;
      case 2:
        vertical.add(child);
        parent = &vertical;
        break;
      case 3:
        flex.add(child);
        parent = &flex;
        break;
      default:
        holder.setContents(child);
        parent = &holder;
        break;
    }
    const Dimensions size =
        parent->measure(WidthSpec::AtMost(100), HeightSpec::AtMost(80));
    EXPECT_EQ(26, size.width()) << kind;
    EXPECT_EQ(20, size.height()) << kind;
    parent->layout(Rect(0, 0, 25, 19));
    EXPECT_EQ(Rect(3, 5, 22, 14), child.parent_bounds()) << kind;
  }
}

// Verifies the task root receives the screen area inside its own margins.
TEST_F(ContainerMargins, TaskRoot) {
  roo::byte raster[100 * 80 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      100, 80, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  std::unique_ptr<Probe> root;
  {
    Application app(&env, display);
    root = std::make_unique<Probe>(app.context());
    app.addTaskFullScreen(*root);
    ASSERT_TRUE(app.refresh());
    EXPECT_EQ(Rect(3, 5, 96, 74), root->parent_bounds());
    expectMeasured(*root, 94, 70);
  }
}

// Verifies pane slots inset the active pane and clear inactive geometry.
TEST_F(ContainerMargins, Panes) {
  Probe main(context);
  Probe side(context);
  material3::PaneLayout panes(context);
  panes.setMainPane(main);
  panes.setLeadingPane(side);
  layout(panes, 100, 80);
  expectMeasured(main, 94, 70);
  EXPECT_EQ(Rect(3, 5, 96, 74), main.parent_bounds());
  EXPECT_TRUE(side.parent_bounds().empty());
  panes.setActivePane(material3::PaneRole::kLeading);
  layout(panes, 100, 80);
  EXPECT_TRUE(main.parent_bounds().empty());
  EXPECT_EQ(Rect(3, 5, 96, 74), side.parent_bounds());
}

// Verifies children stay in container-local coordinates when an outer layout
// positions an adaptive container at a nonzero origin (for example, margins).
TEST_F(ContainerMargins, NestedAdaptiveLayoutsUseLocalCoordinates) {
  for (int kind = 0; kind < 3; ++kind) {
    Probe child(context);
    material3::LayoutScaffold scaffold(context);
    material3::PaneLayout panes(context);
    material3::GridLayout grid(context);
    material3::LayoutBreakpointPolicy policy({}, {1, 0, 0}, {1, 0, 0},
                                             {1, 0, 0}, {1, 0, 0}, {1, 0, 0});
    grid.setBreakpointPolicy(policy);
    Widget* parent = nullptr;
    if (kind == 0) {
      scaffold.setBody(child);
      parent = &scaffold;
    }
    if (kind == 1) {
      panes.setMainPane(child);
      parent = &panes;
    }
    if (kind == 2) {
      grid.add(child);
      parent = &grid;
    }
    parent->measure(WidthSpec::Exactly(100), HeightSpec::Exactly(80));
    parent->layout(Rect(20, 30, 119, 109));
    EXPECT_EQ(3, child.offsetLeft()) << kind;
    EXPECT_EQ(5, child.offsetTop()) << kind;
    EXPECT_EQ(94, child.width()) << kind;
  }
  Probe header(context);
  material3::NavigationRail rail(context);
  rail.setHeader(header);
  rail.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(80));
  static_cast<Widget&>(rail).layout(Rect(20, 30, 119, 109));
  EXPECT_EQ(40, header.offsetLeft());
  EXPECT_EQ(Scaled(4) + 5, header.offsetTop());
}

// Verifies row height includes margins, gravity aligns the occupied rectangle,
// and RTL reverses columns while preserving physical child margins.
TEST_F(ContainerMargins, GridRowsAndGravity) {
  Probe short_child(context, 20, 10);
  Probe tall_child(context, 20, 30);
  Probe next(context, 20, 8);
  material3::LayoutBreakpointPolicy policy({}, {2, 0, 0}, {2, 0, 0}, {2, 0, 0},
                                           {2, 0, 0}, {2, 0, 0});
  material3::GridLayout grid(context);
  grid.setBreakpointPolicy(policy);
  grid.setRowGapDp(0);
  material3::GridLayout::Params params;
  params.span = {1, 1, 1, 1, 1};
  params.gravity = kGravityBottom;
  grid.add(short_child, params);
  grid.add(tall_child, params);
  grid.add(next, params);
  Dimensions size =
      grid.measure(WidthSpec::Exactly(100), HeightSpec::Unspecified(0));
  EXPECT_EQ(58, size.height());
  grid.layout(Rect(0, 0, 99, 57));
  expectMeasured(short_child, 44, 10);
  EXPECT_EQ(Rect(3, 25, 46, 34), short_child.parent_bounds());
  EXPECT_EQ(Rect(53, 5, 96, 34), tall_child.parent_bounds());
  EXPECT_EQ(Rect(3, 45, 46, 52), next.parent_bounds());
  grid.setLayoutDirection(LayoutDirection::kRightToLeft);
  layout(grid, 100, 58);
  EXPECT_EQ(Rect(53, 25, 96, 34), short_child.parent_bounds());
}

// Verifies app-bar fixed slots and search-bar natural slots include margins
// without changing the child's natural surface size.
TEST_F(ContainerMargins, AppAndSearchBars) {
  {
    Probe action(context, Scaled(48), Scaled(48));
    material3::AppBar bar(context);
    bar.setLeading(action);
    layout(bar, 300, Scaled(64));
    expectMeasured(action, Scaled(48) - 6, Scaled(48) - 10);
    EXPECT_EQ(Scaled(4) + 3, action.offsetLeft());
    EXPECT_EQ((Scaled(64) - Scaled(48)) / 2 + 5, action.offsetTop());
  }
  {
    Probe action(context);
    material3::SearchBar bar(context);
    bar.setLeading(action);
    layout(bar, 300, Scaled(56));
    expectMeasured(action, 20, 10);
    EXPECT_EQ(Scaled(24) + 3, action.offsetLeft());
    EXPECT_EQ((Scaled(56) - 10) / 2, action.offsetTop());
  }
  {
    Probe action(context);
    material3::SearchAppBar bar(context);
    bar.setLeading(action);
    layout(bar, 300, Scaled(64));
    expectMeasured(action, Scaled(48) - 6, Scaled(48) - 10);
    EXPECT_EQ(Scaled(4) + 3, action.offsetLeft());
    EXPECT_EQ(Scaled(48) - 6, action.width());
  }
}

// Verifies fixed tab slots inset their children and scrollable tabs advance by
// the complete occupied width on repeated layouts.
TEST_F(ContainerMargins, FixedAndScrollableTabs) {
  {
    WithMargins<material3::Tab> first(context, "One");
    WithMargins<material3::Tab> second(context, "Two");
    material3::Tabs tabs(context);
    tabs.addTab(first);
    tabs.addTab(second);
    layout(tabs, 200, Scaled(48));
    EXPECT_EQ(Rect(3, 5, 96, Scaled(48) - 6), first.parent_bounds());
    EXPECT_EQ(Rect(103, 5, 196, Scaled(48) - 6), second.parent_bounds());
  }
  {
    WithMargins<material3::Tab> first(context, "One");
    WithMargins<material3::Tab> second(context, "Two");
    material3::ScrollableTabs tabs(context);
    tabs.addTab(first);
    tabs.addTab(second);
    layout(tabs, 200, Scaled(48));
    EXPECT_EQ(first.parent_bounds().xMax() + 7, second.offsetLeft());
    const Rect before = second.parent_bounds();
    tabs.requestLayout();
    layout(tabs, 200, Scaled(48));
    EXPECT_EQ(before, second.parent_bounds());
  }
}

// Verifies navigation destinations and generic rail headers respect margins.
TEST_F(ContainerMargins, NavigationSlots) {
  {
    WithMargins<material3::NavigationBarDestination> first(context, "One");
    WithMargins<material3::NavigationBarDestination> second(context, "Two");
    material3::NavigationBar bar(context);
    ASSERT_TRUE(bar.add(first));
    ASSERT_TRUE(bar.add(second));
    layout(bar, 200, 80);
    EXPECT_EQ(Rect(3, 5, 96, 74), first.parent_bounds());
    EXPECT_EQ(Rect(103, 5, 196, 74), second.parent_bounds());
  }
  {
    Probe header(context);
    WithMargins<material3::NavigationRailDestination> item(context, "One");
    material3::NavigationRail rail(context);
    rail.setHeader(header);
    ASSERT_TRUE(rail.add(item));
    layout(rail, 100, 200);
    EXPECT_EQ(40, header.offsetLeft());
    EXPECT_EQ(10, header.height());
    EXPECT_EQ(3, item.offsetLeft());
    EXPECT_EQ(94, item.width());
    EXPECT_EQ(Scaled(56) - 10, item.height());
  }
}

// Verifies compressed rail slots stay within the available band and keep
// margins even when preferred destination heights do not fit.
TEST_F(ContainerMargins, NavigationRailUnderHeightPressure) {
  WithMargins<material3::NavigationRailDestination> first(context, "One");
  WithMargins<material3::NavigationRailDestination> second(context, "Two");
  material3::NavigationRail rail(context);
  ASSERT_TRUE(rail.add(first));
  ASSERT_TRUE(rail.add(second));
  layout(rail, 100, 80);
  EXPECT_EQ(Rect(3, 9, 96, 34), first.parent_bounds());
  EXPECT_EQ(Rect(3, 45, 96, 70), second.parent_bounds());
}

// Verifies list row margins affect list width/height and custom slot margins
// affect row measurement and placement without stretching the slot's surface.
TEST_F(ContainerMargins, ListsAndSlots) {
  Probe leading(context);
  Probe body(context);
  material3::StandardListItemInit init;
  init.leading = &leading;
  init.body = &body;
  material3::StandardListItem descriptor(init);
  WithMargins<material3::ListEntry> row(context);
  row.setItem(descriptor);
  material3::List list(context);
  list.add(row);
  layout(list, 200, 180);
  EXPECT_EQ(3, row.offsetLeft());
  EXPECT_EQ(5, row.offsetTop());
  EXPECT_EQ(194, row.width());
  EXPECT_EQ(20, leading.width());
  EXPECT_EQ(10, leading.height());
  EXPECT_EQ(Scaled(16) + 3, leading.offsetLeft());
  EXPECT_EQ(Scaled(16) + 3, body.offsetLeft());
  EXPECT_EQ(194 - 2 * Scaled(16) - 6, body.width());
}

// Verifies expansion includes content margins and removes content geometry
// when collapsed.
TEST_F(ContainerMargins, ExpandableContent) {
  Probe content(context);
  material3::ExpandablePanel panel(context);
  panel.setContent(content);
  panel.setExpanded(true, false);
  Dimensions size =
      panel.measure(WidthSpec::AtMost(100), HeightSpec::AtMost(80));
  EXPECT_EQ(26, size.width());
  EXPECT_EQ(20, size.height());
  layout(panel, 26, 20);
  EXPECT_EQ(Rect(3, 5, 22, 14), content.parent_bounds());
  panel.setExpanded(false, false);
  size = panel.measure(WidthSpec::AtMost(100), HeightSpec::AtMost(80));
  EXPECT_EQ(0, size.height());
  panel.layout(Rect(0, 0, 25, -1));
  EXPECT_TRUE(content.parent_bounds().empty());
}

// Verifies menu rows reserve vertical margins before positioning the next row.
TEST_F(ContainerMargins, MenuRows) {
  WithMargins<material3::MenuEntry> first(context);
  WithMargins<material3::MenuEntry> second(context);
  material3::MenuGroup group(context);
  group.add(first);
  group.add(second);
  layout(group, 200, 180);
  EXPECT_EQ(3, first.offsetLeft());
  EXPECT_EQ(5, first.offsetTop());
  EXPECT_EQ(194, first.width());
  EXPECT_GE(second.offsetTop(), first.parent_bounds().yMax() + 11);
}

class TestDialog : public material3::internal::DialogScaffold {
 public:
  TestDialog(ApplicationContext& context, Widget& body, Widget& chrome,
             material3::internal::DialogScaffoldVariant variant =
                 material3::internal::DialogScaffoldVariant::kBasic)
      : DialogScaffold(context, body, variant) {
    attachDerivedChrome(material3::internal::DialogChromeSlot::kSecondary,
                        chrome);
  }
  ~TestDialog() override { prepareForDerivedDestruction(); }
};

// Verifies dialog chrome reserves margins while the body scroller handles its
// own child's margins, including repeated layout.
TEST_F(ContainerMargins, DialogChromeAndBody) {
  Probe body(context);
  Probe chrome(context);
  TestDialog dialog(context, body, chrome);
  layout(dialog, 200, 180);
  EXPECT_EQ(Scaled(24) + 3, chrome.offsetLeft());
  EXPECT_EQ(10, chrome.height());
  EXPECT_EQ(180 - Scaled(24) - 5 - 1, chrome.parent_bounds().yMax());
  EXPECT_EQ(3, body.offsetLeft());
  EXPECT_EQ(5, body.offsetTop());
  const Rect before = body.parent_bounds();
  dialog.requestLayout();
  layout(dialog, 200, 180);
  EXPECT_EQ(before, body.parent_bounds());
}

// Verifies a naturally sized full-screen trailing control reserves its
// margins in addition to its natural width.
TEST_F(ContainerMargins, FullScreenDialogChrome) {
  Probe body(context);
  Probe chrome(context);
  TestDialog dialog(context, body, chrome,
                    material3::internal::DialogScaffoldVariant::kFullScreen);
  layout(dialog, 200, 180);
  expectMeasured(chrome, 20, Scaled(48) - 10);
  EXPECT_EQ(200 - Scaled(4) - 3 - 1, chrome.parent_bounds().xMax());
  EXPECT_EQ(20, chrome.width());
}

// Verifies re-layout preserves a scrolled origin instead of adding margins
// again, and alignment uses the viewport remaining inside the margins.
TEST_F(ContainerMargins, ScrollingAndAlignment) {
  Probe content(context, 20, 200);
  SimpleScrollablePanel panel(context, content);
  layout(panel, 100, 80);
  panel.scrollTo(0, -20);
  EXPECT_EQ(-15, content.offsetTop());
  panel.requestLayout();
  layout(panel, 100, 80);
  EXPECT_EQ(-15, content.offsetTop());
  content.natural = Dimensions(20, 10);
  content.requestLayout();
  panel.setAlign(roo_display::kCenter | roo_display::kMiddle);
  layout(panel, 100, 80);
  EXPECT_EQ(Rect(40, 35, 59, 44), content.parent_bounds());
}

}  // namespace
}  // namespace roo_windows
