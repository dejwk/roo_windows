#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/child_layout.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows/material3/list/internal/list_row_geometry.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows::material3 {
namespace {
using internal::ListRowGeometry;
using internal::ListRowGeometryInput;
using internal::ListRowLayoutMetrics;
using internal::ListRowTokens;

ListRowGeometryInput Input(ListVariant variant, uint8_t lines = 1) {
  return {variant,
          lines,
          false,
          VerticalVisualAlignment::kMiddle,
          VerticalVisualAlignment::kMiddle,
          Scaled(24),
          {Scaled(32), Scaled(32)},
          {},
          {}};
}

// Verifies all six levels against independent band/padding expectations for
// baseline and expressive rows, with the figure's leading-slot content floor.
TEST(ListDensityGeometry, BandsPaddingAndPreservedTokens) {
  const int bands[3][6] = {{56, 52, 48, 44, 40, 40},
                           {72, 68, 64, 60, 56, 52},
                           {88, 84, 80, 76, 72, 72}};
  const int padding[2][6] = {{8, 6, 4, 4, 4, 4}, {10, 8, 6, 4, 4, 4}};
  for (ListVariant variant :
       {ListVariant::kBaseline, ListVariant::kExpressive}) {
    for (int lines = 1; lines <= 3; ++lines) {
      ListRowGeometryInput input = Input(variant, lines);
      input.text_height = Scaled(lines == 1 ? 24 : lines == 2 ? 44 : 64);
      for (int step = 0; step <= 5; ++step) {
        SCOPED_TRACE(::testing::Message()
                     << "lines=" << lines << " step=" << step);
        ListRowTokens tokens = internal::ResolveListRowTokens(variant, -step);
        ListRowGeometry geometry =
            internal::ResolveListRowGeometry(input, -step);
        EXPECT_EQ(geometry.band_height, Scaled(bands[lines - 1][step]));
        EXPECT_EQ(geometry.height, geometry.band_height);
        EXPECT_EQ(
            tokens.vertical_padding,
            Scaled(padding[variant == ListVariant::kBaseline ? 0 : 1][step]));
        EXPECT_EQ(tokens.horizontal_padding, Scaled(16));
        EXPECT_EQ(tokens.slot_gap,
                  Scaled(variant == ListVariant::kBaseline ? 16 : 12));
        EXPECT_EQ(tokens.body_gap, Scaled(8));
        EXPECT_EQ(tokens.body_bottom_padding,
                  Scaled(variant == ListVariant::kBaseline ? 8 : 10));
        EXPECT_GE(geometry.text_y, tokens.vertical_padding);
        EXPECT_GE(geometry.band_height - geometry.text_y - input.text_height,
                  tokens.vertical_padding);
      }
    }
  }
}

// Verifies tall slots and appended bodies dominate natural size without
// compacting body spacing, and top alignment still chooses the three-line band.
TEST(ListDensityGeometry, TallSlotsBodiesAndTopAlignment) {
  for (ListVariant variant :
       {ListVariant::kBaseline, ListVariant::kExpressive}) {
    ListRowGeometryInput input = Input(variant);
    input.leading = {Scaled(40), Scaled(100)};
    input.trailing = {Scaled(24), Scaled(80)};
    input.body = {Scaled(150), Scaled(60)};
    for (int step = 0; step <= 5; ++step) {
      ListRowTokens tokens = internal::ResolveListRowTokens(variant, -step);
      ListRowGeometry geometry = internal::ResolveListRowGeometry(input, -step);
      EXPECT_EQ(geometry.leading_y, tokens.vertical_padding);
      EXPECT_EQ(geometry.band_height,
                input.leading.height() + 2 * tokens.vertical_padding);
      EXPECT_EQ(geometry.body_y - geometry.band_height, Scaled(8));
      EXPECT_EQ(geometry.height - geometry.body_y - input.body.height(),
                tokens.body_bottom_padding);
    }
    input = Input(variant);
    input.top_text = true;
    ListRowGeometry geometry = internal::ResolveListRowGeometry(input, -5);
    EXPECT_EQ(geometry.band_height, Scaled(68));
    EXPECT_EQ(geometry.text_y, Scaled(4));
    EXPECT_EQ(geometry.leading_y, Scaled(4));
    input.top_text = false;
    input.trailing_alignment = VerticalVisualAlignment::kTop;
    input.trailing = {Scaled(16), Scaled(16)};
    geometry = internal::ResolveListRowGeometry(input, -5);
    EXPECT_EQ(geometry.trailing_y, Scaled(4));
    input.body = {0, 40000};
    EXPECT_GT(internal::ResolveListRowGeometry(input, -5).height, 40000);
  }
}

// Verifies specialized row token and owner-painted content floors enter the
// shared geometry before slot positioning and exact parent constraints.
TEST(ListDensityGeometry, SpecializedBandAndContentFloors) {
  ListRowGeometryInput input = Input(ListVariant::kBaseline);
  input.minimum_band_height = Scaled(100);
  ListRowGeometry geometry = internal::ResolveListRowGeometry(input, -5);
  EXPECT_EQ(geometry.band_height, Scaled(100));
  EXPECT_EQ(geometry.text_y, (Scaled(100) - Scaled(24)) / 2);
  input.extra_content_height = Scaled(120);
  geometry = internal::ResolveListRowGeometry(input, -5);
  EXPECT_EQ(geometry.band_height, Scaled(120) + 2 * Scaled(4));
  EXPECT_EQ(geometry.leading_y, (geometry.band_height - Scaled(32)) / 2);
}

class Slot : public Widget {
 public:
  Slot(ApplicationContext& context, YDim height)
      : Widget(context), height_(height) {}
  Margins getMargins() const override { return Margins(3, 5, 3, 5); }
  Dimensions getSuggestedMinimumDimensions() const override { return {20, 1}; }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    return {width.resolveSize(20), height.resolveSize(height_)};
  }

