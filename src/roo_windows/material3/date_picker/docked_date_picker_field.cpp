#include "roo_windows/material3/date_picker/docked_date_picker_field.h"

#include <new>

#include "roo_icons/filled/24/action.h"
#include "roo_logging.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/transient_surface_host.h"

namespace roo_windows::material3 {
using roo_time::CivilDay;
namespace {
const MonoIcon& CalendarIcon() {
  static const MonoIcon icon = ic_filled_24_action_calendar_today();
  return icon;
}
}  // namespace

class DockedDatePickerField::Picker final : public ModalDatePicker,
                                            private roo_scheduler::Executable {
 public:
  explicit Picker(DockedDatePickerField& field)
      : ModalDatePicker(field.context()), field_(field) {}

  ~Picker() override { cancelPending(); }

  void schedule() {
    pending_ = field_.context().scheduler().scheduleOn(
        roo_time::Uptime::Now(), *this, roo_scheduler::PRIORITY_NORMAL);
  }

  void cancelPending() {
    if (pending_ >= 0) field_.context().scheduler().cancel(pending_);
    pending_ = -1;
  }

  bool pending() const { return pending_ >= 0; }

 protected:
  bool isDateEnabled(CivilDay day) const override {
    return field_.isDateEnabled(day);
  }
  const DatePickerStrings& datePickerStrings() const override {
    return field_.datePickerStrings();
  }
  const DateTextCodec& dateTextCodec() const override {
    return field_.dateTextCodec();
  }
  void onAccepted(CivilDay) override {
    field_.completed(true, DatePickerDismissReason::kCancel);
  }
  void onDismissed(DatePickerDismissReason reason) override {
    field_.completed(false, reason);
  }

 private:
  void execute(roo_scheduler::ExecutionID id) override {
    if (id != pending_) return;
    pending_ = -1;
    field_.openPicker();
  }

  DockedDatePickerField& field_;
  roo_scheduler::ExecutionID pending_ = -1;
};

DockedDatePickerField::DockedDatePickerField(ApplicationContext& context,
                                             roo::string_view label,
                                             TextFieldVariant variant)
    : TextField(context, label, variant) {
  setTrailingIcon(&CalendarIcon());
}

DockedDatePickerField::~DockedDatePickerField() {
  context().presentations().unobserve(*this);
  changing_text_ = true;
  picker_.reset();
}

void DockedDatePickerField::setDate(CivilDay day) {
  CHECK(!isPickerOpen());
  date_ = day;
  char text[64] = {};
  if (day.isValid()) dateTextCodec().format(day, text, sizeof(text));
  changing_text_ = true;
  setText(text);
  changing_text_ = false;
  clearError();
}

void DockedDatePickerField::setToday(CivilDay day) {
  CHECK(!isPickerOpen());
  today_ = day;
}

void DockedDatePickerField::setBounds(DatePickerBounds bounds) {
  CHECK(!isPickerOpen());
  bounds_ = bounds;
}

void DockedDatePickerField::setDisplayedMonth(CivilDay day) {
  CHECK(!isPickerOpen());
  month_ = internal::MonthStart(day);
}

CivilDay DockedDatePickerField::displayedMonth() const {
  return picker_ ? picker_->displayedMonth() : month_;
}

const DatePickerStrings& DockedDatePickerField::datePickerStrings() const {
  return DefaultDatePickerStrings();
}
const DateTextCodec& DockedDatePickerField::dateTextCodec() const {
  return DefaultDateTextCodec();
}

bool DockedDatePickerField::parseText(CivilDay& day) const {
  return dateTextCodec().parse(text(), day).status ==
             roo_time::TextStatus::kOk &&
         bounds_.contains(day) && isDateEnabled(day);
}

PresentationStartResult DockedDatePickerField::openPicker() {
  if (picker_ && picker_->isOpen()) return PresentationStartResult::kHostBusy;
  if (picker_) picker_->cancelPending();
  Task* owner = getTask();
  ::roo_windows::internal::TransientSourceGeometry geometry;
  if (!isEnabled() || owner == nullptr ||
      !::roo_windows::internal::CaptureTransientSourceGeometry(*owner, *this,
                                                               geometry)) {
    picker_.reset();
    return PresentationStartResult::kInteractionOwnerUnavailable;
  }
  if (!picker_) picker_.reset(new (std::nothrow) Picker(*this));
  if (!picker_) return PresentationStartResult::kSurfaceUnavailable;
  CivilDay seed;
  // An unconfirmed but valid typed date seeds the draft without committing it.
  picker_->setValue(parseText(seed) ? seed : date_);
  picker_->setToday(today_);
  picker_->setBounds(bounds_);
  picker_->setDisplayedMonth(month_);
  PresentationStartResult result =
      picker_->openAt(*owner, &geometry.bounds_in_window);
  if (result != PresentationStartResult::kStarted) {
    picker_.reset();
    return result;
  }
  context().presentations().observe(*this);
  return result;
}

void DockedDatePickerField::dismissPicker(DatePickerDismissReason reason) {
  if (!picker_) return;
  if (picker_->isOpen())
    picker_->dismiss(reason);
  else
    picker_.reset();
}

void DockedDatePickerField::onClicked() {
  // Focus schedules admission; the explicit click completes it immediately.
  requestFocus();
  openPicker();
}

void DockedDatePickerField::onFocusChanged(bool focused) {
  TextField::onFocusChanged(focused);
  if (!focused && picker_ && picker_->pending()) picker_.reset();
  if (focused && !picker_ && !changing_text_) {
    // FocusManager is still updating its current target here. Admit only after
    // that operation unwinds; opening a scope synchronously would reenter it.
    picker_.reset(new (std::nothrow) Picker(*this));
    if (picker_) picker_->schedule();
  }
}

void DockedDatePickerField::onPresentationChanged(
    const PresentationChange& change) {
  if (picker_ && (change.state != PresentationState::kPresented ||
                  change.detached_since_delivery)) {
    dismissPicker(DatePickerDismissReason::kOwnerUnavailable);
  }
}

void DockedDatePickerField::onTextChanged() {
  if (changing_text_) return;
  CivilDay parsed;
  if (text().empty() || parseText(parsed))
    clearError();
  else
    setErrorText(datePickerStrings().invalid_date);
}

void DockedDatePickerField::onEditFinished(bool confirmed) {
  if (!confirmed || picker_) return;
  CivilDay parsed;
  if (!parseText(parsed)) {
    setErrorText(datePickerStrings().invalid_date);
    return;
  }
  date_ = parsed;
  clearError();
  onAccepted(parsed);
}

void DockedDatePickerField::completed(bool accepted,
                                      DatePickerDismissReason reason) {
  CivilDay value = picker_->value();
  month_ = picker_->displayedMonth();
  context().presentations().unobserve(*this);
  picker_.reset();
  if (accepted) {
    setDate(value);
    onAccepted(value);
  } else {
    onDismissed(reason);
  }
}
}  // namespace roo_windows::material3
