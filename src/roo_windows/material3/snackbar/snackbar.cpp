#include "roo_windows/material3/snackbar/snackbar.h"

#include <algorithm>

#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
namespace {
constexpr int16_t kMargin = Scaled(16);
constexpr int16_t kGap = Scaled(8);
constexpr int16_t kVerticalPadding = Scaled(12);
constexpr int16_t kControlHeight = Scaled(48);
constexpr uint32_t kEnterMs = 150;
constexpr uint32_t kExitMs = 100;
}  // namespace

SnackbarRequest::~SnackbarRequest() {
  if (presenter_ != nullptr) presenter_->cancel(*this);
}

bool SnackbarRequest::configure(roo::string_view message,
                                roo::string_view action,
                                SnackbarDuration duration, bool show_dismiss) {
  if (isRegistered() || message.size() > 256 || action.size() > 64)
    return false;
  message_ = std::string(message.data(), message.size());
  action_ = std::string(action.data(), action.size());
  duration_ = duration;
  show_dismiss_ = show_dismiss;
  return true;
}

SnackbarWidget::Control::Control(SnackbarWidget& owner, bool dismiss)
    : Button(owner.context(), {}, ButtonVariant::kText),
      owner_(owner),
      dismiss_(dismiss) {}

Color SnackbarWidget::Control::background() const {
  return owner_.background();
}

void SnackbarWidget::Control::paint(PaintContext& ctx) const {
  const TextStyle& style = text_style_label_large();
  Color color = dismiss_ ? theme().material3Theme().color.inverseOnSurface
                         : theme().material3Theme().color.inversePrimary;
  ctx.canvas().drawTiled(roo_display::StringViewLabel(
                             label(), style.font(), color, style.fontOptions()),
                         bounds(), roo_display::kCenter | roo_display::kMiddle);
}

void SnackbarWidget::Control::onClicked() {
  if (dismiss_)
    owner_.onDismiss();
  else
    owner_.onAction();
}

void SnackbarWidget::Control::onFocusChanged(bool focused) {
  owner_.onControlFocusChanged(focused);
  Button::onFocusChanged(focused);
}

SnackbarWidget::SnackbarWidget(ApplicationContext& context)
    : Container(context),
      message_(context, "", text_style_body_medium()),
      action_(*this, false),
      dismiss_(*this, true) {
  message_.setMaxLines(2);
  message_.setEllipsize(true);
  message_.setColor(theme().material3Theme().color.inverseOnSurface);
  attachChild(message_);
  attachChild(action_);
  attachChild(dismiss_);
  setContent("", {}, false);
}

SnackbarWidget::~SnackbarWidget() {
  detachChild(&dismiss_);
  detachChild(&action_);
  detachChild(&message_);
}

void SnackbarWidget::setContent(const std::string& message,
                                roo::string_view action, bool show_dismiss) {
  message_.setText(message);
  action_.setLabel(action);
  action_.setVisibility(action.empty() ? Visibility::kGone
                                       : Visibility::kVisible);
  dismiss_.setLabel("Close");
  dismiss_.setVisibility(show_dismiss ? Visibility::kVisible
                                      : Visibility::kGone);
  requestLayout();
}

Color SnackbarWidget::background() const {
  return theme().material3Theme().color.inverseSurface;
}

BorderStyle SnackbarWidget::getBorderStyle() const {
  return BorderStyle(Scaled(4), 0);
}

const Widget& SnackbarWidget::getChild(int index) const {
  if (index == 0) return message_;
  if (index == 1) return action_;
  return dismiss_;
}

Widget& SnackbarWidget::getChild(int index) {
  if (index == 0) return message_;
  if (index == 1) return action_;
  return dismiss_;
}

