#include "roo_windows/material3/date_picker/date_picker.h"

#include <new>

#include "roo_logging.h"
#include "roo_windows/core/display_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows/material3/menu/menu_geometry.h"

namespace roo_windows::material3 {
using roo_time::CivilDay;

ModalDatePicker::ModalDatePicker(ApplicationContext& context)
    : context_(context) {}

ModalDatePicker::~ModalDatePicker() = default;

void ModalDatePicker::setValue(CivilDay day) {
  CHECK(!isOpen());
  value_ = day;
}

void ModalDatePicker::setToday(CivilDay day) {
  CHECK(!isOpen());
  today_ = day;
}

void ModalDatePicker::setBounds(DatePickerBounds bounds) {
  CHECK(!isOpen());
  bounds_ = bounds;
}

void ModalDatePicker::setDisplayedMonth(CivilDay day) {
  CHECK(!isOpen());
  month_ = internal::MonthStart(day);
}

CivilDay ModalDatePicker::displayedMonth() const {
  return session_ ? session_->month : month_;
}

void ModalDatePicker::setEntryMode(DatePickerEntryMode mode) {
  CHECK(!isOpen());
  entry_mode_ = mode;
}

const DatePickerStrings& ModalDatePicker::datePickerStrings() const {
  return DefaultDatePickerStrings();
}

const DateTextCodec& ModalDatePicker::dateTextCodec() const {
  return DefaultDateTextCodec();
}

PresentationStartResult ModalDatePicker::open(Task& owner) {
  return openAt(owner, nullptr);
}

PresentationStartResult ModalDatePicker::openAt(Task& owner,
                                                const Rect* anchor) {
  if (isOpen()) return PresentationStartResult::kHostBusy;
  if (!bounds_.isValid() || (entry_mode_ != DatePickerEntryMode::kCalendar &&
                             entry_mode_ != DatePickerEntryMode::kInput)) {
    return PresentationStartResult::kSurfaceUnavailable;
  }
  session_.reset(new (std::nothrow) internal::DatePickerSession(*this));
  if (!session_) return PresentationStartResult::kSurfaceUnavailable;
  PresentationStartResult result = session_->show(owner, anchor);
  if (result != PresentationStartResult::kStarted) session_.reset();
  return result;
}

void ModalDatePicker::dismiss(DatePickerDismissReason reason) {
  if (!session_) return;
  session_->dismiss_reason = reason;
  session_->finish(PresentationFinishReason::kCancel);
}

void ModalDatePicker::completed(PresentationFinishReason reason) {
  CivilDay accepted = session_->draft;
  DatePickerDismissReason dismissal = session_->dismiss_reason;
  month_ = session_->month;
  if (reason == PresentationFinishReason::kOutsideInteraction) {
    dismissal = DatePickerDismissReason::kOutsideTap;
  } else if (reason == PresentationFinishReason::kBack) {
    dismissal = DatePickerDismissReason::kBack;
  } else if (reason == PresentationFinishReason::kInteractionOwnerDetached ||
             reason == PresentationFinishReason::kHostDestroyed) {
    dismissal = DatePickerDismissReason::kOwnerUnavailable;
  }
  if (reason == PresentationFinishReason::kAction) value_ = accepted;
  session_.reset();
  // A terminal hook may destroy or reopen this presenter. It is the last
  // access.
  if (reason == PresentationFinishReason::kAction) {
    onAccepted(accepted);
  } else {
    onDismissed(dismissal);
  }
}

namespace internal {
namespace {
/// Prepares local child geometry before the host activates the focus scope.
class PickerPreparation final
    : public ::roo_windows::internal::TransientSurfacePreparation {
 public:
  /// Borrows the panel and its resolved receiving-window rectangle.
  PickerPreparation(DatePickerPanel& panel, Rect bounds)
      : panel_(panel), bounds_(bounds) {}

 private:
  bool createAndResolveBounds(Rect& bounds) override {
    panel_.measure(WidthSpec::Exactly(bounds_.width()),
                   HeightSpec::Exactly(bounds_.height()));
    panel_.layout(Rect(0, 0, bounds_.width() - 1, bounds_.height() - 1));
    bounds = bounds_;
    return true;
  }

  void deleteAfterFailedAdmission() override {}
  DatePickerPanel& panel_;
  Rect bounds_;
};
}  // namespace

DatePickerSession::DatePickerSession(ModalDatePicker& owner)
    : owner_(owner), draft(owner.value_), panel(owner.context_, *this) {
  CivilDay seed = owner.month_;
  if (!seed.isValid()) seed = owner.value_;
  if (!seed.isValid()) seed = owner.today_;
  if (!seed.isValid()) seed = owner.bounds_.first;
  if (!seed.isValid()) seed = owner.bounds_.last;
  if (!seed.isValid()) seed = CivilDay::FromYmd(1970, 1, 1);
  month = MonthStart(seed);
  if (!enabled(draft)) draft = CivilDay::Invalid();
  panel.updateSelection();
  if (owner.entry_mode_ == DatePickerEntryMode::kInput) {
    panel.setMode(DatePickerMode::kInput);
  }
}

DatePickerSession::~DatePickerSession() { cancel(); }

bool DatePickerSession::enabled(CivilDay day) const {
  return owner_.bounds_.contains(day) && owner_.isDateEnabled(day);
}

PresentationStartResult DatePickerSession::show(Task& task,
                                                const Rect* anchor) {
  if (owner_.entry_mode_ == DatePickerEntryMode::kInput &&
      panel.mode() != DatePickerMode::kInput) {
    return PresentationStartResult::kSurfaceUnavailable;
  }
  const Rect window = task.window().root().bounds();
  const Dimensions desired = panel.getSuggestedMinimumDimensions();
  const int margin = Scaled(16);
  // Below this size even pinned actions and one full calendar target cannot
  // fit.
  if (window.width() < Scaled(240) || window.height() < Scaled(240)) {
    return PresentationStartResult::kSurfaceUnavailable;
  }
  bool fits = window.width() >= desired.width() + 2 * margin &&
              window.height() >= desired.height() + 2 * margin;
  bool docked = anchor != nullptr && fits;
  Rect bounds;
  if (fits) {
    if (docked) {
      Rect viewport(margin, margin, window.width() - margin - 1,
                    window.height() - margin - 1);
      bounds = ResolveRootMenuPlacement(viewport, *anchor, desired,
                                        MenuPlacement::kBelowStart,
                                        LayoutDirection::kLeftToRight)
                   .bounds;
    } else {
      int x = (window.width() - desired.width()) / 2;
      int y = (window.height() - desired.height()) / 2;
      bounds = Rect(x, y, x + desired.width() - 1, y + desired.height() - 1);
    }
  } else {
    panel.setFullScreen(true);
    bounds = window;
  }
  PickerPreparation preparation(panel, bounds);
  const TransientSurfaceSpec spec{docked ? TransientBarrierPaint::kTransparent
                                         : TransientBarrierPaint::kScrim,
                                  TransientAdmissionPolicy::kRejectIfBusy,
                                  OutsideInteractionPolicy::kDismiss,
                                  TransientPresentationPolicy(true, true),
                                  false};
  return ::roo_windows::internal::GetTransientSurfaceHost(task).showPrepared(
      *this, task, panel, scope_, spec, preparation);
}

void DatePickerSession::accept() {
  if (enabled(draft)) finish(PresentationFinishReason::kAction);
}

void DatePickerSession::onFinished(PresentationFinishReason reason) {
  owner_.completed(reason);
}

BackResult DatePickerSession::onBackRequested(BackSource) {
  if (panel.mode() == DatePickerMode::kMonths ||
      panel.mode() == DatePickerMode::kYears) {
    panel.setMode(DatePickerMode::kDays);
  } else {
    finish(PresentationFinishReason::kBack);
  }
  return BackResult::kHandled;
}
}  // namespace internal
}  // namespace roo_windows::material3
