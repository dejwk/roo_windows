#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/list/dynamic_list.h"
#include "roo_windows/material3/theme.h"
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
  DividerInsetHint hint = {16, 8};
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
    return {selected, hint};
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

class FullWidthList : public List {
 public:
  using List::List;
  PreferredSize getPreferredSize() const override {
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::WrapContentHeight()};
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

using RadioRow = ListRow<RadioListItem>;
using CheckRow = ListRow<CheckboxListItem>;

class SingleModel : public DynamicSingleSelectionListModel<RadioRow> {
 public:
  int count = 10000;
  std::vector<std::pair<int, SelectionState>> changes;
  std::function<void()> changed;

  int elementCount() const override { return count; }
  void prepare(RadioRow& row) const override {
    row.item().setHeadline("Device");
  }
  void bind(int, RadioRow& row) const override {
    row.item().setHeadline("Device");
  }
  void unbind(RadioRow& row) const override { row.item().setHeadline({}); }
  DynamicListSectionState sectionState() const override {
    return {true, DynamicListFocusTarget::kDescendant};
  }

 protected:
  void onSelectionChanged(int index, SelectionState state) override {
    changes.emplace_back(index, state);
    if (changed) changed();
  }
};

template <typename Item>
class ChoiceModel : public DynamicListModel<ListRow<Item>> {
 public:
  bool selected[3] = {};
  std::vector<std::pair<int, SelectionState>> changes;
  std::function<void(int, SelectionState)> changed;