Dimensions SnackbarWidget::onMeasure(WidthSpec width, HeightSpec height) {
  const int16_t w = width.resolveSize(Scaled(344));
  const int16_t inner = std::max<int16_t>(0, w - 2 * kMargin);
  action_width_ = action_.isGone()
                      ? 0
                      : action_
                            .measure(WidthSpec::AtMost(inner),
                                     HeightSpec::Exactly(kControlHeight))
                            .width();
  dismiss_width_ = dismiss_.isGone()
                       ? 0
                       : dismiss_
                             .measure(WidthSpec::AtMost(inner),
                                      HeightSpec::Exactly(kControlHeight))
                             .width();
  const int16_t controls = action_width_ + dismiss_width_;
  separate_dismiss_ = controls > inner;
  stacked_ = controls > 0 && inner - controls - kGap < Scaled(80);
  const int16_t text_width = std::max<int16_t>(
      0, inner - (!stacked_ && controls > 0 ? controls + kGap : 0));
  message_height_ =
      message_
          .measure(
              WidthSpec::Exactly(text_width),
              height.kind() == MeasureSpecKind::UNSPECIFIED
                  ? HeightSpec::Unspecified(0)
                  : HeightSpec::AtMost(std::max<int16_t>(0, height.value())))
          .height();
  int16_t h = message_height_ + 2 * kVerticalPadding;
  if (controls > 0)
    h = stacked_ ? h + kControlHeight * (separate_dismiss_ ? 2 : 1)
                 : std::max(h, kControlHeight);
  return Dimensions(w, height.resolveSize(std::max(h, kControlHeight)));
}

void SnackbarWidget::onLayout(bool changed, const Rect& rect) {
  const int16_t controls = action_width_ + dismiss_width_;
  const int16_t text_width = std::max<int16_t>(
      0, rect.width() - 2 * kMargin -
             (!stacked_ && controls > 0 ? controls + kGap : 0));
  const int16_t tx =
      kMargin + (rtl_ && !stacked_ && controls > 0 ? controls + kGap : 0);
  const int16_t ty =
      stacked_ ? kVerticalPadding : (rect.height() - message_height_) / 2;
  message_.setTextAlign(rtl_ ? TextAlign::kEnd : TextAlign::kStart);
  message_.layout(Rect(tx, ty, tx + text_width - 1, ty + message_height_ - 1));
  if (separate_dismiss_) {
    const int16_t ay = rect.height() - 2 * kControlHeight;
    const int16_t ax = rtl_ ? kMargin : rect.width() - kMargin - action_width_;
    const int16_t dx = rtl_ ? kMargin : rect.width() - kMargin - dismiss_width_;
    action_.layout(
        Rect(ax, ay, ax + action_width_ - 1, ay + kControlHeight - 1));
    dismiss_.layout(Rect(dx, ay + kControlHeight, dx + dismiss_width_ - 1,
                         ay + 2 * kControlHeight - 1));
    return;
  }
  const int16_t cy = stacked_ ? rect.height() - kControlHeight
                              : (rect.height() - kControlHeight) / 2;
  int16_t cx = rtl_ ? kMargin : rect.width() - kMargin - controls;
  if (rtl_) {
    dismiss_.layout(
        Rect(cx, cy, cx + dismiss_width_ - 1, cy + kControlHeight - 1));
    cx += dismiss_width_;
    action_.layout(
        Rect(cx, cy, cx + action_width_ - 1, cy + kControlHeight - 1));
  } else {
    action_.layout(
        Rect(cx, cy, cx + action_width_ - 1, cy + kControlHeight - 1));
    cx += action_width_;
    dismiss_.layout(
        Rect(cx, cy, cx + dismiss_width_ - 1, cy + kControlHeight - 1));
  }
}

SnackbarPresenter::SnackbarPresenter(SnackbarHost& host)
    : host_(host), lifetime_(std::make_shared<Lifetime>(Lifetime{this})) {}

SnackbarPresenter::~SnackbarPresenter() {
  shutdown();
  lifetime_->owner = nullptr;
}

bool SnackbarPresenter::available() const {
  return !draining_ && !host_.detaching_ && host_.parent() != nullptr &&
         host_.parent()->parent() == host_.getMainWindow() &&
         host_.getTask() != nullptr &&
         host_.getTask()->navigation().isAvailable();
}

size_t SnackbarPresenter::pendingCount() const {
  size_t count = 0;
  for (SnackbarRequest* p = head_; p != nullptr; p = p->next_) ++count;
  return count == 0 ? 0 : count - 1;
}

