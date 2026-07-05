#include "roo_windows/containers/horizontal_page_host.h"

#include <algorithm>
#include <cmath>

#include "roo_windows/core/application.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_logging.h"

namespace roo_windows {

namespace {

constexpr float kEdgeResistanceFactor = 0.25f;
constexpr int16_t kSettleFrameMs = 10;
constexpr unsigned long kSettleDurationMs = 180;

template <typename T>
T Clamp(T value, T min_value, T max_value) {
  if (value < min_value) return min_value;
  if (value > max_value) return max_value;
  return value;
}

}  // namespace

HorizontalPageHost::HorizontalPageHost(ApplicationContext& context)
    : Container(context),
      pages_(),
      page_to_slot_(),
      slot_wrappers_(),
      active_slots_(),
      scheduler_(context.scheduler()),
      notification_id_(-1),
      animation_state_(AnimationState::kIdle),
      settle_{0, 0, 0.0f, 0.0f, -1, -1},
      dragging_(false),
      intercepted_gesture_(false),
      settled_index_(-1),
      target_index_(-1),
      page_position_(0.0f) {
  for (int i = 0; i < kSlotCount; ++i) {
    slot_wrappers_[i] = std::make_unique<BlitCacheContainer>(context);
    active_slots_[i].wrapper = slot_wrappers_[i].get();
  }
}

HorizontalPageHost::~HorizontalPageHost() {
  cancelPendingUpdate();
  clearPages();
}

void HorizontalPageHost::addPage(WidgetRef page) {
  if (page.get() == nullptr) return;
  pages_.push_back(std::move(page));
  page_to_slot_.push_back(-1);
  if (settled_index_ < 0) {
    settled_index_ = 0;
    target_index_ = 0;
    page_position_ = 0.0f;
    syncActiveSlots();
    requestLayout();
    invalidateInterior();
    return;
  }
  syncActiveSlots();
  requestLayout();
  updateActivePagePositions();
}

void HorizontalPageHost::clearPages() {
  cancelPendingUpdate();
  animation_state_ = AnimationState::kIdle;
  dragging_ = false;
  intercepted_gesture_ = false;
  for (int i = 0; i < kSlotCount; ++i) {
    clearSlot(static_cast<SlotId>(i));
  }
  pages_.clear();
  page_to_slot_.clear();
  settled_index_ = -1;
  target_index_ = -1;
  page_position_ = 0.0f;
  requestLayout();
  invalidateInterior();
}

int HorizontalPageHost::pageCount() const { return pages_.size(); }

int HorizontalPageHost::currentIndex() const { return settled_index_; }

int HorizontalPageHost::targetIndex() const { return target_index_; }

bool HorizontalPageHost::setCurrentIndex(int index, bool animate) {
  if (index < 0 || index >= pageCount()) return false;
  if (index == settled_index_) return false;
  if (animate && std::abs(index - settled_index_) == 1) {
    startSettleToIndex(index);
    return true;
  }
  snapToIndex(index);
  return true;
}

void HorizontalPageHost::paint(PaintContext& ctx) const { (void)ctx; }

void HorizontalPageHost::onSettledIndexChanged(int old_index, int new_index) {
  (void)old_index;
  (void)new_index;
}

void HorizontalPageHost::onTargetIndexChanged(int old_index, int new_index) {
  (void)old_index;
  (void)new_index;
}

Dimensions HorizontalPageHost::onMeasure(WidthSpec width, HeightSpec height) {
  if (pageCount() == 0) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }

  if (width.kind() == EXACTLY && height.kind() == EXACTLY) {
    BlitCacheContainer* current = active_slots_[kCurrentSlot].wrapper;
    if (current != nullptr && active_slots_[kCurrentSlot].attached) {
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
  (void)rect;
  updateActivePagePositions();
}

bool HorizontalPageHost::onInterceptTouchEvent(const TouchEvent& event) {
  // Keep ownership while a settle is running so a fresh touch can interrupt
  // animation immediately instead of leaking move/up to descendants.
  if (animation_state_ == AnimationState::kSettling) {
    intercepted_gesture_ = true;
    return true;
  }
  if (event.type() == TouchEvent::DOWN) {
    // Reset tentative interception on each new gesture sequence.
    intercepted_gesture_ = false;
    return false;
  }
  if (event.type() != TouchEvent::MOVE) {
    return intercepted_gesture_;
  }
  if (intercepted_gesture_ || pageCount() < 2) {
    return intercepted_gesture_;
  }
  const GestureDetector& gd = getApplication()->gesture_detector();
  int16_t dx = gd.xTotalMoveDelta();
  int16_t dy = gd.yTotalMoveDelta();
  int32_t dist_sq = dx * dx + dy * dy;
  if (dist_sq <= kTouchSlopSquare) {
    // Do not steal touch before intent is clear.
    return false;
  }
  if (std::abs(dx) <= std::abs(dy)) {
    // Vertical-dominant movement belongs to scrollable descendants/ancestors.
    return false;
  }
  // Horizontal intent is now clear; claim the gesture for page dragging.
  intercepted_gesture_ = true;
  return true;
}

bool HorizontalPageHost::onDown(XDim x, YDim y) {
  (void)x;
  (void)y;
  // A new touch should always take over immediately from any in-flight settle
  // animation, so cancel scheduled ticks and start from the current page state.
  cancelPendingUpdate();
  animation_state_ = AnimationState::kIdle;
  dragging_ = true;
  page_position_ = settled_index_ >= 0 ? settled_index_ : 0.0f;
  setTargetIndex(settled_index_);
  syncActiveSlots();
  updateActivePagePositions();
  return true;
}

bool HorizontalPageHost::onScroll(XDim x, YDim y, XDim dx, YDim dy) {
  (void)x;
  (void)y;
  (void)dy;
  if (pageCount() <= 1 || width() <= 0 || settled_index_ < 0) {
    return false;
  }
  if (!dragging_) {
    dragging_ = true;
  }
  float raw_position = page_position_ - ((float)dx / (float)width());
  page_position_ = applyEdgeResistance(raw_position);
  setTargetIndex(resolveGestureSettleTarget(0));
  syncActiveSlots();
  updateActivePagePositions();
  invalidateInterior();
  return true;
}

bool HorizontalPageHost::onFling(XDim x, YDim y, XDim vx, YDim vy) {
  (void)x;
  (void)y;
  (void)vy;
  if (pageCount() <= 1 || settled_index_ < 0) return true;
  int target = resolveGestureSettleTarget(vx);
  startSettleToIndex(target);
  return true;
}

bool HorizontalPageHost::onTouchUp(XDim x, YDim y) {
  bool handled = Widget::onTouchUp(x, y);
  if (pageCount() > 1 && settled_index_ >= 0 &&
      animation_state_ != AnimationState::kSettling) {
    int target = resolveGestureSettleTarget(0);
    startSettleToIndex(target);
  }
  dragging_ = false;
  intercepted_gesture_ = false;
  return handled;
}

int HorizontalPageHost::getChildrenCount() const {
  int count = 0;
  for (const ActiveSlot& slot : active_slots_) {
    if (slot.attached) {
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

void HorizontalPageHost::syncActiveSlots() {
  int prev_idx = settled_index_ - 1;
  int next_idx = settled_index_ + 1;

  if (settled_index_ < 0 || settled_index_ >= pageCount()) {
    clearSlot(kPreviousSlot);
    clearSlot(kCurrentSlot);
    clearSlot(kNextSlot);
    return;
  }

  if (prev_idx >= 0) {
    bindSlotToPage(kPreviousSlot, prev_idx);
  } else {
    clearSlot(kPreviousSlot);
  }

  bindSlotToPage(kCurrentSlot, settled_index_);

  if (next_idx < pageCount()) {
    bindSlotToPage(kNextSlot, next_idx);
  } else {
    clearSlot(kNextSlot);
  }
}

void HorizontalPageHost::bindSlotToPage(SlotId slot_id, int page_index) {
  DCHECK(page_index >= 0 && page_index < pageCount());
  ActiveSlot& slot = active_slots_[slot_id];
  int existing_slot = page_to_slot_[page_index];
  if (existing_slot == static_cast<int>(slot_id) &&
      slot.page_index == page_index) {
    return;
  }

  if (existing_slot >= 0 && existing_slot < kSlotCount) {
    clearSlot(static_cast<SlotId>(existing_slot));
  }

  clearSlot(slot_id);

  Widget* page = pages_[page_index].get();
  if (!slot.attached) {
    attachChild(WidgetRef(*slot.wrapper));
    slot.attached = true;
  }
  slot.wrapper->setChild(WidgetRef(*page));
  slot.page = page;
  slot.page_index = page_index;
  page_to_slot_[page_index] = static_cast<int8_t>(slot_id);
}

void HorizontalPageHost::clearSlot(SlotId slot_id) {
  ActiveSlot& slot = active_slots_[slot_id];
  if (slot.page == nullptr && !slot.attached) return;
  if (slot.page_index >= 0 && slot.page_index < (int)page_to_slot_.size() &&
      page_to_slot_[slot.page_index] == static_cast<int8_t>(slot_id)) {
    page_to_slot_[slot.page_index] = -1;
  }
  if (slot.wrapper->child() != nullptr) {
    slot.wrapper->clearChild();
  }
  if (slot.attached) {
    detachChild(slot.wrapper);
    slot.attached = false;
  }
  slot.page = nullptr;
  slot.page_index = -1;
}

Widget* HorizontalPageHost::activeChildAt(int idx) {
  DCHECK(idx >= 0);
  int found = 0;
  for (ActiveSlot& slot : active_slots_) {
    if (!slot.attached) continue;
    if (found == idx) return slot.wrapper;
    ++found;
  }
  CHECK(false);
  return nullptr;
}

const Widget* HorizontalPageHost::activeChildAt(int idx) const {
  DCHECK(idx >= 0);
  int found = 0;
  for (const ActiveSlot& slot : active_slots_) {
    if (!slot.attached) continue;
    if (found == idx) return slot.wrapper;
    ++found;
  }
  CHECK(false);
  return nullptr;
}

void HorizontalPageHost::updateActivePagePositions() {
  if (width() <= 0 || height() <= 0) return;
  for (ActiveSlot& slot : active_slots_) {
    if (!slot.attached || slot.page_index < 0) continue;
    XDim x = (XDim)std::lround((slot.page_index - page_position_) * width());
    Rect rect(x, 0, x + width() - 1, height() - 1);
    if (slot.wrapper->width() != rect.width() ||
        slot.wrapper->height() != rect.height() ||
        slot.wrapper->isLayoutRequired() || slot.wrapper->isLayoutRequested()) {
      slot.wrapper->layout(rect);
    } else {
      slot.wrapper->moveTo(rect);
    }
  }
}

void HorizontalPageHost::cancelPendingUpdate() {
  if (notification_id_ > 0) {
    scheduler_.cancel(notification_id_);
    notification_id_ = -1;
  }
}

void HorizontalPageHost::scheduleSettleUpdate() {
  cancelPendingUpdate();
  notification_id_ = scheduler_.scheduleAfter(roo_time::Millis(kSettleFrameMs), *this);
}

void HorizontalPageHost::startSettleToIndex(int target_index) {
  if (settled_index_ < 0 || pageCount() == 0) return;
  target_index = Clamp(target_index, 0, pageCount() - 1);
  target_index = Clamp(target_index, settled_index_ - 1, settled_index_ + 1);
  cancelPendingUpdate();

  settle_.old_index = settled_index_;
  settle_.target_index = target_index;
  setTargetIndex(target_index);
  settle_.start_position = page_position_;
  settle_.target_position = target_index;
  settle_.start_time_ms = millis();
  settle_.end_time_ms = settle_.start_time_ms + kSettleDurationMs;
  animation_state_ = AnimationState::kSettling;

  syncActiveSlots();
  updateActivePagePositions();
  invalidateInterior();

  if (settle_.start_position == settle_.target_position) {
    snapToIndex(target_index);
    return;
  }

  scheduleSettleUpdate();
}

void HorizontalPageHost::snapToIndex(int target_index) {
  target_index = Clamp(target_index, 0, pageCount() - 1);
  cancelPendingUpdate();
  animation_state_ = AnimationState::kIdle;
  setTargetIndex(target_index);

  int old_index = settled_index_;
  settled_index_ = target_index;
  page_position_ = target_index;
  syncActiveSlots();
  updateActivePagePositions();
  requestLayout();
  invalidateInterior();
  if (old_index != settled_index_) {
    onSettledIndexChanged(old_index, settled_index_);
  }
}

void HorizontalPageHost::setTargetIndex(int target_index) {
  if (pageCount() == 0) {
    target_index = -1;
  } else {
    target_index = Clamp(target_index, 0, pageCount() - 1);
  }
  if (target_index_ == target_index) return;
  int old_index = target_index_;
  target_index_ = target_index;
  onTargetIndexChanged(old_index, target_index_);
}

int HorizontalPageHost::resolveGestureSettleTarget(XDim velocity_x) const {
  if (settled_index_ < 0) return -1;
  int target = settled_index_;
  float offset_pages = page_position_ - settled_index_;
  if (offset_pages >= 0.5f) {
    target = settled_index_ + 1;
  } else if (offset_pages <= -0.5f) {
    target = settled_index_ - 1;
  } else if (std::abs(velocity_x) >=
             (XDim)std::sqrt((float)kMinFlingVelocitySquare)) {
    target = velocity_x < 0 ? settled_index_ + 1 : settled_index_ - 1;
  }
  target = Clamp(target, settled_index_ - 1, settled_index_ + 1);
  return Clamp(target, 0, pageCount() - 1);
}

float HorizontalPageHost::applyEdgeResistance(float raw_position) const {
  if (settled_index_ < 0) return 0.0f;
  float min_pos = settled_index_ - (settled_index_ > 0 ? 1.0f : 0.0f);
  float max_pos = settled_index_ +
                  (settled_index_ + 1 < pageCount() ? 1.0f : 0.0f);
  if (raw_position < min_pos) {
    return min_pos + (raw_position - min_pos) * kEdgeResistanceFactor;
  }
  if (raw_position > max_pos) {
    return max_pos + (raw_position - max_pos) * kEdgeResistanceFactor;
  }
  return raw_position;
}

void HorizontalPageHost::execute(roo_scheduler::ExecutionID id) {
  (void)id;
  notification_id_ = -1;
  if (animation_state_ != AnimationState::kSettling) return;

  unsigned long now = millis();
  if ((long)(now - settle_.end_time_ms) >= 0) {
    snapToIndex(settle_.target_index);
    return;
  }

  float t =
      (float)(now - settle_.start_time_ms) / (float)(settle_.end_time_ms - settle_.start_time_ms);
  t = Clamp(t, 0.0f, 1.0f);
  float eased = 1.0f - (1.0f - t) * (1.0f - t);
  page_position_ =
      settle_.start_position +
      (settle_.target_position - settle_.start_position) * eased;

  syncActiveSlots();
  updateActivePagePositions();
  invalidateInterior();
  scheduleSettleUpdate();
}

}  // namespace roo_windows