  int elementCount() const override { return 3; }
  void prepare(ListRow<Item>& row) const override {
    row.item().setHeadline("Choice");
  }
  void bind(int, ListRow<Item>& row) const override {
    row.item().setHeadline("Choice");
  }
  DynamicListRowState rowState(int index) const override {
    return {selected[index],
            {},
            index == 2 ? SelectionParticipation::kAction
                       : SelectionParticipation::kSelectable};
  }
  void onSelectionChanged(int index, SelectionState state) override {
    selected[index] = state == SelectionState::kSelected;
    changes.emplace_back(index, state);
    if (changed) changed(index, state);
  }
};

template <typename Item>
class ChoiceSection : public DynamicList<ListRow<Item>> {
 public:
  using DynamicList<ListRow<Item>>::DynamicList;
  ListRow<Item>* row(int index) {
    return static_cast<ListRow<Item>*>(this->materializedRow(index));
  }
};

// Verifies helper-owned selection synchronizes row/radio activation,
// programmatic changes, and offscreen binding without application callback
// wiring or scans.
TEST_F(DynamicListTest, ModelOwnedSingleSelection) {
  SingleModel model;
  ChoiceSection<RadioListItem> section(context(), model);
  List list(context());
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  section.row(0)->onClicked();
  EXPECT_EQ(model.selectedIndex(), 0);
  EXPECT_TRUE(section.row(0)->item().isSelected());
  section.row(1)->item().radioButton().onClicked();
  EXPECT_EQ(model.selectedIndex(), 1);
  EXPECT_FALSE(section.row(0)->item().isSelected());
  EXPECT_TRUE(section.row(1)->visualContext().selected);
  ASSERT_EQ(model.changes.size(), 3u);
  EXPECT_EQ(model.changes[1], std::make_pair(0, SelectionState::kDeselected));
  EXPECT_EQ(model.changes[2], std::make_pair(1, SelectionState::kSelected));
  model.select(1);
  EXPECT_EQ(model.changes.size(), 3u);
  model.select(0, SelectionState::kDeselected);
  EXPECT_EQ(model.selectedIndex(), 1);
  EXPECT_FALSE(model.select(-1));
  EXPECT_FALSE(model.select(10000));
  ASSERT_TRUE(model.select(9000));
  EXPECT_FALSE(section.row(1)->item().isSelected());
  scroll.scrollTo(0, -9000 * 56);
  ASSERT_TRUE(refresh());
  ASSERT_NE(section.row(9000), nullptr);
  EXPECT_TRUE(section.row(9000)->item().isSelected());
  model.clearSelection();
  EXPECT_EQ(model.selectedIndex(), -1);
  EXPECT_FALSE(section.row(9000)->item().isSelected());
}

// Verifies independent groups and static action participation, including normal
// invocation without stealing selection and append reset without restoration.
TEST_F(DynamicListTest, IndependentGroupsAndAppend) {
  SingleModel first_model;
  first_model.count = 2;
  SingleModel second_model;
  second_model.count = 2;
  ChoiceSection<RadioListItem> first(context(), first_model);
  ChoiceSection<RadioListItem> second(context(), second_model);
  Row action(context(), "Add");
  int invokes = 0;
  action.item().setSelectionParticipation(SelectionParticipation::kAction);
  action.item().setOnInvoked([&]() { ++invokes; });
  List list(context());
  list.add(action);
  list.add(first);
  list.add(second);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  first_model.select(0);
  second_model.select(1);
  action.onClicked();
  EXPECT_EQ(invokes, 1);
  EXPECT_EQ(first_model.selectedIndex(), 0);
  EXPECT_EQ(second_model.selectedIndex(), 1);
  first.beginModelReset();
  ++first_model.count;
  first.endModelReset();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(first_model.selectedIndex(), 0);
  EXPECT_TRUE(first.row(0)->item().isSelected());
  first.beginModelReset();
  first_model.clearSelection();
  first_model.count = 0;
  first.endModelReset();
  EXPECT_EQ(first_model.selectedIndex(), -1);
}

// Verifies parent-wide single selection emits enum transitions for old/new
// rows, while static and dynamic action rows cannot acquire selection.
TEST_F(DynamicListTest, ParentSelectionNotificationsAndActions) {
  ChoiceModel<RadioListItem> model;
  ChoiceSection<RadioListItem> section(context(), model);
  Row action(context(), "Action");
  action.item().setSelectionParticipation(SelectionParticipation::kAction);
  int invokes = 0;
  action.item().setOnInvoked([&]() { ++invokes; });
  List list(context());
  list.setSelectionPolicy(SingleSelection());
  list.add(action);
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(list.select(section, 0));
  section.row(1)->item().radioButton().onClicked();
  ASSERT_EQ(model.changes.size(), 3u);
  EXPECT_EQ(model.changes[1], std::make_pair(0, SelectionState::kDeselected));
  EXPECT_EQ(model.changes[2], std::make_pair(1, SelectionState::kSelected));
  EXPECT_FALSE(section.row(0)->item().isSelected());
  EXPECT_TRUE(section.row(1)->item().isSelected());
  action.onClicked();
  EXPECT_EQ(invokes, 1);
  EXPECT_EQ(list.selection().index, 1);
  EXPECT_FALSE(list.select(action));
  EXPECT_FALSE(list.select(section, 2));
  section.row(2)->onClicked();
  EXPECT_EQ(list.selection().index, 1);
  list.clearSelection();
  EXPECT_EQ(model.changes.back(),
            std::make_pair(1, SelectionState::kDeselected));
}

// Verifies multiple selection updates model flags and checkbox state exactly
// once for row, accessory, and explicit operations, preserving other choices.
TEST_F(DynamicListTest, MultipleSelectionCallbacksAndControls) {
  ChoiceModel<CheckboxListItem> model;
  ChoiceSection<CheckboxListItem> section(context(), model);
  List list(context());
  ListSelectionPolicy policy;
  policy.mode = SelectionMode::kMultiple;
  list.setSelectionPolicy(policy);
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  section.row(0)->onClicked();
  EXPECT_TRUE(model.selected[0]);
  EXPECT_TRUE(section.row(0)->item().isChecked());
  section.row(1)->item().checkbox().onClicked();
  EXPECT_TRUE(model.selected[0]);
  EXPECT_TRUE(model.selected[1]);
  EXPECT_TRUE(section.row(1)->item().isChecked());
  section.row(0)->item().checkbox().onClicked();
  EXPECT_FALSE(model.selected[0]);
  EXPECT_FALSE(section.row(0)->item().isChecked());
  EXPECT_EQ(model.changes.size(), 3u);
  list.setSelected(section, 1, SelectionState::kDeselected);
  EXPECT_FALSE(section.row(1)->item().isChecked());
  EXPECT_FALSE(list.setSelected(section, 2, SelectionState::kSelected));
  policy.mode = SelectionMode::kNone;
  list.setSelectionPolicy(policy);
  section.row(0)->item().checkbox().onClicked();
  EXPECT_TRUE(section.row(0)->item().isChecked());
  EXPECT_FALSE(model.selected[0]);
  EXPECT_EQ(model.changes.size(), 4u);
}

// Verifies follows-press opt-out and rejected requests restore a control's
// authoritative value before delivering its action, without extra
// notifications.
TEST_F(DynamicListTest, SelectionControlOptOutAndRejection) {
  ChoiceModel<CheckboxListItem> model;
  ChoiceSection<CheckboxListItem> section(context(), model);
  List list(context());
  ListSelectionPolicy policy;
  policy.mode = SelectionMode::kMultiple;
  policy.selection_follows_press = false;
  list.setSelectionPolicy(policy);
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  section.row(0)->item().checkbox().onClicked();
  EXPECT_FALSE(section.row(0)->item().isChecked());
  EXPECT_TRUE(model.changes.empty());
  policy.selection_follows_press = true;
  list.setSelectionPolicy(policy);
  model.changed = [&](int index, SelectionState) {
    model.selected[index] = false;
  };
  section.row(0)->onClicked();
  EXPECT_FALSE(section.row(0)->item().isChecked());
  EXPECT_EQ(model.changes.size(), 1u);
}

// Verifies nested single-selection changes supersede a pending selected event,
// and reset sends deselection while old model data still exists.
TEST_F(DynamicListTest, ReentrantSelectionNotifications) {
  ChoiceModel<RadioListItem> model;
  ChoiceSection<RadioListItem> section(context(), model);
  List list(context());
  list.setSelectionPolicy(SingleSelection());
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  list.select(section, 0);
  model.changed = [&](int index, SelectionState state) {
    if (index == 0 && state == SelectionState::kDeselected)
      list.select(section, 0);
  };
  list.select(section, 1);
  EXPECT_EQ(list.selection().index, 0);
  EXPECT_EQ(model.changes.back(), std::make_pair(0, SelectionState::kSelected));
  model.changed = {};
  section.beginModelReset();
  EXPECT_EQ(model.changes.back(),
            std::make_pair(0, SelectionState::kDeselected));
  section.endModelReset();
}

// Verifies model callbacks can destroy an adopted section, canceling pending
// invocation and later selection notifications without accessing deleted rows.
TEST_F(DynamicListTest, SelectionCallbackCanDestroySection) {
  ChoiceModel<RadioListItem> model;
  List list(context());
  list.setSelectionPolicy(SingleSelection());
  auto section =
      std::make_unique<ChoiceSection<RadioListItem>>(context(), model);
  auto* borrowed = section.get();
  list.add(std::move(section));
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  list.select(*borrowed, 0);
  model.changed = [&](int index, SelectionState state) {
    if (index == 0 && state == SelectionState::kDeselected) list.clear();
  };
  borrowed->row(1)->onClicked();
  EXPECT_EQ(list.selection().section, nullptr);
  // The pending selected notification for the destroyed target is canceled.
  for (const auto& change : model.changes) {
    EXPECT_NE(change, std::make_pair(1, SelectionState::kSelected));
  }
}

// Verifies helper notifications may clear the view without retaining a row or
// calling its action after detachment; the model remains usable independently.
TEST_F(DynamicListTest, HelperCallbackCanClearView) {
  SingleModel model;
  List list(context());
  auto section =
      std::make_unique<ChoiceSection<RadioListItem>>(context(), model);
  auto* borrowed = section.get();
  list.add(std::move(section));
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  model.changed = [&]() { list.clear(); };
  borrowed->row(0)->item().radioButton().onClicked();
  EXPECT_EQ(model.selectedIndex(), 0);
  model.changed = {};
  EXPECT_TRUE(model.select(5));
}

// Verifies incompatible ownership and accidental sharing fail explicitly.
TEST_F(DynamicListTest, SelectionOwnershipChecks) {
  SingleModel model;
  ChoiceSection<RadioListItem> section(context(), model);
  List list(context());
  list.add(section);
  EXPECT_DEATH(list.setSelectionPolicy(SingleSelection()), "");
  EXPECT_DEATH((ChoiceSection<RadioListItem>(context(), model)), "");
  model.changed = [&]() { model.clearSelection(); };
  EXPECT_DEATH(model.select(0), "");
  model.changed = {};
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

// Verifies unselected mixed sections render exactly like an eager list across
// styles, variants, and divider policies, including both dynamic/static seams.
TEST_F(DynamicListTest, MixedPixelsMatchEagerReference) {
  for (ListVariant variant :
       {ListVariant::kBaseline, ListVariant::kExpressive}) {
    for (ListStyle style : {ListStyle::kStandard, ListStyle::kSegmented}) {
      for (DividerMode mode :
           {DividerMode::kNone, DividerMode::kFullWidth, DividerMode::kInset}) {
        Model model;
        model.count = 2;
        model.hint = {};
        Section section(context(), model);
        Row first(context(), "Device");
        Row last(context(), "Device");
        FullWidthList mixed(context());
        mixed.setVariant(variant);
        mixed.setStyle(style);
        ListDividerPolicy policy;
        policy.mode = mode;
        policy.start_inset = 12;
        policy.end_inset = 8;
        mixed.setDividerPolicy(policy);
        mixed.add(first);
        mixed.add(section);
        mixed.add(last);
        SimpleScrollablePanel scroll(context(), mixed);
        std::vector<roo::byte> reference;
        {
          Mount mount(app_, scroll);
          ASSERT_TRUE(refresh());
          reference.assign(raster_, raster_ + sizeof(raster_));
        }
        FullWidthList eager(context());
        eager.setVariant(variant);
        eager.setStyle(style);
        eager.setDividerPolicy(policy);
        for (int i = 0; i < 4; ++i) {
          eager.add(std::make_unique<Row>(context(), "Device"));
        }
        SimpleScrollablePanel eager_scroll(context(), eager);
        Mount mount(app_, eager_scroll);
        ASSERT_TRUE(refresh());
        int mismatch_count = 0;
        int first_x = -1;
        int first_y = -1;
        for (size_t byte = 0; byte < reference.size(); ++byte) {
          if (reference[byte] == raster_[byte]) continue;
          if (first_x < 0) {
            first_x = (byte / 2) % kWidth;
            first_y = (byte / 2) / kWidth;
          }
          ++mismatch_count;
        }
        EXPECT_EQ(mismatch_count, 0)
            << "variant=" << static_cast<int>(variant)
            << " style=" << static_cast<int>(style)
            << " divider=" << static_cast<int>(mode) << " first=" << first_x
            << "," << first_y << " mixed_height=" << mixed.height()
            << " eager_height=" << eager.height()
            << " section_height=" << section.height()
            << " row_height=" << first.height();
      }
    }
  }
}

// Verifies uniform selected geometry and partial-viewport dividers against
// persistent golden images; mixed seams have no enclosing section surface.
TEST_F(DynamicListTest, MixedGoldenStates) {
  Model model;
  model.count = 8;
  Section section(context(), model);
  Row first(context(), "Add device");
  Row last(context(), "Advanced");
  FullWidthList list(context());
  list.setStyle(ListStyle::kSegmented);
  ListDividerPolicy dividers;
  dividers.mode = DividerMode::kInset;
  list.setDividerPolicy(dividers);
  list.add(first);
  list.add(section);
  list.add(last);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  auto capture = [&]() {
    return test::CaptureRgb(offscreen_.raster(), 0, 0, kWidth, kHeight);
  };
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      capture(), "test/goldens/material3_dynamic_list/mixed.ppm",
      "material3_dynamic_list_mixed"));
  ListSelectionPolicy policy;
  policy.mode = SelectionMode::kMultiple;
  list.setSelectionPolicy(policy);
  model.selected = true;
  section.modelChanged();
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      capture(), "test/goldens/material3_dynamic_list/selected.ppm",
      "material3_dynamic_list_selected"));
  model.selected = false;
  section.modelChanged();
  scroll.scrollTo(0, -112);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      capture(), "test/goldens/material3_dynamic_list/offscreen.ppm",
      "material3_dynamic_list_offscreen"));
}