SnackbarShowResult SnackbarPresenter::show(SnackbarRequest& request) {
  if (request.isRegistered()) return SnackbarShowResult::kAlreadyRegistered;
  if (!available()) return SnackbarShowResult::kHostUnavailable;
  if (head_ != nullptr && pendingCount() >= 3)
    return SnackbarShowResult::kQueueFull;
  request.presenter_ = this;
  if (head_ == nullptr) {
    head_ = &request;
    start();
    return SnackbarShowResult::kShown;
  }
  SnackbarRequest* tail = head_;
  while (tail->next_ != nullptr) tail = tail->next_;
  tail->next_ = &request;
  return SnackbarShowResult::kQueued;
}

SnackbarShowResult SnackbarPresenter::replaceCurrent(SnackbarRequest& request) {
  if (request.isRegistered()) return SnackbarShowResult::kAlreadyRegistered;
  if (!available()) return SnackbarShowResult::kHostUnavailable;
  if (head_ == nullptr) return show(request);
  SnackbarRequest* old = head_;
  request.next_ = old->next_;
  request.presenter_ = this;
  head_ = &request;
  old->next_ = nullptr;
  old->presenter_ = nullptr;
  start();
  old->onFinished(SnackbarDismissReason::kReplaced);
  return SnackbarShowResult::kShown;
}

void SnackbarPresenter::cancelMotion() {
  Application* app = host_.getApplication();
  if (app != nullptr) {
    app->context().animations().cancel(host_, SnackbarHost::kMotion);
  }
}

void SnackbarPresenter::consumeReadableTime(roo_time::Uptime now) {
  if (timeout_id_ < 0 || !timeout_enabled_) return;
  roo_time::Duration elapsed = now - timeout_anchor_;
  if (elapsed <= roo_time::Duration()) return;
  if (elapsed >= timeout_remaining_) {
    timeout_remaining_ = roo_time::Duration();
  } else {
    timeout_remaining_ -= elapsed;
  }
}

void SnackbarPresenter::cancelTimeout(roo_time::Uptime now,
                                      bool consume_elapsed) {
  if (timeout_id_ < 0) return;
  if (consume_elapsed) consumeReadableTime(now);
  Application* app = host_.getApplication();
  if (app != nullptr) app->context().scheduler().cancel(timeout_id_);
  timeout_id_ = -1;
}

void SnackbarPresenter::armTimeout(roo_time::Uptime now) {
  if (!timeout_enabled_ || timeout_id_ >= 0 || head_ == nullptr ||
      phase_ != Phase::kVisible || readableTimePaused()) {
    return;
  }
  Application* app = host_.getApplication();
  if (app == nullptr) return;
  timeout_anchor_ = now;
  timeout_id_ = app->context().scheduler().scheduleAfter(
      timeout_remaining_, *this);
}

bool SnackbarPresenter::motionPaused() const {
  return host_.detaching_ ||
         host_.presentationState() != PresentationState::kPresented ||
         host_.target_.empty() || transient_active_;
}

bool SnackbarPresenter::readableTimePaused() const {
  return motionPaused() || control_focused_;
}

void SnackbarPresenter::reconcile(roo_time::Uptime now) {
  if (head_ == nullptr || draining_) {
    cancelTimeout(now, false);
    return;
  }
  Application* app = host_.getApplication();
  if (app == nullptr) {
    cancelTimeout(now, false);
    return;
  }
  AnimationRegistry& animations = app->context().animations();
  if (animations.contains(host_, SnackbarHost::kMotion)) {
    if (motionPaused()) {
      animations.pause(host_, SnackbarHost::kMotion);
    } else {
      animations.resume(host_, SnackbarHost::kMotion);
    }
  }
  if (phase_ != Phase::kVisible || readableTimePaused()) {
    cancelTimeout(now);
  } else {
    armTimeout(now);
  }
}

