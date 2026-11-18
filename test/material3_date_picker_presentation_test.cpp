#include <functional>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"

namespace roo_windows::material3 {
namespace {
using internal::DatePickerMode;
using roo_time::CivilDay;

class Picker : public ModalDatePicker {
 public:
  using ModalDatePicker::ModalDatePicker;
  int accepted = 0;
  int dismissed = 0;
  bool disable_weekends = false;
  const roo_locale::CalendarLocale* calendar_locale = nullptr;
  DatePickerDismissReason reason = DatePickerDismissReason::kCancel;
  std::function<void()> completion;

 protected:
  const roo_locale::CalendarLocale& calendarLocale() const override {
    return calendar_locale != nullptr ? *calendar_locale
                                      : ModalDatePicker::calendarLocale();
  }

  bool isDateEnabled(CivilDay day) const override {
    return !disable_weekends || (day.dayOfWeek() != roo_time::kSaturday &&
                                 day.dayOfWeek() != roo_time::kSunday);
  }
  void onAccepted(CivilDay) override {
    ++accepted;
    if (completion) completion();
  }
  void onDismissed(DatePickerDismissReason value) override {
    ++dismissed;
    reason = value;
    if (completion) completion();
  }
};

class TestPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

class DatePickerPresentation : public testing::Test {
 protected:
  DatePickerPresentation()
      : device_(480, 640, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        content_(app_.context()),
        owner_(app_.addTaskFullScreen(content_)),
        picker_(app_.context()) {
    picker_.setValue(CivilDay::FromYmd(2024, 1, 31));
  }
  internal::DatePickerPanel& panel() {
    return *static_cast<internal::DatePickerPanel*>(owner_.focus().scopeRoot());
  }
  void open() {
    app_.refresh();
    ASSERT_EQ(PresentationStartResult::kStarted, picker_.open(owner_));
    app_.refresh();
  }
  roo::byte raster_[480 * 640 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment environment_;
  Application app_;
  TestPanel content_;
  Task& owner_;
  Picker picker_;
};

// Verifies calendar names, abbreviations, and week starts are borrowed from
// roo_locale rather than copied into picker label tables.
TEST_F(DatePickerPresentation, CalendarLocaleSuppliesNamesAndWeekStart) {
  for (const roo_locale::CalendarLocale* locale :
       {&roo_locale::PolishPoland(), &roo_locale::FrenchFrance(),
        &roo_locale::GermanGermany(), &roo_locale::EnglishUnitedKingdom()}) {
    picker_.calendar_locale = locale;
    open();
    EXPECT_EQ(locale, &panel().session().locale());
    EXPECT_EQ(static_cast<roo_time::DayOfWeek>(locale->first_weekday),
              panel().session().firstWeekday());
    auto& header = *static_cast<internal::DatePickerHeader*>(
        static_cast<Widget&>(panel()).focusChildAt(0));
    for (int i = 0; i < 12; ++i)
      EXPECT_EQ(locale->month_standalone_wide[i], header.monthLabel(i));
    header.measure(WidthSpec::Exactly(Scaled(208)),
                   HeightSpec::Exactly(Scaled(104)));
    header.layout(Rect(0, 0, Scaled(208) - 1, Scaled(104) - 1));
    for (int i = 0; i < 12; ++i)
      EXPECT_EQ(locale->month_standalone_abbreviated[i], header.monthLabel(i));
    picker_.dismiss();
    app_.refresh();
  }
}

// Verifies packed configuration still preserves invalid entry modes so
// admission can reject them instead of silently treating them as calendar mode.
TEST_F(DatePickerPresentation, InvalidEntryModeRejectsAdmission) {
  picker_.setEntryMode(static_cast<DatePickerEntryMode>(2));
  EXPECT_EQ(static_cast<DatePickerEntryMode>(2), picker_.entryMode());
  app_.refresh();
  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable, picker_.open(owner_));
  EXPECT_FALSE(picker_.isOpen());
}

// Verifies initial calendar focus and focus transfer to newly laid-out input.
TEST_F(DatePickerPresentation, ActiveBodyReceivesFocus) {
  open();
  ASSERT_NE(nullptr, owner_.focus().focused());
  EXPECT_NE(&content_, owner_.focus().focused());
  panel().setMode(DatePickerMode::kInput);
  app_.refresh();
  EXPECT_EQ(panel().input(), owner_.focus().focused());
  panel().setMode(DatePickerMode::kDays);
  app_.refresh();
  EXPECT_NE(nullptr, owner_.focus().focused());
  EXPECT_NE(panel().input(), owner_.focus().focused());
}

// Verifies OK can be pressed immediately after a day, safely retiring the
// day ripple before the terminal action destroys the session.
TEST_F(DatePickerPresentation, ConfirmationCanInterruptDayFeedback) {
  open();
  auto& body =
      *static_cast<internal::DatePickerBody*>(owner_.focus().focused());
  const Rect day = body.cellBounds(14);  // January 14, 2024.
  body.onSingleTapUp(day.xMin() + 4, day.yMin() + 4);
  EXPECT_TRUE(body.isClicking());
  Widget* confirm = static_cast<Widget&>(panel()).focusChildAt(3);
  ASSERT_NE(nullptr, confirm);
  confirm->onSingleTapUp(10, 10);
  EXPECT_FALSE(body.isClicking());
  system_time_lag_ns(350000000);
  app_.refresh();
  EXPECT_EQ(1, picker_.accepted);
  EXPECT_EQ(CivilDay::FromYmd(2024, 1, 14), picker_.value());
  EXPECT_FALSE(picker_.isOpen());
}

// Verifies a draft commits once, only on confirmation and after detachment.
TEST_F(DatePickerPresentation, DraftAndAcceptance) {
  open();
  CivilDay chosen = CivilDay::FromYmd(2024, 1, 15);
  panel().selectDate(chosen);
  EXPECT_EQ(CivilDay::FromYmd(2024, 1, 31), picker_.value());
  picker_.completion = [&] {
    EXPECT_FALSE(picker_.isOpen());
    EXPECT_FALSE(
        app_.root().transient_presentation_slot().hasActivePresentation());
  };
  panel().session().accept();
  EXPECT_EQ(chosen, picker_.value());
  EXPECT_EQ(1, picker_.accepted);
  EXPECT_EQ(0, picker_.dismissed);
}

// Verifies bounds and application filters cannot be bypassed by selection.
TEST_F(DatePickerPresentation, DisabledDatesAndBounds) {
  picker_.setBounds(
      {CivilDay::FromYmd(2024, 1, 10), CivilDay::FromYmd(2024, 1, 20)});
  picker_.disable_weekends = true;
  open();
  EXPECT_FALSE(panel().session().draft.isValid());
  panel().selectDate(CivilDay::FromYmd(2024, 1, 21));
  panel().selectDate(CivilDay::FromYmd(2024, 1, 13));
  panel().session().accept();
  EXPECT_TRUE(picker_.isOpen());
  panel().selectDate(CivilDay::FromYmd(2024, 1, 12));
  panel().session().accept();
  EXPECT_EQ(CivilDay::FromYmd(2024, 1, 12), picker_.value());
}

// Verifies month jumps preserve a possible day and clear impossible dates.
TEST_F(DatePickerPresentation, MonthAndYearBodyModes) {
  open();
  panel().setMode(DatePickerMode::kYears);
  EXPECT_EQ(DatePickerMode::kYears, panel().mode());
  EXPECT_EQ(BackResult::kHandled, owner_.requestBack(BackSource::kEscapeKey));
  EXPECT_EQ(DatePickerMode::kDays, panel().mode());
  panel().selectMonth(2024, 2);
  EXPECT_FALSE(panel().session().draft.isValid());
  panel().selectDate(CivilDay::FromYmd(2024, 2, 29));
  panel().selectMonth(2025, 2);
  EXPECT_FALSE(panel().session().draft.isValid());
  picker_.dismiss();
  EXPECT_EQ(CivilDay::FromYmd(2024, 1, 31), picker_.value());
}

// Verifies invalid/incomplete input vetoes confirmation and calendar switching.
TEST_F(DatePickerPresentation, InputValidationAndModeSwitch) {
  picker_.setEntryMode(DatePickerEntryMode::kInput);
  open();
  ASSERT_NE(nullptr, panel().input());
  EXPECT_FALSE(panel().input()->hasError());
  panel().input()->setText("02/");
  EXPECT_TRUE(panel().input()->hasError());
  panel().session().accept();
  EXPECT_TRUE(picker_.isOpen());
  panel().setMode(DatePickerMode::kDays);
  EXPECT_EQ(DatePickerMode::kInput, panel().mode());
  char text[64];
  DefaultDateTextCodec().format(CivilDay::FromYmd(2024, 2, 29), text,
                                sizeof(text));
  panel().input()->setText(text);
  EXPECT_FALSE(panel().input()->hasError());
  panel().setMode(DatePickerMode::kDays);
  EXPECT_EQ(DatePickerMode::kDays, panel().mode());
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 1), picker_.displayedMonth());
  panel().session().accept();
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 29), picker_.value());
}