 private:
  YDim height_;
};

using ListDensityTest = test_support::RooWindowsRenderTestSized<240, 320>;

// Verifies real control margins, measured custom slots, multiline text, and
// remeasured appended bodies use the same geometry for measurement and
// placement.
TEST_F(ListDensityTest, ActualRowSlotsAndVariantStyles) {
  for (ListVariant variant :
       {ListVariant::kBaseline, ListVariant::kExpressive}) {
    for (ListStyle style : {ListStyle::kStandard, ListStyle::kSegmented}) {
      Slot leading(context(), Scaled(90));
      Slot trailing(context(), Scaled(65));
      Slot body(context(), Scaled(30));
      StandardListItemInit init = StandardListItemInit::TwoLine(
          "Headline", "Two lines of supporting content");
      init.leading = &leading;
      init.trailing = &trailing;
      init.body = &body;
      init.supporting_policy.max_lines = 2;
      StandardListItem item(init);
      ListEntry row(context());
      row.setItem(item);
      ListEntryVisualContext visual;
      visual.variant = variant;
      visual.style = style;
      row.setVisualContext(visual);
      for (int step = 0; step <= 5; ++step) {
        ListRowLayoutMetrics layout = internal::ResolveListRowLayout(
            row, WidthSpec::Exactly(220), HeightSpec::Unspecified(0), -step);
        EXPECT_EQ(layout.leading.height(), Scaled(90) + 10);
        EXPECT_EQ(layout.body.height(), Scaled(30) + 10);
        EXPECT_EQ(layout.row_band_height,
                  layout.leading.height() +
                      2 * internal::ResolveListRowTokens(variant, -step)
                              .vertical_padding);
        EXPECT_EQ(layout.height - layout.body_y - layout.body.height(),
                  Scaled(variant == ListVariant::kBaseline ? 8 : 10));
        row.layout(Rect(0, 0, 219, layout.height - 1));
        internal::LayoutListRow(row, layout);
        EXPECT_EQ(leading.offsetTop(), layout.leading_y + 5);
        EXPECT_EQ(trailing.offsetTop(), layout.trailing_y + 5);
        EXPECT_EQ(body.offsetTop(), layout.body_y + 5);
        EXPECT_EQ(leading.offsetLeft(), Scaled(16) + 3);
        EXPECT_EQ(internal::ResolveListRowLayout(row, WidthSpec::Exactly(7),
                                                 HeightSpec::Exactly(5), -step)
                      .height,
                  5);
      }
      EXPECT_EQ(row.measure(WidthSpec::Exactly(220), HeightSpec::Unspecified(0))
                    .height(),
                internal::ResolveListRowLayout(row, WidthSpec::Exactly(220),
                                               HeightSpec::Unspecified(0), 0)
                    .height);
    }
  }
}