bool SnackbarPresenter::startMotion(float from, float to,
                                    roo_time::Duration duration) {
  offset_ = from;
  host_.placeSnackbar(false);
  AnimationSpec spec = AnimationSpec::value(from, to, duration);
  spec.minimum_interval = roo_time::Millis(20);
  spec.easing.kind = EasingKind::kLinear;
  Application* app = host_.getApplication();
  if (app == nullptr ||
      app->context().animations().start(host_, SnackbarHost::kMotion, spec) !=
      AnimationStatus::kOk) {
    offset_ = to;
    host_.placeSnackbar(false);
    return false;
  }
  return true;
}

void SnackbarPresenter::start() {
  const roo_time::Uptime now = roo_time::Uptime::Now();
  cancelMotion();
  cancelTimeout(now, false);
  phase_ = animations_ ? Phase::kEntering : Phase::kVisible;
  SnackbarDuration duration = head_->duration_;
  if (duration == SnackbarDuration::kDefault)
    duration = head_->action_.empty() && !head_->show_dismiss_
                   ? SnackbarDuration::kShort
                   : SnackbarDuration::kPersistent;
  timeout_enabled_ = duration != SnackbarDuration::kPersistent;
  timeout_remaining_ = duration == SnackbarDuration::kShort
                           ? roo_time::Seconds(4)
                       : duration == SnackbarDuration::kLong
                           ? roo_time::Seconds(10)
                           : roo_time::Duration();
  host_.widget_.setContent(head_->message_, head_->action_,
                           head_->show_dismiss_);
  host_.widget_.setVisibility(Visibility::kVisible);
  host_.placeSnackbar(true);
  host_.observeTransientActivity();
  if (animations_ &&
      !startMotion(1.0f, 0.0f, roo_time::Millis(kEnterMs))) {
    phase_ = Phase::kVisible;
  } else if (!animations_) {
    offset_ = 0.0f;
    host_.placeSnackbar(false);
  }
  reconcile(now);
}

void SnackbarPresenter::finish(SnackbarDismissReason reason, bool notify) {
  if (head_ == nullptr) return;
  cancelMotion();
  cancelTimeout(roo_time::Uptime::Now(), false);
  // Suppress readable-time rearming if hiding the old visual clears focus
  // before the next request is configured.
  phase_ = Phase::kExiting;
  SnackbarRequest* old = head_;
  head_ = old->next_;
  old->next_ = nullptr;
  old->presenter_ = nullptr;
  host_.widget_.setVisibility(Visibility::kGone);
  host_.widget_.setContent("", {}, false);
  if (head_ != nullptr && !draining_) {
    start();
  } else {
    host_.unobserveTransientActivity();
  }
  if (notify) old->onFinished(reason);  // Terminal: callback may destroy host.
}

void SnackbarPresenter::cancel(SnackbarRequest& request) {
  if (&request == head_) {
    finish(SnackbarDismissReason::kProgrammatic, false);
    return;
  }
  SnackbarRequest* prev = head_;
  while (prev != nullptr && prev->next_ != &request) prev = prev->next_;
  if (prev != nullptr) prev->next_ = request.next_;
  request.next_ = nullptr;
  request.presenter_ = nullptr;
}

void SnackbarPresenter::dismissCurrent(SnackbarDismissReason reason) {
  if (head_ == nullptr || phase_ == Phase::kExiting) return;
  if (!animations_) {
    finish(reason);
    return;
  }
  cancelTimeout(roo_time::Uptime::Now());
  phase_ = Phase::kExiting;
  exit_reason_ = reason;
  if (!startMotion(offset_, 1.0f, roo_time::Millis(kExitMs))) {
    finish(reason);
    return;
  }
  reconcile(roo_time::Uptime::Now());
}

void SnackbarPresenter::drain(SnackbarDismissReason reason) {
  if (draining_) return;
  std::shared_ptr<Lifetime> live = lifetime_;
  draining_ = true;
  cancelMotion();
  cancelTimeout(roo_time::Uptime::Now(), false);
  while (head_ != nullptr) {
    finish(reason);
    if (live->owner == nullptr) return;
  }
  draining_ = false;
}

