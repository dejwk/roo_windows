#include "roo_windows/material3/list/dynamic_list.h"

#include "roo_windows/material3/list/list_geometry.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows {
namespace material3 {

DynamicListBase::DynamicListBase(ApplicationContext& context,
                                 internal::DynamicModel& model,
                                 PrototypeFn factory)
    : ListLayout(context, model, std::move(factory)) {
  model.setSelectionListener(this);
}

DynamicListBase::~DynamicListBase() {
  CHECK(!cleaning());
  model().setSelectionListener(nullptr);
  flags_ |= 2;
  releaseRows();
}

internal::DynamicModel& DynamicListBase::model() {
  return static_cast<internal::DynamicModel&>(listModel());
}

const internal::DynamicModel& DynamicListBase::model() const {
  return static_cast<const internal::DynamicModel&>(listModel());
}

void DynamicListBase::checkModelNotification() const {
  CHECK(!resetting());
  CHECK(!cleaning());
}

void DynamicListBase::prepareRow(Widget& widget) {
  auto& row = static_cast<ListEntry&>(widget);
  CHECK_EQ(row.getMargins().top(), 0);
  CHECK_EQ(row.getMargins().bottom(), 0);
  CHECK_EQ(row.getMargins().left(), 0);
  CHECK_EQ(row.getMargins().right(), 0);
  model().prepareEntry(row);
  row.refreshFromItem();
}

void DynamicListBase::bindRow(int index, Widget& widget) {
  CHECK(!resetting());
  auto& row = static_cast<ListEntry&>(widget);
  model().set(index, row);
  syncSelection(row, index);
  row.refreshFromItem();
  DynamicListSectionState state = model().sectionState();
  row.setEnabled(state.enabled);
  if (owner_ != nullptr) {
    ListEntryVisualContext visual = owner_->rowContext(section_index_, index);
    visual.focused = row.isFocused();
    row.setVisualContext(visual);
  }
  if (state.enabled &&
      state.focus_target == DynamicListFocusTarget::kRowSurface) {
    CHECK(row.isFocusable());
  }
}

void DynamicListBase::unbindRow(Widget& widget) {
  auto& row = static_cast<ListEntry&>(widget);
  if (owner_ != nullptr) owner_->invalidateInvocations(this);
  if (row.item() != nullptr) row.item()->setSelectionHandler({});
  model().unbindEntry(row);
  row.releaseTextViews();
}

void DynamicListBase::beginModelReset() {
  CHECK(!resetting());
  CHECK(!cleaning());
  if (owner_ != nullptr) owner_->checkNotCleaningBindings();
  flags_ |= 3;
  if (owner_ != nullptr) owner_->invalidateInvocations(this);
  releaseRows();
  setElementCount(0);
  flags_ &= ~2;
  invalidateInterior();
  requestLayout();
  if (owner_ != nullptr) owner_->sectionChanged(*this);
  // Selection notification is terminal: this section may no longer exist.
}

void DynamicListBase::endModelReset() {
  CHECK(resetting());
  CHECK(!cleaning());
  flags_ &= ~1;
  modelChanged();
}

void DynamicListBase::onModelChanged(int old_count) {
  DynamicListSectionState state = model().sectionState();
  if (!state.enabled || state.focus_target == DynamicListFocusTarget::kNone) {
    focusManager().onSubtreeDetaching(*this);
  }
  if (owner_ != nullptr) {
    owner_->sectionChanged(*this, old_count != elementCount());
  }
}

YDim DynamicListBase::rowStride() const { return rowHeight() + gap_; }

YDim DynamicListBase::contentExtent() const {
  int64_t extent =
      elementCount() == 0
          ? 0
          : static_cast<int64_t>(elementCount()) * rowStride() - gap_;
  CHECK_GE(extent, 0);
  CHECK_LE(extent, Rect::MaximumRect().yMax());
  return extent;
}

Rect DynamicListBase::rowBounds(int index) const {
  YDim top = index * rowStride();
  return Rect(0, top, width() - 1, top + rowHeight() - 1);
}

PreferredSize DynamicListBase::getPreferredSize() const {
  return {PreferredSize::MatchParentWidth(),
          PreferredSize::WrapContentHeight()};
}

Dimensions DynamicListBase::onMeasure(WidthSpec width, HeightSpec height) {
  CHECK_EQ(getPadding().top(), 0);
  CHECK_EQ(getPadding().bottom(), 0);
  CHECK_EQ(getPadding().left(), 0);
  CHECK_EQ(getPadding().right(), 0);
  if (resetting()) return {width.resolveSize(0), 0};
  gap_ = owner_ == nullptr ? 0 : owner_->uniformGap();
  if (owner_ != nullptr && elementCount() != 0) {
    static_cast<ListEntry&>(prototype())
        .setVisualContext(owner_->rowContext(section_index_, 0));
  }
  return ListLayout::onMeasure(width, height);
}

void DynamicListBase::selectionChanged() {
  if (resetting()) return;
  if (owner_ != nullptr) owner_->sectionChanged(*this, false);
}

void DynamicListBase::syncSelection(ListEntry& row, int index) {
  if (row.item() == nullptr) return;
  bool managed = model().ownsSelection() ||
                 (owner_ != nullptr &&
                  owner_->selection_policy_.mode != SelectionMode::kNone);
  managed = managed &&
            model().rowState(index).participation ==
                SelectionParticipation::kSelectable &&
            row.item()->selectionParticipation() ==
                SelectionParticipation::kSelectable;
  if (!managed) {
    row.item()->setSelectionHandler({});
    return;
  }
  bool selected = owner_ == nullptr
                      ? model().rowState(index).selected
                      : owner_->rowContext(section_index_, index).selected;
  row.item()->applySelection(selected ? SelectionState::kSelected
                                      : SelectionState::kDeselected);
  Widget* control = row.item()->selectionControl();
  if (control != nullptr) {
    row.item()->setSelectionHandler([this, &row]() { invokeChild(row); });
  }
}

void DynamicListBase::refreshContexts() {
  if (resetting() || owner_ == nullptr) return;
  for (int index = first(); index <= last(); ++index) {
    auto& row = static_cast<ListEntry&>(*materializedRow(index));
    syncSelection(row, index);
    ListEntryVisualContext visual = owner_->rowContext(section_index_, index);
    visual.focused = row.isFocused();
    row.setVisualContext(visual);
  }
  invalidateInterior();
}

int DynamicListBase::indexOf(const Widget& row) const {
  for (int index = first(); index <= last(); ++index) {
    if (materializedRow(index) == &row) return index;
  }
  return -1;
}

ListEntry* DynamicListBase::focusRow(int index) {
  return static_cast<ListEntry*>(materialize(index));
}

bool DynamicListBase::invokeChild(Widget& child) {
  int index = indexOf(child);
  if (owner_ == nullptr || index < 0 || resetting()) return false;
  return owner_->invokeRow({this, index}, static_cast<ListEntry&>(child));
}

bool DynamicListBase::fillTouchTargetPath(XDim x, YDim y,
                                          std::vector<Widget*>& path) {
  if (!Widget::fillTouchTargetPath(x, y, path)) return false;
  if (resetting() || rowStride() <= 0 || y < 0) return true;
  int index = y / rowStride();
  if (index >= elementCount() || !rowBounds(index).contains(x, y)) return true;
  Widget* row = materializedRow(index);
  if (row != nullptr) row->fillTouchTargetPath(x, y - row->offsetTop(), path);
  return true;
}

bool DynamicListBase::fillSloppyTouchTargetPath(XDim x, YDim y,
                                                std::vector<Widget*>& path) {
  return fillTouchTargetPath(x, y, path);
}

Color DynamicListBase::background() const {
  return owner_ == nullptr ? ListLayout::background() : owner_->background();
}

void DynamicListBase::paint(PaintContext& context) const {
  if (owner_ != nullptr && !resetting() && elementCount() > 1 && gap_ > 0 &&
      !context.isDeadlineExceeded()) {
    Rect viewport = unclippedRegion(this);
    if (!viewport.empty()) {
      int first_band = std::max<YDim>(0, viewport.yMin() / rowStride() - 1);
      int last_band =
          std::min<YDim>(elementCount() - 2, viewport.yMax() / rowStride());
      for (int index = first_band; index <= last_band; ++index) {
        ListEntryVisualContext visual =
            owner_->rowContext(section_index_, index);
        YDim top = index * rowStride() + rowHeight() +
                   (gap_ - std::max(1, Scaled(1))) / 2;
        internal::DividerMetrics divider = internal::ResolveDividerMetrics(
            visual, model().rowState(index).divider_inset_hint, width(), 0,
            top);
        if (!divider.visible) continue;
        Rect band(divider.start_x, top, divider.end_x,
                  top + std::max(1, Scaled(1)) - 1);
        context.fillRect(band, theme().material3Theme().color.outlineVariant);
        context.addExclusion(band);
      }
    }
  }
  ListLayout::paint(context);
}

}  // namespace material3
}  // namespace roo_windows