// Verifies occupied-host rejection preserves the first picker and its draft.
TEST_F(DatePickerPresentation, AdmissionFailureIsAtomic) {
  open();
  Picker other(app_.context());
  EXPECT_EQ(PresentationStartResult::kHostBusy, other.open(owner_));
  EXPECT_FALSE(other.isOpen());
  EXPECT_TRUE(picker_.isOpen());
  EXPECT_EQ(0, other.dismissed);
  EXPECT_EQ(PresentationStartResult::kHostBusy, picker_.open(owner_));
}

// Verifies completion can reopen using the same presenter after host cleanup.
TEST_F(DatePickerPresentation, CompletionCanReopen) {
  open();
  picker_.completion = [&] {
    EXPECT_EQ(PresentationStartResult::kStarted, picker_.open(owner_));
  };
  picker_.dismiss();
  EXPECT_TRUE(picker_.isOpen());
  picker_.completion = {};
}

// Verifies destruction cancels the registration and restores the owner's scope.
TEST_F(DatePickerPresentation, PresenterDestructionIsSilent) {
  auto other = std::make_unique<Picker>(app_.context());
  app_.refresh();
  ASSERT_EQ(PresentationStartResult::kStarted, other->open(owner_));
  bool called = false;
  other->completion = [&] { called = true; };
  other.reset();
  EXPECT_FALSE(called);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
  EXPECT_NE(nullptr, owner_.focus().scopeRoot());
}