// Verifies retained row storage and metadata/bind work are independent of model
// length in every selection mode, and reports host viewport-update timings.
TEST_F(DynamicListTest, ResourceBoundsAcrossModelSizes) {
  for (SelectionMode mode : {SelectionMode::kNone, SelectionMode::kSingle,
                             SelectionMode::kMultiple}) {
    size_t capacity = 0;
    int expected_reads = -1;
    int expected_binds = -1;
    for (int count : {100, 10000}) {
      Model model;
      model.count = count;
      Section section(context(), model);
      List list(context());
      list.add(section);
      ListSelectionPolicy policy;
      policy.mode = mode;
      list.setSelectionPolicy(policy);
      SimpleScrollablePanel scroll(context(), list);
      Mount mount(app_, scroll);
      ASSERT_TRUE(refresh());
      if (mode == SelectionMode::kSingle) list.select(section, 50);
      if (capacity != 0) {
        EXPECT_EQ(section.poolCapacity(), capacity);
      }
      capacity = section.poolCapacity();
      EXPECT_LE(capacity, 8u);
      int prepares = model.prepares;
      model.reads = 0;
      model.binds = 0;
      auto start = std::chrono::steady_clock::now();
      for (int i = 1; i <= 20; ++i) {
        scroll.scrollTo(0, -i * 56);
        ASSERT_TRUE(refresh());
      }
      auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                         std::chrono::steady_clock::now() - start)
                         .count();
      EXPECT_EQ(model.prepares, prepares);
      EXPECT_LT(model.reads, 2000);
      EXPECT_LT(model.binds, 200);
      if (expected_reads >= 0) {
        EXPECT_EQ(model.reads, expected_reads);
      }
      if (expected_binds >= 0) {
        EXPECT_EQ(model.binds, expected_binds);
      }
      expected_reads = model.reads;
      expected_binds = model.binds;
      std::cout << "dynamic-list mode=" << static_cast<int>(mode)
                << " count=" << count << " pool=" << capacity
                << " prepares=" << prepares << " reads=" << model.reads
                << " binds=" << model.binds << " 20-updates-us=" << elapsed
                << '\n';
    }
  }
}

