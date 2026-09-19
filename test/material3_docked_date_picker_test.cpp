#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows/material3/date_picker/docked_date_picker_field.h"

namespace roo_windows::material3 {
namespace {
using roo_time::CivilDay;
class TestPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};
class Field : public DockedDatePickerField {
 public:
  using DockedDatePickerField::DockedDatePickerField;
  int accepted = 0;
  int dismissed = 0;

 protected:
  void onAccepted(CivilDay) override { ++accepted; }
  void onDismissed(DatePickerDismissReason) override { ++dismissed; }
};
class DockedDatePickerTest : public testing::Test {
 protected:
  DockedDatePickerTest()
      : device_(480, 640, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        content_(app_.context()),
        owner_(app_.addTaskFullScreen(content_)) {
    auto field = std::make_unique<Field>(app_.context(), "Maintenance date");
    field_ = field.get();
    field_->setDate(CivilDay::FromYmd(2024, 2, 29));
    content_.add(WidgetRef(std::move(field)), Rect(32, 16, 367, 71));
  }
  internal::DatePickerPanel& panel() {
    return *static_cast<internal::DatePickerPanel*>(owner_.focus().scopeRoot());
  }
  roo::byte raster_[480 * 640 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
  TestPanel content_;
  Task& owner_;
  Field* field_;
};

// Verifies focus-triggered admission is deferred and restoration does not
// reopen.
TEST_F(DockedDatePickerTest, FocusTakeoverAndRestoration) {
  ASSERT_TRUE(app_.refresh());
  EXPECT_TRUE(field_->requestFocus());
  EXPECT_EQ(field_, owner_.focus().focused());
  scheduler_.executeEligibleTasks(10);
  ASSERT_TRUE(field_->isPickerOpen());
  ASSERT_NE(nullptr, owner_.focus().focused());
  EXPECT_NE(field_, owner_.focus().focused());
  EXPECT_FALSE(field_->isEdited());
  field_->dismissPicker();
  EXPECT_FALSE(field_->isPickerOpen());
  EXPECT_EQ(field_, owner_.focus().focused());
  scheduler_.executeEligibleTasks(10);
  EXPECT_FALSE(field_->isPickerOpen());
}

// Verifies calendar edits remain drafts and cancellation preserves typed text.
TEST_F(DockedDatePickerTest, CancelPreservesUnconfirmedText) {
  ASSERT_TRUE(app_.refresh());
  field_->setText("incomplete");
  ASSERT_EQ(PresentationStartResult::kStarted, field_->openPicker());
  panel().selectDate(CivilDay::FromYmd(2024, 2, 15));
  field_->dismissPicker();
  EXPECT_EQ("incomplete", field_->text());
  EXPECT_EQ(CivilDay::FromYmd(2024, 2, 29), field_->date());
  EXPECT_EQ(1, field_->dismissed);
}

// Verifies acceptance updates the typed value only after the calendar detaches.
TEST_F(DockedDatePickerTest, ConfirmUpdatesField) {
  ASSERT_TRUE(app_.refresh());
  ASSERT_EQ(PresentationStartResult::kStarted, field_->openPicker());
  CivilDay selected = CivilDay::FromYmd(2024, 2, 15);
  panel().selectDate(selected);
  panel().session().accept();
  EXPECT_FALSE(field_->isPickerOpen());
  EXPECT_EQ(selected, field_->date());
  CivilDay parsed;
  EXPECT_EQ(roo_time::TextStatus::kOk,
            DefaultDateTextCodec().parse(field_->text(), parsed).status);
  EXPECT_EQ(selected, parsed);
  EXPECT_EQ(1, field_->accepted);
}

// Verifies pending focus admission is canceled when the field is destroyed.
TEST_F(DockedDatePickerTest, PendingAdmissionDoesNotOutliveField) {
  ASSERT_TRUE(app_.refresh());
  field_->requestFocus();
  content_.removeLast();
  scheduler_.executeEligibleTasks(10);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies destruction of an open field silently detaches its calendar.
TEST_F(DockedDatePickerTest, ActiveFieldDestruction) {
  ASSERT_TRUE(app_.refresh());
  field_->onClicked();
  ASSERT_TRUE(field_->isPickerOpen());
  content_.removeLast();
  scheduler_.executeEligibleTasks(10);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies a calendar ends editing and rejects further underlying field input.
TEST_F(DockedDatePickerTest, EditorIsInactiveWhileCalendarIsOpen) {
  ASSERT_TRUE(app_.refresh());
  field_->requestFocus();
  scheduler_.executeEligibleTasks(10);
  field_->dismissPicker();
  field_->edit();
  EXPECT_TRUE(field_->isEdited());
  ASSERT_EQ(PresentationStartResult::kStarted, field_->openPicker());
  EXPECT_FALSE(field_->isEdited());
  field_->edit();
  EXPECT_FALSE(field_->isEdited());
}
}  // namespace
}  // namespace roo_windows::material3