void SnackbarPresenter::clear() { drain(SnackbarDismissReason::kCleared); }

void SnackbarPresenter::shutdown() {
  draining_ = false;
  drain(SnackbarDismissReason::kHostUnavailable);
  host_.unobserveTransientActivity();
}

void SnackbarPresenter::setAnimationsEnabled(bool enabled) {
  animations_ = enabled;
  if (enabled || head_ == nullptr) return;
  cancelMotion();
  cancelTimeout(roo_time::Uptime::Now());
  if (phase_ == Phase::kExiting) {
    finish(exit_reason_);
    return;
  }
  phase_ = Phase::kVisible;
  offset_ = 0.0f;
  host_.placeSnackbar(false);
  reconcile(roo_time::Uptime::Now());
}

void SnackbarPresenter::motionFrame(float value) {
  if (head_ == nullptr || phase_ == Phase::kVisible) return;
  offset_ = std::max(0.0f, std::min(1.0f, value));
  host_.placeSnackbar(false);
}

void SnackbarPresenter::motionFinished(AnimationFinishReason reason) {
  (void)reason;
  if (head_ == nullptr) return;
  if (phase_ == Phase::kEntering) {
    offset_ = 0.0f;
    phase_ = Phase::kVisible;
    host_.placeSnackbar(false);
    reconcile(roo_time::Uptime::Now());
    return;
  }
  if (phase_ == Phase::kExiting) {
    offset_ = 1.0f;
    host_.placeSnackbar(false);
    finish(exit_reason_);  // Terminal: callback may destroy the host.
  }
}

void SnackbarPresenter::presentationOrLayoutChanged() {
  reconcile(roo_time::Uptime::Now());
}

void SnackbarPresenter::transientActivityChanged(bool active) {
  transient_active_ = active;
  reconcile(roo_time::Uptime::Now());
}

void SnackbarPresenter::controlFocusChanged(bool focused) {
  control_focused_ = focused;
  reconcile(roo_time::Uptime::Now());
}

void SnackbarPresenter::execute(roo_scheduler::ExecutionID id) {
  if (id != timeout_id_) return;
  const roo_time::Uptime now = roo_time::Uptime::Now();
  consumeReadableTime(now);
  timeout_id_ = -1;
  if (head_ == nullptr || phase_ != Phase::kVisible ||
      readableTimePaused()) {
    return;
  }
  if (timeout_remaining_ > roo_time::Duration()) {
    armTimeout(now);
    return;
  }
  dismissCurrent(SnackbarDismissReason::kTimeout);
}

SnackbarHost::Visual::Visual(ApplicationContext& context,
                             SnackbarPresenter& presenter)
    : SnackbarWidget(context), presenter_(presenter) {}
void SnackbarHost::Visual::onAction() {
  presenter_.dismissCurrent(SnackbarDismissReason::kAction);
}
void SnackbarHost::Visual::onDismiss() {
  presenter_.dismissCurrent(SnackbarDismissReason::kDismiss);
}
void SnackbarHost::Visual::onControlFocusChanged(bool focused) {
  presenter_.controlFocusChanged(focused);
}

SnackbarHost::SnackbarHost(ApplicationContext& context)
    : LayoutScaffold(context), presenter_(*this), widget_(context, presenter_) {
  context.presentations().observe(*this);
  attachChild(widget_);
  widget_.setVisibility(Visibility::kGone);
}

SnackbarHost::~SnackbarHost() {
  detaching_ = true;
  unobserveTransientActivity();
  presenter_.shutdown();
  detachChild(&widget_);
}

void SnackbarHost::setParent(Container* parent, bool is_owned) {
  if (parent == nullptr) {
    detaching_ = true;
    unobserveTransientActivity();
    presenter_.shutdown();
  }
  LayoutScaffold::setParent(parent, is_owned);
  detaching_ = parent == nullptr;
}

void SnackbarHost::onTransientActivityChanged(bool active) {
  presenter_.transientActivityChanged(active);
}