// Verifies a pending touch animation is canceled before a pooled row is
// rebound.
TEST_F(DynamicListTest, RecyclingCancelsPendingTouch) {
  Model model;
  model.count = 100;
  Section section(context(), model);
  List list(context());
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  Row* row = section.row(0);
  row->onShowPress(10, 10);
  EXPECT_TRUE(row->isPressed());
  scroll.scrollTo(0, -2000);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(row->isPressed());
  EXPECT_FALSE(row->isClicking());
  EXPECT_EQ(app_.root().click_animation().target(), nullptr);
  EXPECT_EQ(model.invokes, 0);
}

// Verifies an interrupted paint resumes existing bindings, then a reset starts
// a new revision without retaining views into freed model storage.
TEST_F(DynamicListTest, InterruptedPaintingDoesNotRebind) {
  class SlowRow : public Row {
   public:
    explicit SlowRow(ApplicationContext& context) : Row(context) {}
    mutable bool slow = false;

   protected:
    bool retainsTextSlots() const override { return true; }
    void paint(PaintContext& context) const override {
      Row::paint(context);
      if (slow) delay(20);
    }
  };
  Model model;
  DynamicList<Row> section(
      context(), model, [&]() { return std::make_unique<SlowRow>(context()); });
  List list(context());
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  for (Widget* child : section.children())
    static_cast<SlowRow*>(child)->slow = true;
  section.invalidateInterior();
  EXPECT_FALSE(refresh(roo_time::Uptime::Now() + roo_time::Millis(5)));
  int binds = model.binds;
  for (Widget* child : section.children())
    static_cast<SlowRow*>(child)->slow = false;
  ASSERT_TRUE(refresh());
  EXPECT_EQ(model.binds, binds);
  section.beginModelReset();
  model.text.assign(4096, 'X');
  model.text = "New storage";
  model.text.shrink_to_fit();
  section.endModelReset();
  ASSERT_TRUE(refresh());
}

