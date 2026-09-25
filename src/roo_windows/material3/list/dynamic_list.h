#pragma once

#include <type_traits>

#include "roo_windows/containers/list_layout.h"
#include "roo_windows/material3/list/list.h"

namespace roo_windows {
namespace material3 {

/// Model metadata available without constructing a row.
struct DynamicListRowState {
  bool selected = false;
  DividerInsetHint divider_inset_hint = {};
  SelectionParticipation participation = SelectionParticipation::kSelectable;
};

/// Uniform keyboard target policy for one virtualized section.
enum class DynamicListFocusTarget : uint8_t { kNone, kRowSurface, kDescendant };

/// Interaction policy shared by every model element in a section.
struct DynamicListSectionState {
  bool enabled = true;
  DynamicListFocusTarget focus_target = DynamicListFocusTarget::kNone;
};

namespace internal {
// One section observes the optional selection helper; no model-row storage.
class SelectionListener {
 public:
  virtual void selectionChanged() = 0;
};

// Type-erased model bridge; no callback or bridge object per allocated row.
class DynamicModel : public ListModel {
 public:
  virtual bool ownsSelection() const { return false; }
  virtual void setSelectionListener(SelectionListener* listener) {}
  virtual void setSelected(int index, SelectionState state) {}
  virtual void prepareEntry(ListEntry& row) const = 0;
  virtual void unbindEntry(ListEntry& row) const = 0;
  virtual DynamicListRowState rowState(int index) const = 0;
  virtual DynamicListSectionState sectionState() const = 0;
};

template <typename Row>
class PreparedListRow : public Row {
 public:
  explicit PreparedListRow(ApplicationContext& context) : Row(context) {}

 protected:
  bool retainsTextSlots() const override { return true; }
};
}  // namespace internal

/// Typed borrowed data source for fixed-height Material rows.
/// Text and action captures must survive until unbind(). Replace backing
/// storage only between beginModelReset()/endModelReset(). Structural index
/// changes also require reset. All methods execute synchronously on the UI
/// context.
template <typename Row>
class DynamicListModel : private internal::DynamicModel {
 public:
  static_assert(std::is_base_of<ListEntry, Row>::value,
                "Dynamic rows must derive from ListEntry");
  virtual ~DynamicListModel() = default;

  /// Returns the nonnegative number of logical rows.
  virtual int elementCount() const override = 0;

  /// Configures fixed slots and representative prototype content once per row.
  /// Prepared content must outlive the row, including its unbound prototype;
  /// do not retain replaceable model backing storage during preparation.
  virtual void prepare(Row& row) const {}

  /// Fully replaces model-dependent content without changing prepared slots.
  virtual void bind(int index, Row& row) const = 0;

  /// Releases borrowed views/captures while their backing storage is still
  /// alive. Cleanup must not mutate list structure, selection, or reset state.
  virtual void unbind(Row& row) const {}

  /// Returns allocation-free visual metadata; selection is used in multiple
  /// mode.
  virtual DynamicListRowState rowState(int index) const override { return {}; }

  /// Receives per-item selection transitions on the UI context. In parent
  /// single mode this reports committed state; in multiple mode update the
  /// backing selected flag returned by rowState(). Controls refresh
  /// automatically. May clear/reset the view; the model must survive the
  /// callback.
  virtual void onSelectionChanged(int index, SelectionState state) {}

  /// Reports whether this model owns an independent selection group.
  bool ownsSelection() const override { return false; }

  /// Returns the enabled/focus policy, uniform across all indices.
  virtual DynamicListSectionState sectionState() const override { return {}; }

 private:
  template <typename>
  friend class DynamicList;
  void setSelected(int index, SelectionState state) override {
    onSelectionChanged(index, state);
  }

  void set(int index, Widget& row) const override {
    bind(index, static_cast<Row&>(row));
  }
  void prepareEntry(ListEntry& row) const override {
    prepare(static_cast<Row&>(row));
  }
  void unbindEntry(ListEntry& row) const override {
    unbind(static_cast<Row&>(row));
  }
};

/// Owns one optional selection independently of other sections and static
/// actions. Attach to at most one DynamicList at a time, inside a List with
/// selection mode kNone. Standard radio items synchronize automatically; custom
/// items can implement selectionControl()/applySelection(). The model must
/// outlive its section. All operations run on the UI context.
///
/// Reset preserves the index for append/content replacement. Before removing or
/// reordering rows, clear selection or explicitly remap it while bindings are
/// released. No identity lookup is performed by the framework.
template <typename Row = ListRow<RadioListItem>>
class DynamicSingleSelectionListModel : public DynamicListModel<Row> {
 public:
  /// Returns the selected index, or -1 when the group has no selection.
  int selectedIndex() const { return selected_index_; }

  /// Selects or deselects @p index, returning false for an invalid index.
  /// Deselecting an unselected row and selecting the current row are no-ops.
  /// Visible controls update before notification; offscreen rows update on
  /// bind.
  bool select(int index, SelectionState state = SelectionState::kSelected) {
    CHECK(!notifying_);
    if (index < 0 || index >= this->elementCount()) return false;
    if (state == SelectionState::kSelected &&
        this->rowState(index).participation ==
            SelectionParticipation::kAction) {
      return false;
    }
    int next = state == SelectionState::kSelected ? index : -1;
    if (state == SelectionState::kDeselected && index != selected_index_) {
      return true;
    }
    changeSelection(next);
    return true;
  }