void SnackbarHost::onAnimationFrame(AnimationTag tag,
                                    const AnimationSample& sample) {
  if (tag != kMotion) {
    LayoutScaffold::onAnimationFrame(tag, sample);
    return;
  }
  presenter_.motionFrame(sample.value);
}

void SnackbarHost::onAnimationFinished(AnimationTag tag,
                                       AnimationFinishReason reason) {
  if (tag != kMotion) {
    LayoutScaffold::onAnimationFinished(tag, reason);
    return;
  }
  presenter_.motionFinished(reason);
}

void SnackbarHost::onPresentationChanged(const PresentationChange& change) {
  (void)change;
  presenter_.presentationOrLayoutChanged();
}

void SnackbarHost::observeTransientActivity() {
  MainWindow* window = getMainWindow();
  if (window == nullptr) return;
  TransientPresentationSlot& slot =
      window->transient_presentation_slot();
  presenter_.transientActivityChanged(slot.hasActivePresentation());
  slot.observeActivity(*this);
}

void SnackbarHost::unobserveTransientActivity() {
  MainWindow* window = getMainWindow();
  if (window != nullptr) {
    window->transient_presentation_slot().unobserveActivity(*this);
  }
  presenter_.transientActivityChanged(false);
}

bool SnackbarHost::setSnackbarAvoidance(const Rect* rectangles, size_t count) {
  if (count > 3 || (count != 0 && rectangles == nullptr)) return false;
  for (size_t i = 0; i < count; ++i) avoidance_[i] = rectangles[i];
  avoidance_count_ = count;
  placeSnackbar(true);
  presenter_.presentationOrLayoutChanged();
  return true;
}

void SnackbarHost::setSnackbarStartAligned(bool start) {
  start_aligned_ = start;
  placeSnackbar(true);
  presenter_.presentationOrLayoutChanged();
}

int SnackbarHost::getChildrenCount() const {
  return LayoutScaffold::getChildrenCount() + 1;
}
const Widget& SnackbarHost::getChild(int index) const {
  if (index == LayoutScaffold::getChildrenCount()) return widget_;
  return LayoutScaffold::getChild(index);
}
Widget& SnackbarHost::getChild(int index) {
  if (index == LayoutScaffold::getChildrenCount()) return widget_;
  return LayoutScaffold::getChild(index);
}

void SnackbarHost::onLayout(bool changed, const Rect& rect) {
  LayoutScaffold::onLayout(changed, rect);
  placeSnackbar(true);
  presenter_.presentationOrLayoutChanged();
}

void SnackbarHost::placeSnackbar(bool measure) {
  if (!presenter_.isShowing()) return;
  const Rect band = bodyBounds();
  if (measure) {
    const int16_t margin =
        std::min<int16_t>(kMargin, std::max<int16_t>(0, band.width() / 4));
    const int16_t w = std::min<int16_t>(
        Scaled(568), std::max<int16_t>(0, band.width() - 2 * margin));
    widget_.rtl_ = layoutDirection() == LayoutDirection::kRightToLeft;
    const Dimensions size = widget_.measure(
        WidthSpec::Exactly(w),
        HeightSpec::AtMost(std::max<int16_t>(0, band.height() - 2 * margin)));
    int16_t x = band.xMin() + (band.width() - w) / 2;
    if (start_aligned_)
      x = widget_.rtl_ ? band.xMax() - margin - w + 1 : band.xMin() + margin;
    int16_t y = band.yMax() - margin - size.height() + 1;
    for (uint8_t i = 0; i < avoidance_count_; ++i) {
      const Rect& obstacle = avoidance_[i];
      if (!obstacle.empty() && obstacle.xMin() < x + w && obstacle.xMax() >= x)
        y = std::min<int16_t>(y, obstacle.yMin() - kGap - size.height());
    }
    y = std::max<int16_t>(y, band.yMin());
    target_ = band.empty() ? Rect(0, 0, -1, -1)
                           : Rect(x, y, x + w - 1, y + size.height() - 1);
  }
  Rect position = target_;
  position = position.translate(
      0, static_cast<int16_t>(Scaled(12) * presenter_.offset()));
  widget_.layout(position);
}

}  // namespace roo_windows::material3
