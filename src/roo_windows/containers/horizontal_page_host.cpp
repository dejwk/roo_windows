#include "roo_windows/containers/horizontal_page_host.h"

#include <algorithm>

#include "roo_logging.h"

namespace roo_windows {

HorizontalPageHost::HorizontalPageHost(ApplicationContext& context)
    : Container(context),
      pages_(),
      page_to_slot_(),
      active_slots_(),
      settled_index_(-1),
      page_position_(0.0f) {}

HorizontalPageHost::~HorizontalPageHost() { clearPages(); }

void HorizontalPageHost::addPage(WidgetRef page) {
  if (page.get() == nullptr) return;
  pages_.push_back(std::move(page));
  page_to_slot_.push_back(-1);
  if (settled_index_ < 0) {
    settled_index_ = 0;
    page_position_ = 0.0f;
    syncActiveSlots();
    requestLayout();
    invalidateInterior();
    return;
  }
  requestLayout();
}

void HorizontalPageHost::clearPages() {
  for (int i = 0; i < kSlotCount; ++i) {
    clearSlot(static_cast<SlotId>(i));
  }
  pages_.clear();
  page_to_slot_.clear();
  settled_index_ = -1;
  page_position_ = 0.0f;
  requestLayout();
  invalidateInterior();
}

int HorizontalPageHost::pageCount() const { return pages_.size(); }

int HorizontalPageHost::currentIndex() const { return settled_index_; }

bool HorizontalPageHost::setCurrentIndex(int index, bool animate) {
  if (index < 0 || index >= pageCount()) return false;
  if (index == settled_index_) return false;
  if (animate) {
    LOG(WARNING) << "Unimplemented: HorizontalPageHost animated settle";
  }
  int old_index = settled_index_;
  settled_index_ = index;
  page_position_ = settled_index_;
  syncActiveSlots();
  requestLayout();
  invalidateInterior();
  onSettledIndexChanged(old_index, settled_index_);
  return true;
}

void HorizontalPageHost::paint(PaintContext& ctx) const { (void)ctx; }

void HorizontalPageHost::onSettledIndexChanged(int old_index, int new_index) {
  (void)old_index;
  (void)new_index;
}

Dimensions HorizontalPageHost::onMeasure(WidthSpec width, HeightSpec height) {
  if (pageCount() == 0) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }

  if (width.kind() == EXACTLY && height.kind() == EXACTLY) {
    Widget* current = active_slots_[kCurrentSlot].widget;
    if (current != nullptr) {
      current->measure(width, height);
    }
    return Dimensions(width.value(), height.value());
  }

  XDim max_width = 0;
  YDim max_height = 0;
  for (WidgetRef& page : pages_) {
    Dimensions d = page->measure(width, height);
    max_width = std::max(max_width, d.width());
    max_height = std::max(max_height, d.height());
  }
  return Dimensions(width.resolveSize(max_width), height.resolveSize(max_height));
}

void HorizontalPageHost::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)page_position_;
  Widget* current = active_slots_[kCurrentSlot].widget;
  if (current == nullptr) return;
  Rect viewport(0, 0, rect.width() - 1, rect.height() - 1);
  current->layout(viewport);
}

int HorizontalPageHost::getChildrenCount() const {
  int count = 0;
  for (const ActiveSlot& slot : active_slots_) {
    if (slot.widget != nullptr) {
      ++count;
    }
  }
  return count;
}

Widget& HorizontalPageHost::getChild(int idx) {
  return *activeChildAt(idx);
}

const Widget& HorizontalPageHost::getChild(int idx) const {
  return *activeChildAt(idx);
}

void HorizontalPageHost::execute(roo_scheduler::ExecutionID id) { (void)id; }

void HorizontalPageHost::syncActiveSlots() {
  clearSlot(kPreviousSlot);
  clearSlot(kNextSlot);
  if (settled_index_ < 0 || settled_index_ >= pageCount()) {
    clearSlot(kCurrentSlot);
    return;
  }
  bindSlotToPage(kCurrentSlot, settled_index_);
}

void HorizontalPageHost::bindSlotToPage(SlotId slot_id, int page_index) {
  DCHECK(page_index >= 0 && page_index < pageCount());
  int existing_slot = page_to_slot_[page_index];
  if (existing_slot == static_cast<int>(slot_id) &&
      active_slots_[slot_id].page_index == page_index) {
    return;
  }

  if (existing_slot >= 0 && existing_slot < kSlotCount) {
    clearSlot(static_cast<SlotId>(existing_slot));
  }

  clearSlot(slot_id);

  Widget* page = pages_[page_index].get();
  attachChild(WidgetRef(*page));
  active_slots_[slot_id].widget = page;
  active_slots_[slot_id].page_index = page_index;
  page_to_slot_[page_index] = static_cast<int8_t>(slot_id);
}

void HorizontalPageHost::clearSlot(SlotId slot_id) {
  ActiveSlot& slot = active_slots_[slot_id];
  if (slot.widget == nullptr) return;
  if (slot.page_index >= 0 && slot.page_index < (int)page_to_slot_.size() &&
      page_to_slot_[slot.page_index] == static_cast<int8_t>(slot_id)) {
    page_to_slot_[slot.page_index] = -1;
  }
  detachChild(slot.widget);
  slot.widget = nullptr;
  slot.page_index = -1;
}

Widget* HorizontalPageHost::activeChildAt(int idx) {
  DCHECK(idx >= 0);
  int found = 0;
  for (ActiveSlot& slot : active_slots_) {
    if (slot.widget == nullptr) continue;
    if (found == idx) return slot.widget;
    ++found;
  }
  CHECK(false);
  return nullptr;
}

const Widget* HorizontalPageHost::activeChildAt(int idx) const {
  DCHECK(idx >= 0);
  int found = 0;
  for (const ActiveSlot& slot : active_slots_) {
    if (slot.widget == nullptr) continue;
    if (found == idx) return slot.widget;
    ++found;
  }
  CHECK(false);
  return nullptr;
}

}  // namespace roo_windows