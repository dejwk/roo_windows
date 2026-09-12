#pragma once
#include <string>

#include "roo_windows/core/widget.h"
namespace roo_windows {
namespace internal {
// Adapter to the task-local editor, including widget lifetime and animation.
class TextEditTarget {
 public:
  virtual ~TextEditTarget() = default;
  virtual Widget& editWidget() = 0;
  virtual const Widget& editWidget() const = 0;
  virtual std::string& textBuffer() = 0;
  virtual const std::string& textBuffer() const = 0;
  virtual const roo_display::Font& textFont() const = 0;
  virtual roo_display::Font::Options textFontOptions() const { return {}; }
  virtual bool obscureText() const = 0;
  virtual void notifyEditVisualChange() = 0;
  virtual void notifyTextChanged() {}
  virtual void onEditFinished(bool confirmed) = 0;
};
}  // namespace internal
}  // namespace roo_windows
