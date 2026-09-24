#include <chrono>
#include <functional>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace material3 {
namespace {

using Row = ListRow<InvokableListItemBase>;

class Model : public DynamicListModel<Row> {
 public:
  int count = 20;
  bool enabled = true;
  bool selected = false;
  DynamicListFocusTarget focus = DynamicListFocusTarget::kRowSurface;
  mutable int reads = 0;
  mutable int binds = 0;
  mutable int releases = 0;
  mutable int prepares = 0;
  mutable int invokes = 0;
  std::function<void()> on_release;
  std::string text = "Device";

  int elementCount() const override {
    ++reads;
    return count;
  }
  void prepare(Row& row) const override {
    ++prepares;
    row.item().setHeadline("Prototype");
  }
  void bind(int index, Row& row) const override {
    EXPECT_GE(index, 0);
    EXPECT_LT(index, count);
    ++binds;
    row.item().setHeadline(text);
    row.item().setOnInvoked([this]() { ++invokes; });
  }
  void unbind(Row& row) const override {
    ++releases;
    row.item().setHeadline({});
    row.item().setOnInvoked({});
    if (on_release) on_release();
  }
  DynamicListRowState rowState(int index) const override {
    EXPECT_GE(index, 0);
    EXPECT_LT(index, count);
    ++reads;
    return {selected, {16, 8}};
  }
  DynamicListSectionState sectionState() const override {
    ++reads;
    return {enabled, focus};
  }
};

class Section : public DynamicList<Row> {
 public:
  using DynamicList<Row>::DynamicList;
  using ListLayout::poolCapacity;
  Row* row(int index) { return static_cast<Row*>(materializedRow(index)); }
};

class ObservingList : public List {
 public:
  using List::List;
  int changes = 0;
  std::function<void(ListRowLocation)> changed;

 protected:
  void onSingleSelectionChanged(ListRowLocation location) override {
    ++changes;
    // Copying is test-only; callbacks may replace this test hook reentrantly.
    auto callback = changed;
    if (callback) callback(location);
  }
};

class Mount {
 public:
  Mount(Application& app, Widget& widget)
      : task_(app.addTaskFullScreen(widget)) {}
  ~Mount() { task_.navigation().clear(); }