  /// Clears selection, including an index whose backing row was removed.
  void clearSelection() {
    CHECK(!notifying_);
    changeSelection(-1);
  }

  /// Supplies selection metadata without consulting or allocating row widgets.
  DynamicListRowState rowState(int index) const override {
    return {index == selected_index_, {}};
  }

  /// Identifies this model as an independent selection owner.
  bool ownsSelection() const final { return true; }

 protected:
  /// Reacts to a committed transition, old deselection before new selection.
  /// Optional application notification; controls already reflect the new state.
  /// May clear/reset the view, but must not destroy this model or recursively
  /// change its selection. The old index may no longer exist after a mutation.
  void onSelectionChanged(int index, SelectionState state) override {}

 private:
  void setSelectionListener(internal::SelectionListener* listener) final {
    CHECK(listener == nullptr || listener_ == nullptr);
    listener_ = listener;
  }

  void setSelected(int index, SelectionState state) final {
    select(index, state);
  }

  // Publish a complete transition before any application callback can run.
  void changeSelection(int next) {
    if (selected_index_ == next) return;
    int previous = selected_index_;
    selected_index_ = next;
    if (listener_ != nullptr) listener_->selectionChanged();
    notifying_ = true;
    if (previous >= 0) {
      onSelectionChanged(previous, SelectionState::kDeselected);
    }
    if (next >= 0) onSelectionChanged(next, SelectionState::kSelected);
    notifying_ = false;
  }

  internal::SelectionListener* listener_ = nullptr;
  int selected_index_ = -1;
  bool notifying_ = false;
};

/// Shared Material adapter over the generic recycler; insert into a List.
/// Each section uses one prototype height and uniform gaps. The model outlives
/// the section; borrowed sections outlive their parent or are explicitly
/// cleared.
class DynamicListBase : public ListLayout, private internal::SelectionListener {
 public:
  /// Releases live bindings before pool or model bridge destruction.
  ~DynamicListBase() override;

  /// Cancels input and releases every binding before model storage replacement.
  /// A final selection callback can clear the parent and destroy this section.
  void beginModelReset();

  /// Publishes new data and ends reset; nested/unmatched resets fail CHECK.
  void endModelReset();

  /// Resolves gaps against the owning list's surface, including custom fills.
  Color background() const override;

  /// Routes touches only inside actual row surfaces; gaps remain
  /// noninteractive.
  bool fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) override;

  /// Keeps sloppy touch expansion from crossing a reserved separator band.
  bool fillSloppyTouchTargetPath(XDim x, YDim y,
                                 std::vector<Widget*>& path) override;

 protected:
  DynamicListBase(ApplicationContext& context, internal::DynamicModel& model,
                  PrototypeFn factory);
  void prepareRow(Widget& row) override;
  void bindRow(int index, Widget& row) override;
  void unbindRow(Widget& row) override;
  void checkModelNotification() const override;
  void onModelChanged(int old_count) override;
  YDim rowStride() const override;
  YDim contentExtent() const override;
  Rect rowBounds(int index) const override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  PreferredSize getPreferredSize() const override;
  void paint(PaintContext& context) const override;
  bool invokeChild(Widget& child) override;

 private:
  friend class List;
  internal::DynamicModel& model();
  const internal::DynamicModel& model() const;
  bool resetting() const { return (flags_ & 1) != 0; }
  bool cleaning() const { return (flags_ & 2) != 0; }
  void selectionChanged() override;
  void syncSelection(ListEntry& row, int index);
  void refreshContexts();
  int indexOf(const Widget& row) const;
  ListEntry* focusRow(int index);
  List* owner_ = nullptr;
  int logical_start_ = 0;
  int section_index_ = 0;
  YDim gap_ = 0;
  uint8_t flags_ = 0;
};

/// Typed section owning a viewport-sized pool, with no per-element allocation.
/// Custom factories must retain prepared text slots; bind cannot add slots or
/// change text-widget classes. Wrapped/custom content budgets its own
/// allocations.
template <typename Row = ListRow<HeadlineListItem>>
class DynamicList : public DynamicListBase {
 public:
  using PrototypeFn = std::function<std::unique_ptr<Row>()>;

  /// Creates standard context-constructed rows borrowing @p model.
  DynamicList(ApplicationContext& context, DynamicListModel<Row>& model)
      : DynamicListBase(context, model, [&context]() {
          auto row = std::make_unique<internal::PreparedListRow<Row>>(context);
          if constexpr (std::is_same<Row, ListRow<HeadlineListItem>>::value) {
            row->item().setHeadline(" ");
            row->refreshFromItem();
          }
          return row;
        }) {}

  /// Creates custom row surfaces with @p factory, borrowing @p model.
  DynamicList(ApplicationContext& context, DynamicListModel<Row>& model,
              PrototypeFn factory)
      : DynamicListBase(context, model, [factory = std::move(factory)]() {
          return factory();
        }) {}
};

}  // namespace material3
}  // namespace roo_windows