class InspectableRow : public ListEntry {
 public:
  using ListEntry::getChild;
  using ListEntry::getChildrenCount;
  using ListEntry::ListEntry;
};

// Verifies a wrapped multiline text stack itself sets the compact floor and
// every placed text line fits inside the row's resolved vertical padding.
TEST_F(ListDensityTest, MeasuredMultilineTextSetsContentFloor) {
  for (ListVariant variant :
       {ListVariant::kBaseline, ListVariant::kExpressive}) {
    StandardListItemInit init = StandardListItemInit::TwoLine(
        "Headline",
        "Supporting text long enough to wrap across three lines at "
        "the constrained column width, with more words beyond the limit.");
    init.supporting_policy.max_lines = 3;
    init.supporting_policy.overflow = TextOverflowPolicy::kWrap;
    StandardListItem item(init);
    InspectableRow row(context());
    row.setItem(item);
    ListEntryVisualContext visual;
    visual.variant = variant;
    row.setVisualContext(visual);
    for (int step = 1; step <= 5; ++step) {
      ListRowLayoutMetrics layout =
          internal::ResolveListRowLayout(row, WidthSpec::Exactly(Scaled(120)),
                                         HeightSpec::Unspecified(0), -step);
      ASSERT_EQ(row.getChildrenCount(), 2);
      YDim actual_text_height = 0;
      for (int child = 0; child < row.getChildrenCount(); ++child) {
        actual_text_height +=
            MeasureChildWithMargins(row.getChild(child),
                                    WidthSpec::AtMost(layout.text_width),
                                    HeightSpec::Unspecified(0))
                .height();
      }
      EXPECT_EQ(layout.text.height, actual_text_height);
      EXPECT_GT(actual_text_height, text_style_body_large().lineHeight());
      row.layout(Rect(0, 0, Scaled(120) - 1, layout.height - 1));
      internal::LayoutListRow(row, layout);
      int padding =
          internal::ResolveListRowTokens(variant, -step).vertical_padding;
      EXPECT_GE(row.getChild(0).offsetTop(), padding);
      EXPECT_LE(row.getChild(1).parent_bounds().yMax(),
                layout.row_band_height - padding - 1);
    }
  }
}

// Verifies growth normalizes a wrapped live interval without releasing,
// reordering, or replacing its widgets, and newly allocated slots remain
// usable.
TEST_F(ListDensityTest, WrappedRecyclerGrowthRetainsOrder) {
  CircularBuffer buffer(context());
  std::vector<Widget*> allocated;
  std::function<std::unique_ptr<Widget>()> factory = [&]() {
    auto row = std::make_unique<Slot>(context(), 10);
    allocated.push_back(row.get());
    return row;
  };
  buffer.ensure_capacity(5, factory);
  for (int i = 0; i < 5; ++i) buffer.push_back();
  buffer.pop_front();
  buffer.pop_front();
  buffer.push_back();
  buffer.push_back();
  buffer.ensure_capacity(8, factory);
  ASSERT_EQ(buffer.count(), 5u);
  ASSERT_EQ(buffer.capacity(), 8u);
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(&buffer[i], allocated[(i + 2) % 5]);
  }
  EXPECT_EQ(&buffer.pop_front(), allocated[2]);
  EXPECT_EQ(&buffer.push_back(), allocated[5]);
}

// Test-only row injects the internal level without publishing theme storage or
// adding state to production rows. The same resolver places its actual text.
class DensityRow : public ListRow<InvokableListItemBase> {
 public:
  DensityRow(ApplicationContext& context, int8_t& level)
      : ListRow<InvokableListItemBase>(context), level_(level) {}
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }

 protected:
  bool retainsTextSlots() const override { return true; }
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    ListRowLayoutMetrics layout =
        internal::ResolveListRowLayout(*this, width, height, level_);
    return {layout.width, layout.height};
  }
  void onLayout(bool, const Rect& rect) override {
    internal::LayoutListRow(*this,
                            internal::ResolveListRowLayout(
                                *this, WidthSpec::Exactly(rect.width()),
                                HeightSpec::Exactly(rect.height()), level_));
  }

 private:
  int8_t& level_;
};

class DensityModel : public DynamicListModel<DensityRow> {
 public:
  int elementCount() const override { return 20; }
  mutable int releases = 0;
  void prepare(DensityRow& row) const override {
    row.item().setHeadline("Row");
  }
  void bind(int index, DensityRow& row) const override {
    row.item().setHeadline("Row");
    row.item().setOnInvoked([]() {});
  }
  void unbind(DensityRow&) const override { ++releases; }
  DynamicListSectionState sectionState() const override {
    return {true, DynamicListFocusTarget::kRowSurface};
  }
};