// Verifies both parent-owned and internal separator bands stop sloppy touch
// expansion while actual row surfaces still route to their bound widgets.
TEST_F(DynamicListTest, SeparatorBandsNeverInvokeNeighborRows) {
  Model model;
  model.count = 2;
  Section section(context(), model);
  Row first(context(), "First");
  first.item().setOnInvoked([]() {});
  FullWidthList list(context());
  list.setStyle(ListStyle::kSegmented);
  list.add(first);
  list.add(section);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  std::vector<Widget*> path;
  ASSERT_TRUE(list.fillTouchTargetPath(20, first.height(), path));
  EXPECT_EQ(path.back(), &list);
  path.clear();
  ASSERT_TRUE(list.fillSloppyTouchTargetPath(20, first.height(), path));
  EXPECT_EQ(path.back(), &list);
  path.clear();
  YDim internal_gap = section.offsetTop() + section.row(0)->height();
  ASSERT_TRUE(list.fillTouchTargetPath(20, internal_gap, path));
  EXPECT_EQ(path.back(), &section);
  path.clear();
  ASSERT_TRUE(list.fillTouchTargetPath(200, section.offsetTop() + 20, path));
  EXPECT_EQ(path.back(), section.row(0));
}

// Verifies multiple nonempty collections share global corners and collapse
// their seam when the first collection becomes empty.
TEST_F(DynamicListTest, MultipleCollectionsShareGlobalEnds) {
  Model first_model;
  first_model.count = 2;
  Model last_model;
  last_model.count = 1;
  Section first(context(), first_model);
  Section last(context(), last_model);
  FullWidthList list(context());
  list.setStyle(ListStyle::kSegmented);
  list.add(first);
  list.add(last);
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(first.row(0)->visualContext().position, ListItemPosition::kFirst);
  EXPECT_EQ(first.row(1)->visualContext().position, ListItemPosition::kMiddle);
  EXPECT_EQ(last.row(0)->visualContext().position, ListItemPosition::kLast);
  EXPECT_EQ(last.offsetTop(), first.height() + Scaled(2));
  first_model.count = 0;
  first.modelChanged();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(last.offsetTop(), 0);
  EXPECT_EQ(last.row(0)->visualContext().position, ListItemPosition::kSingle);
}

