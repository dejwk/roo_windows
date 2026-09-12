#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

using test_support::RooWindowsRenderTest;

class ActivityObserver : public BasicWidget {
 public:
  explicit ActivityObserver(ApplicationContext& context)
      : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }

  std::vector<bool> events;
  TransientPresentationSlot* unsubscribe_from = nullptr;

 protected:
  void onTransientActivityChanged(bool active) override {
    events.push_back(active);
    if (unsubscribe_from != nullptr) {
      TransientPresentationSlot* slot = unsubscribe_from;
      unsubscribe_from = nullptr;
      slot->unobserveActivity(*this);
    }
  }
};

class Registration : public TransientPresentationRegistration {
 protected:
  void detachPresentation(PresentationFinishReason reason) override {
    (void)reason;
  }
};

class TransientActivityObserverTest : public RooWindowsRenderTest {
 protected:
  struct AddedObserver {
    ActivityObserver* observer;
    Task* task;
  };

  ~TransientActivityObserverTest() override {
    for (Task* task : tasks_) task->navigation().clear();
  }

  AddedObserver AddObserver() {
    auto observer = std::make_unique<ActivityObserver>(context());
    ActivityObserver* observer_ptr = observer.get();
    observers_.push_back(std::move(observer));
    Task& task = app_.addTask(*observer_ptr, roo_display::Box(0, 0, 15, 15));
    tasks_.push_back(&task);
    return AddedObserver{observer_ptr, &task};
  }

  TransientPresentationSlot& slot() {
    return app_.root().transient_presentation_slot();
  }

  std::vector<std::unique_ptr<ActivityObserver>> observers_;
  std::vector<Task*> tasks_;
};

TEST_F(TransientActivityObserverTest, DeliversOpenAndCloseChanges) {
  AddedObserver added = AddObserver();
  ASSERT_TRUE(slot().observeActivity(*added.observer));
  EXPECT_TRUE(added.observer->events.empty());

  Registration presentation;
  ASSERT_EQ(PresentationStartResult::kStarted, slot().show(presentation));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(std::vector<bool>({true}), added.observer->events);

  presentation.finish(PresentationFinishReason::kCancel);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(std::vector<bool>({true, false}), added.observer->events);
}

TEST_F(TransientActivityObserverTest, ReplacementCoalescesAsStillActive) {
  AddedObserver added = AddObserver();
  ASSERT_TRUE(slot().observeActivity(*added.observer));
  Registration first;
  Registration second;
  ASSERT_EQ(PresentationStartResult::kStarted, slot().show(first));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(1u, added.observer->events.size());

  ASSERT_EQ(PresentationStartResult::kStarted, slot().replace(second));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(1u, added.observer->events.size());
  EXPECT_TRUE(added.observer->events.back());

  second.finish(PresentationFinishReason::kCancel);
  ASSERT_TRUE(refresh());
  ASSERT_EQ(2u, added.observer->events.size());
  EXPECT_FALSE(added.observer->events.back());
}

TEST_F(TransientActivityObserverTest, UnsubscribeDuringDeliveryIsSafe) {
  AddedObserver added = AddObserver();
  ASSERT_TRUE(slot().observeActivity(*added.observer));
  added.observer->unsubscribe_from = &slot();
  Registration presentation;
  ASSERT_EQ(PresentationStartResult::kStarted, slot().show(presentation));
  ASSERT_TRUE(refresh());
  ASSERT_EQ(1u, added.observer->events.size());

  presentation.finish(PresentationFinishReason::kCancel);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(1u, added.observer->events.size());
}

TEST_F(TransientActivityObserverTest, DetachRemovesBorrowedObserver) {
  AddedObserver added = AddObserver();
  ASSERT_TRUE(slot().observeActivity(*added.observer));
  added.task->navigation().clear();

  Registration presentation;
  ASSERT_EQ(PresentationStartResult::kStarted, slot().show(presentation));
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(added.observer->events.empty());
}

TEST_F(TransientActivityObserverTest, DestroyAfterDetachLeavesNoBorrowedPointer) {
  AddedObserver added = AddObserver();
  ASSERT_TRUE(slot().observeActivity(*added.observer));
  added.task->navigation().clear();
  observers_.clear();

  Registration presentation;
  ASSERT_EQ(PresentationStartResult::kStarted, slot().show(presentation));
  EXPECT_TRUE(refresh());
  presentation.finish(PresentationFinishReason::kCancel);
  EXPECT_TRUE(refresh());
}

TEST(TransientActivityObserver, ControlStorageStaysWithinTargetBudget) {
  constexpr size_t kHostPointerAdjustment =
      sizeof(void*) > 4 ? 5 * (sizeof(void*) - 4) : 0;
  EXPECT_LE(sizeof(TransientPresentationSlot), 96U + kHostPointerAdjustment);
}

}  // namespace
}  // namespace roo_windows