// Verifies each Back request closes one internal mode or the picker itself.
TEST_F(DatePickerPresentation, BackCancelsWithoutCommit) {
  open();
  panel().selectDate(CivilDay::FromYmd(2024, 1, 15));
  owner_.requestBack(BackSource::kEscapeKey);
  EXPECT_FALSE(picker_.isOpen());
  EXPECT_EQ(DatePickerDismissReason::kBack, picker_.reason);
  EXPECT_EQ(CivilDay::FromYmd(2024, 1, 31), picker_.value());
}

// Verifies a real header tap switches body mode and keyboard traversal stays
// scoped.
TEST_F(DatePickerPresentation, TouchAndKeyboardRoutes) {
  open();
  Widget* header = static_cast<Widget&>(panel()).focusChildAt(0);
  ASSERT_NE(nullptr, header);
  header->onSingleTapUp(Scaled(140), Scaled(90));
  EXPECT_EQ(DatePickerMode::kMonths, panel().mode());
  owner_.focus().moveFocus(panel(), false);
  EXPECT_NE(nullptr, owner_.focus().focused());
  owner_.focus().focused()->onKeyEvent(
      {KeyPhase::kDown, KeyCode::kRight, 0, 0});
  owner_.focus().focused()->onKeyEvent(
      {KeyPhase::kDown, KeyCode::kEnter, 0, 0});
  EXPECT_TRUE(picker_.isOpen());
}

// Verifies owner navigation ends a session before its source content detaches.
TEST_F(DatePickerPresentation, OwnerNavigationClosesPicker) {
  open();
  owner_.navigation().clear();
  EXPECT_FALSE(picker_.isOpen());
  EXPECT_EQ(DatePickerDismissReason::kOwnerUnavailable, picker_.reason);
}

// Verifies year paging and civil endpoints stay within valid bounded geometry.
TEST_F(DatePickerPresentation, YearPagesReachCivilLimits) {
  picker_.setDisplayedMonth(CivilDay::FromYmd(9999, 12, 1));
  open();
  panel().setMode(DatePickerMode::kYears);
  app_.refresh();
  EXPECT_LE(panel().firstYear(), 9999);
  panel().activateHeader(3);
  app_.refresh();
  EXPECT_EQ(9961, panel().firstYear());
  panel().selectMonth(1, 1);
  panel().activateHeader(0);
  EXPECT_EQ(CivilDay::FromYmd(1, 1, 1), picker_.displayedMonth());
}

// Verifies an outside hit reaches the host barrier and dismisses after
// dispatch.
TEST_F(DatePickerPresentation, OutsideTapCancelsDraft) {
  open();
  std::vector<Widget*> path;
  ASSERT_TRUE(app_.root().fillTouchTargetPath(1, 1, path));
  ASSERT_FALSE(path.empty());
  path.back()->onSingleTapUp(1, 1);
  app_.start();
  scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 4);
  EXPECT_FALSE(picker_.isOpen());
  EXPECT_EQ(DatePickerDismissReason::kOutsideTap, picker_.reason);
  EXPECT_EQ(1, picker_.dismissed);
}

// Verifies terminal delivery can delete the presenter and its active session.
TEST_F(DatePickerPresentation, CompletionCanDestroyPresenter) {
  class DeletingPicker : public ModalDatePicker {
   public:
    DeletingPicker(ApplicationContext& context,
                   std::unique_ptr<DeletingPicker>& slot)
        : ModalDatePicker(context), slot_(slot) {}

   protected:
    void onAccepted(CivilDay) override { slot_.reset(); }

   private:
    std::unique_ptr<DeletingPicker>& slot_;
  };
  app_.refresh();
  std::unique_ptr<DeletingPicker> picker;
  picker.reset(new DeletingPicker(app_.context(), picker));
  picker->setValue(CivilDay::FromYmd(2024, 2, 29));
  ASSERT_EQ(PresentationStartResult::kStarted, picker->open(owner_));
  panel().session().accept();
  EXPECT_EQ(nullptr, picker);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies rejected configuration leaves focus and the shared slot intact.
TEST_F(DatePickerPresentation, ReversedBoundsRejectAdmission) {
  app_.refresh();
  picker_.setBounds(
      {CivilDay::FromYmd(2024, 2, 1), CivilDay::FromYmd(2024, 1, 1)});
  Widget* scope = owner_.focus().scopeRoot();
  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable, picker_.open(owner_));
  EXPECT_FALSE(picker_.isOpen());
  EXPECT_EQ(scope, owner_.focus().scopeRoot());
}

}  // namespace
}  // namespace roo_windows::material3
