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
};

/// Uniform keyboard target policy for one virtualized section.
enum class DynamicListFocusTarget : uint8_t { kNone, kRowSurface, kDescendant };

/// Interaction policy shared by every model element in a section.
struct DynamicListSectionState {
  bool enabled = true;
  DynamicListFocusTarget focus_target = DynamicListFocusTarget::kNone;
};

namespace internal {
// Type-erased model bridge; no callback or bridge object per allocated row.
class DynamicModel : public ListModel {
 public:
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

  /// Returns the enabled/focus policy, uniform across all indices.
  virtual DynamicListSectionState sectionState() const override { return {}; }

 private:
  template <typename>
  friend class DynamicList;
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

/// Shared Material adapter over the generic recycler; insert into a List.
/// Each section uses one prototype height and uniform gaps. The model outlives
/// the section; borrowed sections outlive their parent or are explicitly
/// cleared.
class DynamicListBase : public ListLayout {
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
