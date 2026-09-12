#include "roo_windows/containers/horizontal_page_host.h"

#include <algorithm>
#include <cmath>

#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/gesture_detector.h"

namespace roo_windows {

namespace {

constexpr int16_t kMaxOvershootPx = Scaled(40);
constexpr int16_t kSettleFrameMs = 10;
constexpr int16_t kSettleDurationMs = 180;

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
      dragging_(false),
      intercepted_gesture_(false),
      reconcile_when_presented_(false),
      settled_index_(-1),
      target_index_(-1),
      raw_drag_position_(0.0f),
      page_position_(0.0f) {
  for (int i = 0; i < kSlotCount; ++i) {
    slot_wrappers_[i] = std::make_unique<BlitCacheContainer>(context);
    active_slots_[i].wrapper = slot_wrappers_[i].get();
  }
  context.presentations().observe(*this);
}

HorizontalPageHost::~HorizontalPageHost() {
  cancelSettle();
  clearPages();
}

void HorizontalPageHost::addPage(WidgetRef page) {
  if (page.get() == nullptr) return;
  pages_.push_back(std::move(page));
  page_to_slot_.push_back(-1);
  if (settled_index_ < 0) {
    settled_index_ = 0;
    target_index_ = 0;
    raw_drag_position_ = 0.0f;
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
  cancelSettle();
  dragging_ = false;
  intercepted_gesture_ = false;
  reconcile_when_presented_ = false;
  for (int i = 0; i < kSlotCount; ++i) {
    clearSlot(static_cast<SlotId>(i));
  }
  pages_.clear();
  page_to_slot_.clear();
  settled_index_ = -1;
  target_index_ = -1;
  raw_drag_position_ = 0.0f;
  page_position_ = 0.0f;
  requestLayout();
  invalidateInterior();
}

int HorizontalPageHost::pageCount() const { return pages_.size(); }

int HorizontalPageHost::currentIndex() const { return settled_index_; }

int HorizontalPageHost::targetIndex() const { return target_index_; }

bool HorizontalPageHost::setCurrentIndex(int index, bool animate) {
  if (index < 0 || index >= pageCount()) return false;
  if (index == target_index_) return false;
  if (animate && std::abs(index - settled_index_) == 1) {
    startSettleToIndex(index);
    return true;
  }
  snapToIndex(index);
  return true;
}

void HorizontalPageHost::paint(PaintContext& ctx) const {
  // Container paints this surface after children. Child paint exclusions keep
  // active pages intact, so this clears only exposed overscroll strips.
  Container::paint(ctx);
}

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
  return Dimensions(width.resolveSize(max_width),
                    height.resolveSize(max_height));
}

void HorizontalPageHost::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  updateActivePagePositions();
}

bool HorizontalPageHost::onInterceptTouchEvent(const TouchEvent& event) {
  // Keep ownership while a settle is running so a fresh touch can interrupt
  // animation immediately instead of leaking move/up to descendants.
  if (isSettling()) {
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

void HorizontalPageHost::onDragStart(XDim x, YDim y) {
  (void)x;
  (void)y;
  // A new touch should always take over immediately from any in-flight settle
  // animation, so cancel scheduled ticks and start from the current page state.
  cancelSettle();
  reconcile_when_presented_ = false;
  dragging_ = true;
  raw_drag_position_ = page_position_;
  setTargetIndex(resolveGestureSettleTarget(0));
  syncActiveSlots();
  updateActivePagePositions();
}

void HorizontalPageHost::onDrag(XDim x, YDim y, XDim dx, YDim dy) {
  (void)x;
  (void)y;
  (void)dy;
  if (pageCount() <= 1 || width() <= 0 || settled_index_ < 0) {
    return;
  }
  if (!dragging_) {
    dragging_ = true;
    raw_drag_position_ = page_position_;
  }
  raw_drag_position_ -= (float)dx / (float)width();
  page_position_ = applyEdgeResistance(raw_drag_position_);
  setTargetIndex(resolveGestureSettleTarget(0));
  syncActiveSlots();
  updateActivePagePositions();
}

void HorizontalPageHost::onFling(XDim x, YDim y, XDim vx, YDim vy) {
  (void)x;
  (void)y;
  (void)vy;
  if (pageCount() <= 1 || settled_index_ < 0) return;
  int target = resolveGestureSettleTarget(vx);
  startSettleToIndex(target);
}

void HorizontalPageHost::onDragFinished(XDim x, YDim y) {
  (void)x;
  (void)y;
  if (pageCount() > 1 && settled_index_ >= 0 &&
      !isSettling()) {
    int target = resolveGestureSettleTarget(0);
    startSettleToIndex(target);
  }
  dragging_ = false;
  intercepted_gesture_ = false;
}

void HorizontalPageHost::onCancel() {
  Container::onCancel();
  dragging_ = false;
  intercepted_gesture_ = false;
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

Widget& HorizontalPageHost::getChild(int idx) { return *activeChildAt(idx); }

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

void HorizontalPageHost::cancelSettle() {
  context().animations().cancel(*this, kSettle);
}

bool HorizontalPageHost::isSettling() const {
  return context().animations().contains(*this, kSettle);
}

void HorizontalPageHost::startSettleToIndex(int target_index) {
  if (settled_index_ < 0 || pageCount() == 0) return;
  target_index = Clamp(target_index, 0, pageCount() - 1);
  target_index = Clamp(target_index, settled_index_ - 1, settled_index_ + 1);
  cancelSettle();
  setTargetIndex(target_index);

  syncActiveSlots();
  updateActivePagePositions();
  invalidateInterior();

  if (page_position_ == static_cast<float>(target_index)) {
    snapToIndex(target_index);
    return;
  }

  if (presentationState() == PresentationState::kDetached) {
    reconcile_when_presented_ = true;
    return;
  }

  AnimationSpec spec = AnimationSpec::value(
      page_position_, static_cast<float>(target_index),
      roo_time::Millis(kSettleDurationMs));
  spec.minimum_interval = roo_time::Millis(kSettleFrameMs);
  spec.easing.kind = EasingKind::kQuadraticOut;
  if (context().animations().start(*this, kSettle, spec) !=
      AnimationStatus::kOk) {
    snapToIndex(target_index);
    return;
  }
  reconcile_when_presented_ = false;
  if (presentationState() == PresentationState::kHidden) {
    context().animations().pause(*this, kSettle);
  }
}

void HorizontalPageHost::snapToIndex(int target_index) {
  target_index = Clamp(target_index, 0, pageCount() - 1);
  cancelSettle();
  reconcile_when_presented_ = false;
  setTargetIndex(target_index);

  int old_index = settled_index_;
  settled_index_ = target_index;
  raw_drag_position_ = target_index;
  page_position_ = target_index;
  syncActiveSlots();
  updateActivePagePositions();
  requestLayout();
  invalidateInterior();
  if (old_index != settled_index_) {
    onSettledIndexChanged(old_index, settled_index_);
  }
}

void HorizontalPageHost::reconcileToTarget() {
  if (pageCount() == 0 || target_index_ < 0) return;
  settled_index_ = Clamp(target_index_, 0, pageCount() - 1);
  target_index_ = settled_index_;
  raw_drag_position_ = settled_index_;
  page_position_ = settled_index_;
  reconcile_when_presented_ = false;
  syncActiveSlots();
  updateActivePagePositions();
  requestLayout();
  invalidateInterior();
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
  float max_pos =
      settled_index_ + (settled_index_ + 1 < pageCount() ? 1.0f : 0.0f);
  if (width() <= 0) return Clamp(raw_position, min_pos, max_pos);
  if (raw_position < min_pos) {
    float excess_px = (raw_position - min_pos) * width();
    float overshoot_px =
        kMaxOvershootPx * excess_px / (-excess_px + kMaxOvershootPx);
    return min_pos + overshoot_px / width();
  }
  if (raw_position > max_pos) {
    float excess_px = (raw_position - max_pos) * width();
    float overshoot_px =
        kMaxOvershootPx * excess_px / (excess_px + kMaxOvershootPx);
    return max_pos + overshoot_px / width();
  }
  return raw_position;
}

void HorizontalPageHost::onAnimationFrame(
    AnimationTag tag, const AnimationSample& sample) {
  if (tag != kSettle) {
    Container::onAnimationFrame(tag, sample);
    return;
  }
  page_position_ = sample.value;
  syncActiveSlots();
  updateActivePagePositions();
  invalidateInterior();
}

void HorizontalPageHost::onAnimationFinished(AnimationTag tag,
                                             AnimationFinishReason reason) {
  (void)reason;
  if (tag != kSettle) {
    Container::onAnimationFinished(tag, reason);
    return;
  }
  // This may invoke user code and therefore must remain the last operation.
  snapToIndex(target_index_);
}

void HorizontalPageHost::onPresentationChanged(
    const PresentationChange& change) {
  AnimationRegistry& animations = context().animations();
  if (change.detached_since_delivery ||
      change.state == PresentationState::kDetached) {
    animations.cancel(*this, kSettle);
    reconcile_when_presented_ =
        target_index_ >= 0 &&
        (settled_index_ != target_index_ ||
         page_position_ != static_cast<float>(target_index_));
    return;
  }
  if (change.state == PresentationState::kHidden) {
    animations.pause(*this, kSettle);
    return;
  }
  if (reconcile_when_presented_) {
    reconcileToTarget();
    return;
  }
  animations.resume(*this, kSettle);
}

}  // namespace roo_windows