class DensitySection : public DynamicList<DensityRow> {
 public:
  using DynamicList<DensityRow>::DynamicList;
  using DynamicListBase::rowStride;
  using ListLayout::poolCapacity;
  DensityRow* row(int index) {
    return static_cast<DensityRow*>(materializedRow(index));
  }
};

class WrappingList : public List {
 public:
  using List::List;
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
  }
};

// Verifies retained section extent/positions are remeasured without model
// reset, pool growth preserves live focus, selection stays logical, and
// scrolling is preserved then clamped after compaction. Offscreen rows bind
// current geometry.
TEST_F(ListDensityTest, MixedSectionsRelayoutRecyclingFocusAndScrollClamping) {
  int8_t level = 0;
  DensityModel first_model;
  DensityModel second_model;
  DensityRow header(context(), level);
  DensityRow footer(context(), level);
  header.item().setHeadline("Header");
  footer.item().setHeadline("Footer");
  DensitySection first(context(), first_model, [&]() {
    return std::make_unique<DensityRow>(context(), level);
  });
  DensitySection second(context(), second_model, [&]() {
    return std::make_unique<DensityRow>(context(), level);
  });
  WrappingList list(context());
  list.setVariant(ListVariant::kBaseline);
  ListSelectionPolicy selection;
  selection.mode = SelectionMode::kSingle;
  list.setSelectionPolicy(selection);
  list.add(header);
  list.add(first);
  list.add(second);
  list.add(footer);
  SimpleScrollablePanel scroll(context(), list);
  Task& task = app_.addTaskFullScreen(scroll);
  refresh();
  ASSERT_NE(first.row(1), nullptr);
  DensityRow* focused = first.row(1);
  ASSERT_TRUE(focused->requestFocus());
  ASSERT_TRUE(list.select(first, 1));
  const size_t old_capacity = first.poolCapacity();
  for (int8_t next : {int8_t(-2), int8_t(-5), int8_t(0)}) {
    level = next;
    scroll.requestLayoutDescending();
    scroll.invalidateInterior();
    refresh();
    int height = Scaled(next == 0 ? 56 : next == -2 ? 48 : 36);
    EXPECT_EQ(first.rowStride(), height);
    EXPECT_EQ(first.height(), 20 * height);
    EXPECT_EQ(second.offsetTop(), 21 * height);
    EXPECT_EQ(footer.offsetTop(), 41 * height);
    EXPECT_EQ(list.height(), 42 * height);
    EXPECT_EQ(list.selection().section, &first);
    EXPECT_EQ(list.selection().index, 1);
    EXPECT_EQ(first.row(1), focused);
    EXPECT_TRUE(focused->isFocused());
    EXPECT_TRUE(first.row(1)->visualContext().selected);
    if (next != 0) {
      EXPECT_EQ(first_model.releases, 0);
    }
    if (next == -5) {
      EXPECT_GT(first.poolCapacity(), old_capacity);
    }
  }
  scroll.scrollTo(0, -500);
  refresh();
  level = -5;
  scroll.requestLayoutDescending();
  refresh();
  EXPECT_EQ(list.offsetTop(), -500);
  for (int index = first.first(); index <= first.last(); ++index) {
    ASSERT_NE(first.row(index), nullptr);
    EXPECT_EQ(first.row(index)->height(), Scaled(36));
    EXPECT_EQ(first.row(index)->offsetTop(), index * Scaled(36));
  }
  level = 0;
  scroll.requestLayoutDescending();
  refresh();
  scroll.scrollTo(0, -100000);
  refresh();
  level = -5;
  scroll.requestLayoutDescending();
  refresh();
  EXPECT_EQ(list.offsetTop(), 320 - 42 * Scaled(36));
  EXPECT_EQ(footer.offsetTop(), 41 * Scaled(36));
  ASSERT_NE(second.row(19), nullptr);
  EXPECT_EQ(second.row(19)->height(), Scaled(36));
  EXPECT_EQ(list.selection().section, &first);
  EXPECT_EQ(list.selection().index, 1);
  task.navigation().clear();
}

}  // namespace
}  // namespace roo_windows::material3
