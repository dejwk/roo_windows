#include <memory>

#include "gtest/gtest.h"
#include "roo_windows/core/transient_presentation.h"

namespace roo_windows {
namespace {

class TestPresenter final : public TransientPresentationRegistration {
 public:
  void reopen(TransientPresentationSlot& slot,
              TransientPresentationRegistration& registration) {
    reopen_slot_ = &slot;
    replacement_ = &registration;
  }

  void observeCompletion(int& completion_count) {
    completion_observer_ = &completion_count;
  }

  int detach_count = 0;
  int completion_count = 0;
  PresentationFinishReason detach_reason = PresentationFinishReason::kAction;
  PresentationFinishReason completion_reason =
      PresentationFinishReason::kAction;
  PresentationState state_seen_by_completion = PresentationState::kVisible;
  bool slot_empty_seen_by_completion = false;

 protected:
  void detachPresentation(PresentationFinishReason reason) override {
    ++detach_count;
    detach_reason = reason;
  }

  void onFinished(PresentationFinishReason reason) override {
    ++completion_count;
    if (completion_observer_ != nullptr) ++*completion_observer_;
    completion_reason = reason;
    state_seen_by_completion = state();
    slot_empty_seen_by_completion =
        reopen_slot_ != nullptr && !reopen_slot_->hasActivePresentation();
    if (reopen_slot_ != nullptr && replacement_ != nullptr) {
      reopen_slot_->show(*replacement_);
    }
  }

 private:
  TransientPresentationSlot* reopen_slot_ = nullptr;
  TransientPresentationRegistration* replacement_ = nullptr;
  int* completion_observer_ = nullptr;
};

// Verifies terminal delivery is detached, idle, and exactly once.
TEST(TransientPresentationLifetime, ExplicitFinishIsIdempotent) {
  TransientPresentationSlot slot;
  TestPresenter presenter;

  EXPECT_EQ(PresentationStartResult::kStarted, slot.show(presenter));
  presenter.finish(PresentationFinishReason::kAction);
  presenter.finish(PresentationFinishReason::kCancel);

  EXPECT_FALSE(slot.hasActivePresentation());
  EXPECT_EQ(PresentationState::kIdle, presenter.state());
  EXPECT_EQ(1, presenter.detach_count);
  EXPECT_EQ(PresentationFinishReason::kAction, presenter.detach_reason);
  EXPECT_EQ(1, presenter.completion_count);
  EXPECT_EQ(PresentationFinishReason::kAction, presenter.completion_reason);
  EXPECT_EQ(PresentationState::kIdle, presenter.state_seen_by_completion);
}

// Verifies an occupied slot rejects an unrelated root presentation.
TEST(TransientPresentationLifetime, ShowRejectsOccupiedSlot) {
  TransientPresentationSlot slot;
  TestPresenter first;
  TestPresenter second;

  EXPECT_EQ(PresentationStartResult::kStarted, slot.show(first));
  EXPECT_EQ(PresentationStartResult::kHostBusy, slot.show(second));
  EXPECT_TRUE(first.isActive());
  EXPECT_FALSE(second.isActive());
}

// Verifies replacement preserves a presentation opened by completion code.
TEST(TransientPresentationLifetime, ReplaceDetectsReentrantCompletion) {
  TransientPresentationSlot slot;
  TestPresenter current;
  TestPresenter reopened;
  TestPresenter requested;
  current.reopen(slot, reopened);

  EXPECT_EQ(PresentationStartResult::kStarted, slot.show(current));
  EXPECT_EQ(PresentationStartResult::kReentrantReplacement,
            slot.replace(requested));

  EXPECT_EQ(1, current.detach_count);
  EXPECT_EQ(PresentationFinishReason::kReplacement, current.detach_reason);
  EXPECT_EQ(PresentationState::kIdle, current.state_seen_by_completion);
  EXPECT_TRUE(current.slot_empty_seen_by_completion);
  EXPECT_TRUE(reopened.isActive());
  EXPECT_FALSE(requested.isActive());
}

// Verifies destruction removes a visible presenter without completion.
TEST(TransientPresentationLifetime, PresenterDestructionVacatesSlot) {
  TransientPresentationSlot slot;
  int completion_count = 0;
  {
    TestPresenter presenter;
    presenter.observeCompletion(completion_count);
    EXPECT_EQ(PresentationStartResult::kStarted, slot.show(presenter));
  }
  EXPECT_FALSE(slot.hasActivePresentation());
  EXPECT_EQ(0, completion_count);
}

// Verifies eligible Back finishes the registration and ineligible Back passes.
TEST(TransientPresentationLifetime, BackRespectsRegistrationPolicy) {
  TransientPresentationSlot slot;
  TestPresenter presenter;

  EXPECT_EQ(PresentationStartResult::kStarted,
            slot.show(presenter, TransientPresentationPolicy(true)));
  EXPECT_EQ(BackResult::kUnhandled, slot.requestBack(BackSource::kEscapeKey));
  EXPECT_TRUE(presenter.isActive());
  EXPECT_EQ(BackResult::kHandled, slot.requestBack(BackSource::kBackKey));
  EXPECT_EQ(PresentationFinishReason::kBack, presenter.completion_reason);
}

// Verifies host destruction performs terminal teardown for a surviving owner.
TEST(TransientPresentationLifetime, HostDestructionFinishesPresenter) {
  TestPresenter presenter;
  {
    std::unique_ptr<TransientPresentationSlot> slot(
        new TransientPresentationSlot());
    EXPECT_EQ(PresentationStartResult::kStarted, slot->show(presenter));
  }

  EXPECT_EQ(PresentationState::kIdle, presenter.state());
  EXPECT_EQ(1, presenter.detach_count);
  EXPECT_EQ(PresentationFinishReason::kHostDestroyed, presenter.detach_reason);
  EXPECT_EQ(1, presenter.completion_count);
  EXPECT_EQ(PresentationFinishReason::kHostDestroyed,
            presenter.completion_reason);
}

}  // namespace
}  // namespace roo_windows
