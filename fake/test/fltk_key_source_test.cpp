#include "gtest/gtest.h"

#include "roo_windows/fake/fltk_key_source.h"

namespace roo_windows::fake {
namespace {

// Verifies that normalized host events retain FIFO order across bounded reads.
TEST(FltkKeySource, DrainsQueuedEventsInOrder) {
  FltkKeySource source;
  EXPECT_TRUE(source.enqueue({KeyPhase::kDown, KeyCode::kTab, 0, 0}));
  EXPECT_TRUE(source.enqueue(
      {KeyPhase::kDown, KeyCode::kCharacter, kKeyModifierShift, 'A'}));
  EXPECT_TRUE(source.enqueue({KeyPhase::kUp, KeyCode::kTab, 0, 0}));

  KeyEvent events[2];
  EXPECT_EQ(2, source.drain(events, 2));
  EXPECT_EQ(KeyCode::kTab, events[0].code);
  EXPECT_EQ(KeyCode::kCharacter, events[1].code);
  EXPECT_EQ(static_cast<uint32_t>('A'), events[1].rune);
  EXPECT_EQ(kKeyModifierShift, events[1].modifiers);

  EXPECT_EQ(1, source.drain(events, 2));
  EXPECT_EQ(KeyPhase::kUp, events[0].phase);
  EXPECT_EQ(KeyCode::kTab, events[0].code);
}

// Verifies that a full queue drops only the newest host event without blocking.
TEST(FltkKeySource, RejectsEventsAfterQueueCapacity) {
  FltkKeySource source;
  KeyEvent event{KeyPhase::kDown, KeyCode::kEnter, 0, 0};
  for (int i = 0; i < FltkKeySource::kQueueCapacity - 1; ++i) {
    EXPECT_TRUE(source.enqueue(event));
  }
  EXPECT_FALSE(source.enqueue(event));

  KeyEvent drained[FltkKeySource::kQueueCapacity];
  EXPECT_EQ(FltkKeySource::kQueueCapacity - 1,
            source.drain(drained, FltkKeySource::kQueueCapacity));
}

}  // namespace
}  // namespace roo_windows::fake
