#include <memory>

#include "gtest/gtest.h"
#include "roo_windows/core/task.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

using roo_display::Color;
using test_support::QuantizeToArgb4444;
using test_support::RooWindowsRenderTestSized;

class KeyboardPresentationPinTest : public RooWindowsRenderTestSized<160, 128> {
};

// Verifies a text-key preview is an active-only popup pin that escapes the
// keyboard task without creating a focus or presentation owner.
TEST_F(KeyboardPresentationPinTest,
       PreviewEscapesKeyboardTaskAndPreservesPopupFocus) {
  static_assert(sizeof(Keyboard) == sizeof(std::unique_ptr<Widget>) +
                                        sizeof(TextInputEmitter) +
                                        sizeof(Task*),
                "Keyboard must retain no persistent preview storage");

  Keyboard& keyboard = app_.keyboard();
  keyboard.show();
  ASSERT_TRUE(refresh());

  Panel& keyboard_contents = static_cast<Panel&>(keyboard.getContents());
  Panel& letter_page = static_cast<Panel&>(keyboard_contents.child_at(0));
  Widget& key = letter_page.child_at(0);
  Task* const owner = keyboard.getContents().getTask();
  ASSERT_NE(nullptr, owner);
  EXPECT_EQ(owner, letter_page.getTask());
  ASSERT_TRUE(key.requestFocus());
  EXPECT_EQ(&key, owner->focus().focused());

  XDim page_dx;
  YDim page_dy;
  letter_page.getAbsoluteOffset(page_dx, page_dy);
  const Rect& key_bounds = key.parent_bounds();
  const Rect preview_bounds(
      key_bounds.xMin() + page_dx, key_bounds.yMin() + page_dy - 50,
      key_bounds.xMax() + page_dx, key_bounds.yMax() + page_dy - 3);
  XDim keyboard_dx;
  YDim keyboard_dy;
  keyboard_contents.getAbsoluteOffset(keyboard_dx, keyboard_dy);
  ASSERT_LT(preview_bounds.yMin(), keyboard_dy);
  ASSERT_GE(preview_bounds.xMin(), 0);
  ASSERT_GE(preview_bounds.yMin(), 0);

  key.onShowPress(0, 0);
  EXPECT_TRUE(letter_page.hasPresentationPin());
  EXPECT_EQ(owner, keyboard.getContents().getTask());
  EXPECT_EQ(&key, owner->focus().focused());
  ASSERT_TRUE(refresh());

  Color overlay = roo_display::color::Black;
  overlay.set_a(
      context()
          .theme()
          .framework.interaction
          .resolve(FrameworkColorRole::kSurface, InteractionState::kPressed)
          .a());
  const Color expected = roo_display::AlphaBlend(
      context().keyboardColorTheme().normalButton, overlay);
  EXPECT_EQ(QuantizeToArgb4444(expected),
            pixelAt(preview_bounds.xMin(), preview_bounds.yMin() + 1));

  key.onCancel();
  EXPECT_FALSE(letter_page.hasPresentationPin());
  EXPECT_EQ(&key, owner->focus().focused());
}

}  // namespace
}  // namespace roo_windows