// Verifies width and policy changes recompute row bounds and gaps without
// allocating new rows or losing the selected logical location.
TEST_F(DynamicListTest, WidthAndPolicyChangesReusePreparedRows) {
  class SizedList : public List {
   public:
    using List::List;
    XDim desired_width = 240;
    PreferredSize getPreferredSize() const override {
      return {PreferredSize::ExactWidth(desired_width),
              PreferredSize::WrapContentHeight()};
    }
  } list(context());
  Model model;
  Section section(context(), model);
  list.add(section);
  list.setSelectionPolicy(SingleSelection());
  SimpleScrollablePanel scroll(context(), list);
  Mount mount(app_, scroll);
  ASSERT_TRUE(refresh());
  list.select(section, 2);
  int prepares = model.prepares;
  list.desired_width = 160;
  list.setStyle(ListStyle::kSegmented);
  list.requestLayout();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(section.width(), 160);
  EXPECT_EQ(section.row(0)->width(), 160);
  EXPECT_EQ(section.row(1)->offsetTop(), section.row(0)->height() + Scaled(2));
  EXPECT_EQ(list.selection().index, 2);
  EXPECT_EQ(model.prepares, prepares);
  // List is declared before its borrowed section in this test, so detach now.
  list.clear();
}

// Verifies the current theme and a specialized parent background reach dynamic
// rows and their gap surfaces after invalidation, without replacing the pool.
TEST(DynamicListTheme, UsesUpdatedThemeAndParentBackground) {
  roo_scheduler::Scheduler scheduler;
  Material3Theme material = DefaultTheme().material3Theme();
  Theme theme = DefaultTheme();
  theme.material3_theme = &material;
  Environment environment(scheduler, theme);
  roo::byte pixels[240 * 320 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      240, 320, pixels, roo_display::Argb4444());
  roo_display::Display display(device);
  Application app(&environment, display);
  Model model;
  Section section(app.context(), model);
  class ColoredList : public FullWidthList {
   public:
    using FullWidthList::FullWidthList;
    Color background() const override { return roo_display::color::Green; }
  } list(app.context());
  list.setStyle(ListStyle::kSegmented);
  list.add(section);
  SimpleScrollablePanel scroll(app.context(), list);
  Mount mount(app, scroll);
  ASSERT_TRUE(app.refresh());
  int prepares = model.prepares;
  material.color.surfaceContainer = roo_display::color::Red;
  list.requestLayout();
  list.invalidateInterior();
  ASSERT_TRUE(app.refresh());
  EXPECT_EQ(section.row(0)->background(), roo_display::color::Red);
  EXPECT_EQ(section.background(), roo_display::color::Green);
  EXPECT_EQ(model.prepares, prepares);
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      test::CaptureRgb(device.raster(), 0, 0, 240, 320),
      "test/goldens/material3_dynamic_list/theme_changed.ppm",
      "material3_dynamic_list_theme_changed"));
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows
