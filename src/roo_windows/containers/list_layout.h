#pragma once

#include "roo_windows/config.h"
#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/measure_spec.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

/// Data source for a virtualized list rendered by `ListLayout`.
///
/// The model owns the underlying items and tells the layout how many there
/// are (`elementCount()`); the layout calls `set()` whenever a recycled child
/// widget needs to be bound to a new model row. Keeping the visual elements
/// separate from the model is what lets `ListLayout` scroll long lists with
/// constant RAM.
class ListModel {
 public:
  ListModel() {}
  virtual ~ListModel() {}

  // How many items in the list?
  virtual int elementCount() const = 0;

  // Set the specified visual element to the ith item of the list.
  virtual void set(int idx, Widget& dest) const = 0;
};

/// Fixed-capacity ring buffer of heap-allocated widget pointers.
///
/// Used internally by `ListLayout` to recycle a small set of element widgets
/// across a much larger model. Exposes STL-style `push_back` / `pop_back` /
/// `push_front` / `pop_front` accessors that walk the ring without
/// reallocating.
class CircularBuffer {
 public:
  CircularBuffer(ApplicationContext& context)
      : elements_(), start_(0), count_(0) {}

  CircularBuffer(CircularBuffer&& other)
      : elements_(std::move(other.elements_)),
        start_(other.start_),
        count_(other.count_) {}

  void ensure_capacity(size_t capacity,
                       std::function<std::unique_ptr<Widget>()>& prototype_fn) {
    CHECK_EQ(count_, 0);
    if (capacity <= this->capacity()) return;
    start_ = 0;
    count_ = 0;
    size_t i = this->capacity();
    while (i < capacity) {
      elements_.push_back(prototype_fn());
      ++i;
    }
  }

  Widget& push_back() {
    CHECK_LT(count_, (int)capacity());
    int offset = (start_ + count_) % capacity();
    ++count_;
    return *elements_[offset];
  }

  Widget& pop_back() {
    CHECK_GT(count_, 0);
    --count_;
    int offset = (start_ + count_) % capacity();
    return *elements_[offset];
  }

  Widget& push_front() {
    CHECK_LT(count_, (int)capacity());
    --start_;
    if (start_ < 0) start_ += capacity();
    ++count_;
    return *elements_[start_];
  }

  Widget& pop_front() {
    CHECK_GT(count_, 0);
    int offset = start_;
    ++start_;
    if ((size_t)start_ >= capacity()) start_ -= capacity();
    --count_;
    return *elements_[offset];
  }

  int pos(int idx) const { return (start_ + idx) % capacity(); }

  Widget& operator[](int idx) { return *elements_[pos(idx)]; }
  const Widget& operator[](int idx) const { return *elements_[pos(idx)]; }

  Widget& storage(size_t index) { return *elements_[index]; }

  bool empty() const { return count_ == 0; }

  size_t capacity() const { return elements_.size(); }

  size_t count() const { return count_; }

 private:
  std::vector<std::unique_ptr<Widget>> elements_;
  // int capacity_;
  int start_;
  int count_;
};

inline Rect unclippedRegion(const Widget* w) {
  Rect rect = w->bounds();
  XDim dx = 0;
  YDim dy = 0;
  while (w->parent() != nullptr) {
    dx -= w->offsetLeft();
    dy -= w->offsetTop();
    if (w->getParentClipMode() == ParentClipMode::kClipped) {
      rect = Rect::Intersect(rect, w->parent()->bounds().translate(dx, dy));
    }
    w = w->parent();
  }
  return rect;
}

/// Virtualized fixed-stride list borrowing a model and owning a reusable pool.
/// Model notifications run on the UI context, outside painting. Derived
/// adapters override protected geometry and binding hooks without replacing the
/// recycler.
class ListLayout : public Panel {
 public:
  using PrototypeFn = std::function<std::unique_ptr<Widget>()>;

  /// Creates a recycler borrowing @p model, allocating rows with @p
  /// prototype_fn.
  ListLayout(ApplicationContext& context, ListModel& model,
             PrototypeFn prototype_fn)
      : Panel(context),
        model_(model),
        elements_(context),
        prototype_fn_(std::move(prototype_fn)),
        prototype_(prototype_fn_()) {
    CHECK(prototype_ != nullptr);
    element_count_ = model.elementCount();
    CHECK_GE(element_count_, 0);
  }

  /// Detaches children before destroying their owning pool.
  ~ListLayout() override { removeAll(); }

  /// Sets content padding and requests measurement.
  void setPadding(Padding padding) {
    if (padding_ == padding) return;
    padding_ = padding;
    requestLayout();
  }

  Padding getPadding() const override { return padding_; }

  /// Refreshes all active bindings and publishes the current model count.
  void modelChanged() { modelRangeChanged(0, element_count_); }

  /// Returns the first bound model index, or zero for an empty active range.
  int first() const { return first_; }

  /// Returns the last bound index; less than first() when none are bound.
  int last() const { return last_; }

  /// Publishes count changes and refreshes bindings in [begin, end).
  /// An empty range publishes append/end-truncation without refreshing
  /// survivors.
  void modelRangeChanged(int begin, int end) {
    checkModelNotification();
    CHECK(!synchronizing_);
    synchronizing_ = true;
    int old_count = element_count_;
    element_count_ = model_.elementCount();
    CHECK_GE(element_count_, 0);
    while (last_ >= element_count_ && first_ <= last_) {
      releaseBack();
    }
    if (first_ > last_) {
      first_ = 0;
      last_ = -1;
    }
    for (int i = std::max(begin, first_); i <= last_ && i < end; ++i) {
      bindRow(i, elements_[i - first_]);
      layoutRow(i, elements_[i - first_]);
    }
    synchronizing_ = false;
    if (old_count != element_count_) requestLayout();
    invalidateInterior();
    onModelChanged(old_count);
  }

  /// Refreshes one model index while also publishing count changes.
  void modelItemChanged(int index) { modelRangeChanged(index, index + 1); }

  bool respectsChildrenBoundaries() const override { return true; }

 protected:
  /// Prepares fixed row content once for each prototype or pool allocation.
  virtual void prepareRow(Widget& row) {}

  /// Replaces the row's model-dependent state before measurement or input.
  virtual void bindRow(int index, Widget& row) { model_.set(index, row); }

  /// Releases a binding after input cancellation, before reuse or model reset.
  virtual void unbindRow(Widget& row) {}

  /// Validates adapter-specific notification preconditions.
  virtual void checkModelNotification() const {}

  /// Notifies an adapter after count publication and active content refresh.
  virtual void onModelChanged(int old_count) {}

  /// Measures the fixed stride from a prepared prototype and its margins.
  virtual YDim rowStride() const { return row_height_; }

  /// Computes content extent with checked wide intermediate arithmetic.
  virtual YDim contentExtent() const {
    int64_t extent = static_cast<int64_t>(element_count_) * rowStride() +
                     padding_.top() + padding_.bottom();
    CHECK_GE(extent, 0);
    CHECK_LE(extent, Rect::MaximumRect().yMax());
    return static_cast<YDim>(extent);
  }

  /// Returns the surface bounds for one row in local content coordinates.
  virtual Rect rowBounds(int index) const {
    Margins margins = prototype_->getMargins();
    YDim top = padding_.top() + index * rowStride() + margins.top();
    return Rect(padding_.left() + margins.left(), top,
                width() - padding_.right() - margins.right() - 1,
                top + row_height_ - margins.top() - margins.bottom() - 1);
  }

  /// Finds a bounded active interval; an empty interval has end < begin.
  virtual void viewportRange(const Rect& viewport, int& begin, int& end) const {
    begin = 0;
    end = -1;
    if (element_count_ == 0 || viewport.empty() || rowStride() <= 0) return;
    YDim top = std::max<YDim>(0, viewport.yMin() - padding_.top());
    YDim bottom = viewport.yMax() - padding_.top();
    if (bottom < 0) return;
    begin = std::min<int>(element_count_, top / rowStride());
    end = std::min<int>(element_count_ - 1, bottom / rowStride());
    if (begin <= end && !rowBounds(begin).intersects(viewport)) ++begin;
    if (begin <= end && !rowBounds(end).intersects(viewport)) --end;
  }

  /// Releases every live binding while retaining prepared allocations.
  void releaseRows() {
    while (first_ <= last_) releaseBack();
    first_ = 0;
    last_ = -1;
  }

  /// Publishes a count without reading the model, for reset protocols.
  void setElementCount(int count) {
    CHECK_GE(count, 0);
    element_count_ = count;
  }

  ListModel& listModel() { return model_; }
  const ListModel& listModel() const { return model_; }

  int elementCount() const { return element_count_; }
  YDim rowHeight() const { return row_height_; }
  Widget& prototype() { return *prototype_; }
  const Widget& prototype() const { return *prototype_; }
  size_t poolCapacity() const { return elements_.capacity(); }

  /// Finds a live binding without allocating or visiting model elements.
  Widget* materializedRow(int index) {
    return index >= first_ && index <= last_ ? &elements_[index - first_]
                                             : nullptr;
  }

  const Widget* materializedRow(int index) const {
    return index >= first_ && index <= last_ ? &elements_[index - first_]
                                             : nullptr;
  }

  /// Replaces the active window for keyboard focus without scanning intervening
  /// rows.
  Widget* materialize(int index) {
    CHECK_GE(index, 0);
    CHECK_LT(index, element_count_);
    CHECK_GT(elements_.capacity(), 0u);
    int begin = std::max(0, index - static_cast<int>(elements_.capacity()) / 2);
    int end = std::min(element_count_ - 1,
                       begin + static_cast<int>(elements_.capacity()) - 1);
    synchronizeRange(begin, end);
    return materializedRow(index);
  }

  void propagateDirty(const Widget* child, const Rect& rect) override {
    if (!synchronizing_) Panel::propagateDirty(child, rect);
  }

  void childHidden(const Widget* child) override {
    if (!synchronizing_) Panel::childHidden(child);
  }

  void childShown(const Widget* child) override {
    if (!synchronizing_) Panel::childShown(child);
  }

  void onRequestLayout() override {
    if (!synchronizing_) Widget::onRequestLayout();
  }

  void paintChildren(PaintContext& context) override {
    int begin;
    int end;
    viewportRange(unclippedRegion(this), begin, end);
    synchronizeRange(begin, end);
    Panel::paintChildren(context);
  }

  PreferredSize getPreferredSize() const override {
    PreferredSize preferred = prototype_->getPreferredSize();
    Margins margins = prototype_->getMargins();
    if (!preferred.height().isExact()) {
      return {PreferredSize::MatchParentWidth(), preferred.height()};
    }
    int64_t extent =
        static_cast<int64_t>(element_count_) *
            (preferred.height().value() + margins.top() + margins.bottom()) +
        padding_.top() + padding_.bottom();
    CHECK_LE(extent, Rect::MaximumRect().yMax());
    return {PreferredSize::MatchParentWidth(),
            PreferredSize::ExactHeight(extent)};
  }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
    if (!prototype_prepared_) {
      prepareRow(*prototype_);
      prototype_prepared_ = true;
    }
    Margins margins = prototype_->getMargins();
    XDim horizontal =
        padding_.left() + padding_.right() + margins.left() + margins.right();
    PreferredSize preferred = prototype_->getPreferredSize();
    Dimensions measured = prototype_->measure(
        width.getChildWidthSpec(horizontal, preferred.width()),
        preferred.height().isExact()
            ? HeightSpec::Exactly(preferred.height().value())
            : HeightSpec::Unspecified(40));
    row_height_ =
        std::max<YDim>(1, measured.height()) + margins.top() + margins.bottom();
    return {width.resolveSize(measured.width() + horizontal),
            height.resolveSize(contentExtent())};
  }

  void onLayout(bool changed, const Rect& rect) override {
    synchronizing_ = true;
    // Reserve against the whole viewport, including sections currently
    // offscreen. This guarantees bounded keyboard materialization and
    // allocation-free scroll.
    YDim viewport_height =
        getMainWindow() == nullptr ? rect.height() : getMainWindow()->height();
    CHECK_GT(rowStride(), 0);
    size_t capacity = std::min<int64_t>(
        element_count_, std::max<YDim>(0, viewport_height) / rowStride() + 2);
    size_t old_capacity = elements_.capacity();
    if (capacity > old_capacity) {
      releaseRows();
      elements_.ensure_capacity(capacity, prototype_fn_);
    }
    for (size_t i = old_capacity; i < elements_.capacity(); ++i) {
      Widget& row = elements_.storage(i);
      prepareRow(row);
      row.setVisibility(Visibility::kGone);
      add(row);
    }
    for (int index = first_; index <= last_; ++index) {
      layoutRow(index, elements_[index - first_]);
    }
    synchronizing_ = false;
  }

 private:
  void releaseBack() {
    Widget& row = elements_.pop_back();
    --last_;
    row.setVisibility(Visibility::kGone);
    unbindRow(row);
  }

  void layoutRow(int index, Widget& row) {
    Rect bounds = rowBounds(index);
    row.measure(WidthSpec::Exactly(bounds.width()),
                HeightSpec::Exactly(bounds.height()));
    row.layout(bounds);
  }

  void show(int index, Widget& row) {
    bindRow(index, row);
    layoutRow(index, row);
    row.setVisibility(Visibility::kVisible);
  }

  // Range changes are idempotent; resumed painting never rotates or rebinds
  // rows.
  void synchronizeRange(int begin, int end) {
    CHECK(!synchronizing_);
    synchronizing_ = true;
    CHECK_LE(std::max(0, end - begin + 1),
             static_cast<int>(elements_.capacity()));
    while (first_ < begin && first_ <= last_) {
      Widget& row = elements_.pop_front();
      ++first_;
      row.setVisibility(Visibility::kGone);
      unbindRow(row);
    }
    while (end < last_ && first_ <= last_) releaseBack();
    if (first_ > last_) {
      first_ = begin;
      last_ = begin - 1;
    }
    while (first_ > begin) show(--first_, elements_.push_front());
    while (last_ < end) show(++last_, elements_.push_back());
    synchronizing_ = false;
  }

  Padding padding_;
  ListModel& model_;
  CircularBuffer elements_;
  PrototypeFn prototype_fn_;
  std::unique_ptr<Widget> prototype_;
  int element_count_ = 0;
  YDim row_height_ = 1;
  int first_ = 0;
  int last_ = -1;
  bool synchronizing_ = false;
  bool prototype_prepared_ = false;
};

}  // namespace roo_windows