 private:
  Task& task_;
};

using DynamicListTest = test_support::RooWindowsRenderTestSized<240, 320>;

ListSelectionPolicy SingleSelection() {
  ListSelectionPolicy policy;
  policy.mode = SelectionMode::kSingle;
  return policy;
}

// Verifies global positions and geometry skip empty sections and retain exactly
// one band at each seam, including when only a dynamic row remains.
TEST_F(DynamicListTest, MixedSectionsFlattenAndCollapse) {
  Model model;
  model.count = 2;
  Model empty;
  empty.count = 0;
  Section dynamic(context(), model);
  Section zero(context(), empty);
  Row first(context(), "First");
  Row last(context(), "Last");
  List list(context());
  list.setStyle(ListStyle::kSegmented);
  list.add(first);
  list.add(dynamic);
  list.add(zero);
  list.add(last);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  ASSERT_NE(dynamic.row(0), nullptr);
  ASSERT_NE(dynamic.row(1), nullptr);
  EXPECT_EQ(first.visualContext().position, ListItemPosition::kFirst);
  EXPECT_EQ(dynamic.row(0)->visualContext().position,
            ListItemPosition::kMiddle);
  EXPECT_EQ(last.visualContext().position, ListItemPosition::kLast);
  EXPECT_EQ(dynamic.offsetTop(), first.height() + Scaled(2));
  EXPECT_EQ(last.offsetTop(),
            dynamic.offsetTop() + dynamic.height() + Scaled(2));
  EXPECT_EQ(zero.height(), 0);
  first.setVisibility(Visibility::kGone);
  last.setVisibility(Visibility::kGone);
  model.count = 1;
  dynamic.modelChanged();
  ASSERT_TRUE(refresh());
  ASSERT_NE(dynamic.row(0), nullptr);
  EXPECT_EQ(dynamic.row(0)->visualContext().position,
            ListItemPosition::kSingle);
  EXPECT_EQ(dynamic.height(), dynamic.row(0)->height());
}

// Verifies exclusive selection never scans offscreen model elements and remains
// stable across scrolling, mode transitions, disabled rows, and tail
// truncation.
TEST_F(DynamicListTest, SelectionIsLogicalAndBounded) {
  Model model;
  model.count = 10000;
  Section dynamic(context(), model);
  Row footer(context(), "Footer");
  ObservingList list(context());
  list.add(dynamic);
  list.add(footer);
  list.setSelectionPolicy(SingleSelection());
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  int allocations = model.prepares;
  model.reads = 0;
  ASSERT_TRUE(list.select(dynamic, 9000));
  EXPECT_LT(model.reads, 100);
  EXPECT_EQ(list.changes, 1);
  EXPECT_TRUE(list.select(dynamic, 9000));
  EXPECT_EQ(list.changes, 1);
  EXPECT_EQ(list.selection().index, 9000);
  EXPECT_EQ(dynamic.row(9000), nullptr);
  scroll.scrollTo(0, -9000 * 56);
  ASSERT_TRUE(refresh());
  ASSERT_NE(dynamic.row(9000), nullptr);
  EXPECT_TRUE(dynamic.row(9000)->visualContext().selected);
  EXPECT_EQ(model.prepares, allocations);
  model.enabled = false;
  dynamic.modelChanged();
  EXPECT_EQ(list.selection().index, 9000);
  EXPECT_TRUE(list.select(footer));
  EXPECT_FALSE(dynamic.row(9000)->visualContext().selected);
  EXPECT_TRUE(list.select(dynamic, 9000));
  model.count = 2;
  dynamic.modelRangeChanged(0, 0);
  EXPECT_EQ(list.selection().section, nullptr);
  EXPECT_FALSE(list.select(dynamic, 2));
  EXPECT_TRUE(list.select(footer));
  list.setSelectionPolicy({});
  EXPECT_EQ(list.selection().section, nullptr);
  list.setSelectionPolicy(SingleSelection());
  EXPECT_EQ(list.selection().section, nullptr);
}

// Verifies reset releases old text and captures before selection notification,
// which can recursively clear the parent and safely destroy an adopted section.
TEST_F(DynamicListTest, ResetCallbackCanDestroyAdoptedSection) {
  Model model;
  auto owned = std::make_unique<Section>(context(), model);
  Section* section = owned.get();
  ObservingList list(context());
  list.add(std::move(owned));
  list.setSelectionPolicy(SingleSelection());
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(list.select(*section, 0));
  const int live_bindings = section->last() - section->first() + 1;
  const int released_before = model.releases;
  list.changed = [&](ListRowLocation location) {
    EXPECT_EQ(location.section, nullptr);
    EXPECT_EQ(model.releases - released_before, live_bindings);
    list.clear();
    list.clear();
  };
  section->beginModelReset();
  model.text.clear();
  model.text.shrink_to_fit();
  EXPECT_EQ(model.releases - released_before, live_bindings);
  EXPECT_EQ(list.selection().section, nullptr);
}

// Verifies borrowed sections can finish reset after callback-driven detachment,
// while requests into a resetting section fail and selection elsewhere works.
TEST_F(DynamicListTest, ResetReentrancyAndBorrowedDetachment) {
  Model model;
  Section section(context(), model);
  Row footer(context(), "Footer");
  ObservingList list(context());
  list.add(section);
  list.add(footer);
  list.setSelectionPolicy(SingleSelection());
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  list.select(section, 0);
  list.changed = [&](ListRowLocation location) {
    if (location.section != nullptr) return;
    EXPECT_FALSE(list.select(section, 0));
    EXPECT_TRUE(list.select(footer));
  };
  section.beginModelReset();
  EXPECT_EQ(list.selection().section, &footer);
  int reads = model.reads;
  refresh();
  EXPECT_EQ(model.reads, reads);
  EXPECT_DEATH(section.beginModelReset(), "");
  EXPECT_DEATH(section.modelChanged(), "");
  model.text = "Replacement";
  section.endModelReset();
  ASSERT_TRUE(refresh());
  EXPECT_DEATH(section.endModelReset(), "");
  list.changed = {};
  list.select(section, 0);
  list.changed = [&](ListRowLocation) { list.clear(); };
  section.beginModelReset();
  EXPECT_EQ(section.parent(), nullptr);
  section.endModelReset();
}

// Verifies invocation stops after a selection callback releases its logical
// row.
TEST_F(DynamicListTest, InvocationDoesNotUseRowAfterCallbackClear) {
  Model model;
  Section section(context(), model);
  ObservingList list(context());
  list.add(section);
  list.setSelectionPolicy(SingleSelection());
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  list.changed = [&](ListRowLocation location) {
    if (location.section != nullptr) list.clear();
  };
  section.row(0)->onClicked();
  EXPECT_EQ(model.invokes, 0);
  EXPECT_EQ(list.selection().section, nullptr);
}

// Verifies Up/Down skip invisible sections without binding them, but jump into
// distant visible rows with a bounded window and retain focus after reveal.
TEST_F(DynamicListTest, KeyboardSkipsInvisibleAndMaterializesDistantRows) {
  Model hidden_model;
  hidden_model.count = 10000;
  Model model;
  model.count = 10000;
  Section hidden(context(), hidden_model);
  Section section(context(), model);
  Row header(context(), "Header");
  header.item().setOnInvoked([]() {});
  List list(context());
  list.add(header);
  list.add(hidden);
  list.add(section);
  hidden.setVisibility(Visibility::kInvisible);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(header.requestFocus());
  int binds = model.binds;
  ASSERT_TRUE(list.onKeyEvent({KeyPhase::kDown, KeyCode::kDown, 0, 0}));
  EXPECT_EQ(hidden_model.binds, 0);
  EXPECT_LE(model.binds - binds, static_cast<int>(section.poolCapacity()));
  ASSERT_NE(section.row(0), nullptr);
  EXPECT_TRUE(section.row(0)->isFocused());
  refresh();
  ASSERT_NE(section.row(0), nullptr);
  EXPECT_TRUE(section.row(0)->isFocused());
  ASSERT_TRUE(list.onKeyEvent({KeyPhase::kDown, KeyCode::kUp, 0, 0}));
  EXPECT_TRUE(header.isFocused());
  list.onKeyEvent({KeyPhase::kDown, KeyCode::kDown, 0, 0});
  section.setVisibility(Visibility::kInvisible);
  EXPECT_EQ(section.focusManager().focused(), nullptr);
  section.setVisibility(Visibility::kVisible);
  EXPECT_EQ(section.focusManager().focused(), nullptr);
}

// Verifies refresh preserves the current binding's focus and disabling a
// section clears focus without clearing the parent-owned logical selection.
TEST_F(DynamicListTest, ContentRefreshPreservesEligibleFocus) {
  Model model;
  Section section(context(), model);
  List list(context());
  list.add(section);
  list.setSelectionPolicy(SingleSelection());
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(section.row(0)->requestFocus());
  list.select(section, 0);
  section.modelItemChanged(0);
  refresh();
  ASSERT_NE(section.row(0), nullptr);
  EXPECT_TRUE(section.row(0)->isFocused());
  model.enabled = false;
  section.modelChanged();
  EXPECT_EQ(section.focusManager().focused(), nullptr);
  EXPECT_EQ(list.selection().section, &section);
}

// Verifies selection and divider suppression preserve a uniform dynamic stride.
TEST_F(DynamicListTest, MultipleSelectionDoesNotChangeStride) {
  Model model;
  model.count = 4;
  Section section(context(), model);
  Row footer(context(), "Footer");
  List list(context());
  list.add(section);
  list.add(footer);
  ListSelectionPolicy policy;
  policy.mode = SelectionMode::kMultiple;
  list.setSelectionPolicy(policy);
  ListDividerPolicy dividers;
  dividers.mode = DividerMode::kInset;
  list.setDividerPolicy(dividers);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  YDim stride = section.row(1)->offsetTop() - section.row(0)->offsetTop();
  YDim extent = list.height();
  model.selected = true;
  section.modelChanged();
  list.setSelected(footer, true);
  refresh();
  EXPECT_EQ(section.row(1)->offsetTop() - section.row(0)->offsetTop(), stride);
  EXPECT_EQ(list.height(), extent);
  EXPECT_FALSE(section.row(0)->visualContext().show_divider);
  EXPECT_FALSE(section.row(3)->visualContext().show_divider);
}

// Verifies descendant-only navigation focuses a radio accessory in an offscreen
// row, and skips a whole section after its uniform focus policy becomes none.
TEST_F(DynamicListTest, DescendantTargetsAndUniformPolicyChanges) {
  using RadioRow = ListRow<RadioListItem>;
  class Radios : public DynamicListModel<RadioRow> {
   public:
    DynamicListFocusTarget focus = DynamicListFocusTarget::kDescendant;
    int elementCount() const override { return 100; }
    void prepare(RadioRow& row) const override {
      row.item().setHeadline("Radio");
    }
    void bind(int, RadioRow& row) const override {
      row.item().setHeadline("Radio");
    }
    void unbind(RadioRow& row) const override { row.item().setHeadline({}); }
    DynamicListSectionState sectionState() const override {
      return {true, focus};
    }
  } model;
  DynamicList<RadioRow> section(context(), model);
  Row first(context(), "First");
  Row last(context(), "Last");
  first.item().setOnInvoked([]() {});
  last.item().setOnInvoked([]() {});
  List list(context());
  list.add(first);
  list.add(section);
  list.add(last);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(last.requestFocus());
  ASSERT_TRUE(list.onKeyEvent({KeyPhase::kDown, KeyCode::kUp, 0, 0}));
  Widget* focused = list.focusManager().focused();
  ASSERT_NE(focused, nullptr);
  ASSERT_NE(focused->parent(), nullptr);
  EXPECT_EQ(focused->parent()->parent(), &section);
  EXPECT_GE(section.last(), 99);
  refresh();
  EXPECT_EQ(list.focusManager().focused(), focused);
  model.focus = DynamicListFocusTarget::kNone;
  section.modelChanged();
  EXPECT_EQ(list.focusManager().focused(), nullptr);
  last.requestFocus();
  EXPECT_TRUE(list.onKeyEvent({KeyPhase::kDown, KeyCode::kUp, 0, 0}));
  EXPECT_TRUE(first.isFocused());
}

// Verifies reset rejects list mutation from unbind and ordinary clear rejects
// callback-driven reselection until all child detachment has finished.
TEST_F(DynamicListTest, CleanupAndClearGuards) {
  Model model;
  Section section(context(), model);
  Row footer(context(), "Footer");
  ObservingList list(context());
  list.add(section);
  list.add(footer);
  list.setSelectionPolicy(SingleSelection());
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  model.on_release = [&]() { list.clear(); };
  EXPECT_DEATH(section.beginModelReset(), "");
  model.on_release = {};
  list.select(section, 0);
  list.changed = [&](ListRowLocation) {
    EXPECT_FALSE(list.select(footer));
    EXPECT_FALSE(list.select(section, 0));
    list.clear();
  };
  list.clear();
  EXPECT_EQ(section.parent(), nullptr);
  EXPECT_EQ(footer.parent(), nullptr);
}

// Verifies a successful row invocation selects before invoking the item, while
// disabling follows-press leaves the explicit selection untouched.
TEST_F(DynamicListTest, InvocationSelectionAndOptOut) {
  Model model;
  Section section(context(), model);
  ObservingList list(context());
  list.add(section);
  ListSelectionPolicy policy = SingleSelection();
  list.setSelectionPolicy(policy);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  section.row(1)->onClicked();
  EXPECT_EQ(model.invokes, 1);
  EXPECT_EQ(list.selection().index, 1);
  policy.selection_follows_press = false;
  list.setSelectionPolicy(policy);
  section.row(2)->onClicked();
  EXPECT_EQ(model.invokes, 2);
  EXPECT_EQ(list.selection().index, 1);
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
