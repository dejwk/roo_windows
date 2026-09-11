#include "roo_windows/material3/snackbar/snackbar.h"

#include <Arduino.h>

#include <algorithm>

#include "roo_display/ui/text_label.h"
#include "roo_logging.h"
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

bool VisibleAncestors(const Widget& widget) {
  for (const Widget* w = &widget; w != nullptr; w = w->parent()) {
    if (!w->isVisible()) return false;
  }
  return true;
}
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

void SnackbarPresenter::cancelTimer() {
  if (timer_ > 0) host_.context().scheduler().cancel(timer_);
  timer_ = -1;
}

void SnackbarPresenter::schedule() {
  cancelTimer();
  if (head_ != nullptr && !draining_ &&
      (phase_ != Phase::kVisible || timeout_ms_ != 0))
    timer_ =
        host_.context().scheduler().scheduleAfter(roo_time::Millis(20), *this);
}

void SnackbarPresenter::start() {
  cancelTimer();
  elapsed_ms_ = phase_ms_ = 0;
  last_ms_ = millis();
  phase_ = animations_ ? Phase::kEntering : Phase::kVisible;
  SnackbarDuration duration = head_->duration_;
  if (duration == SnackbarDuration::kDefault)
    duration = head_->action_.empty() && !head_->show_dismiss_
                   ? SnackbarDuration::kShort
                   : SnackbarDuration::kPersistent;
  timeout_ms_ = duration == SnackbarDuration::kShort  ? 4000
                : duration == SnackbarDuration::kLong ? 10000
                                                      : 0;
  host_.widget_.setContent(head_->message_, head_->action_,
                           head_->show_dismiss_);
  host_.widget_.setVisibility(Visibility::kVisible);
  host_.placeSnackbar(true);
  schedule();
}

void SnackbarPresenter::finish(SnackbarDismissReason reason, bool notify) {
  if (head_ == nullptr) return;
  cancelTimer();
  SnackbarRequest* old = head_;
  head_ = old->next_;
  old->next_ = nullptr;
  old->presenter_ = nullptr;
  host_.widget_.setVisibility(Visibility::kGone);
  host_.widget_.setContent("", {}, false);
  if (head_ != nullptr && !draining_) start();
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
  phase_ = Phase::kExiting;
  phase_ms_ = 0;
  last_ms_ = millis();
  exit_reason_ = reason;
  schedule();
}

void SnackbarPresenter::drain(SnackbarDismissReason reason) {
  if (draining_) return;
  std::shared_ptr<Lifetime> live = lifetime_;
  draining_ = true;
  cancelTimer();
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
}

void SnackbarPresenter::setAnimationsEnabled(bool enabled) {
  animations_ = enabled;
  if (enabled || head_ == nullptr) return;
  if (phase_ == Phase::kExiting) {
    finish(exit_reason_);
    return;
  }
  phase_ = Phase::kVisible;
  phase_ms_ = 0;
  host_.placeSnackbar(false);
}

float SnackbarPresenter::offset() const {
  if (phase_ == Phase::kEntering)
    return 1.0f - std::min(1.0f, static_cast<float>(phase_ms_) / kEnterMs);
  if (phase_ == Phase::kExiting)
    return std::min(1.0f, static_cast<float>(phase_ms_) / kExitMs);
  return 0;
}

void SnackbarPresenter::update(uint32_t now) {
  const uint32_t delta = now - last_ms_;
  last_ms_ = now;
  if (head_ == nullptr) return;
  bool paused = !VisibleAncestors(host_) || host_.target_.empty();
  MainWindow* window = host_.getMainWindow();
  if (window == nullptr) {
    shutdown();
    return;
  }
  paused |= window->transient_presentation_slot().hasActivePresentation();
  if (!paused) {
    if (phase_ == Phase::kVisible) {
      const Widget* focused = host_.getTask()->focus().focused();
      if (focused != &host_.widget_.actionButton() &&
          focused != &host_.widget_.dismissButton())
        elapsed_ms_ += delta;
      if (timeout_ms_ != 0 && elapsed_ms_ >= timeout_ms_) {
        dismissCurrent(SnackbarDismissReason::kTimeout);
        return;
      }
    } else {
      phase_ms_ += delta;
      if (phase_ == Phase::kExiting && phase_ms_ >= kExitMs) {
        finish(exit_reason_);
        return;
      }
      if (phase_ == Phase::kEntering && phase_ms_ >= kEnterMs)
        phase_ = Phase::kVisible;
      host_.placeSnackbar(false);
    }
  }
  schedule();
}

void SnackbarPresenter::execute(roo_scheduler::ExecutionID id) {
  if (id != timer_) return;
  timer_ = -1;
  update(millis());
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

SnackbarHost::SnackbarHost(ApplicationContext& context)
    : LayoutScaffold(context), presenter_(*this), widget_(context, presenter_) {
  attachChild(widget_);
  widget_.setVisibility(Visibility::kGone);
}

SnackbarHost::~SnackbarHost() {
  detaching_ = true;
  presenter_.shutdown();
  detachChild(&widget_);
}

void SnackbarHost::setParent(Container* parent, bool is_owned) {
  if (parent == nullptr) {
    detaching_ = true;
    presenter_.shutdown();
  }
  LayoutScaffold::setParent(parent, is_owned);
  detaching_ = parent == nullptr;
}

bool SnackbarHost::setSnackbarAvoidance(const Rect* rectangles, size_t count) {
  if (count > 3 || (count != 0 && rectangles == nullptr)) return false;
  for (size_t i = 0; i < count; ++i) avoidance_[i] = rectangles[i];
  avoidance_count_ = count;
  placeSnackbar(true);
  return true;
}

void SnackbarHost::setSnackbarStartAligned(bool start) {
  start_aligned_ = start;
  placeSnackbar(true);
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
