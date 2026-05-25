#pragma once

#include <stdint.h>

#include <vector>

#include "roo_backport.h"
#include "roo_backport/string_view.h"
#include "roo_display/core/utf8.h"
#include "roo_display/font/font.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {
namespace log {

// For capturing textual logs, consisting of lines that get continuously
// appended.
class Log : public roo_windows::BasicWidget {
 public:
  // Create the log widget with the specified buffer size and max lines. The
  // total memory footprint of the log will be approximately equal to the buffer
  // size. The line count is a hard limit. If appending a new line causes either
  // the buffer size or the line count limit to be exceeded, the lines get
  // dropped from the beginning of the log until sufficient space is cleared.
  //
  // The implementation uses a circular buffer with some auxiliary circular
  // vectors for caching string offsets and line lengths.
  // Create the log widget with the specified buffer size and max lines. The
  // total memory footprint of the log will be approximately equal to the buffer
  // size. The line count is a hard limit. If appending a new line causes either
  // the buffer size or the line count limit to be exceeded, the lines get
  // dropped from the beginning of the log until sufficient space is cleared.
  //
  // The implementation uses a circular buffer with some auxiliary circular
  // vectors for caching string offsets and line lengths.
    Log(roo_windows::ApplicationContext& context,
      uint32_t buffer_size = 10 * 1024,
      size_t max_lines = 100);

  /// Discards all stored lines, leaving the log empty.
  void clear();
  /// Appends a new line, evicting oldest lines if the buffer or line-count
  /// caps would be exceeded.
  void appendLine(roo::string_view line);

  /// Sets the font used to render the log; invalidates cached line widths.
  void setFont(const roo_display::Font* font);

  /// Returns a minimum size large enough to host a single character.
  roo_windows::Dimensions getSuggestedMinimumDimensions() const override;

  /// Advertises wrap-content sized for the widest measured line and the
  /// total line count.
  roo_windows::PreferredSize getPreferredSize() const override;

  /// Reports a content size derived from cached per-line widths and the line
  /// height of the current font.
  roo_windows::Dimensions onMeasure(roo_windows::WidthSpec width,
                                    roo_windows::HeightSpec height) override;

  /// Draws each stored line top-to-bottom in the current font.
  void paint(roo_windows::PaintContext& ctx) const override;

  const roo_display::Font& font() const { return *font_; }

 private:
  int16_t maxLineWidth() const;
  int line_pos(int idx) const;
  void drop_first_line();

  size_t buffer_size_;
  std::unique_ptr<char[]> buffer_;
  const roo_display::Font* font_;

  char* cursor_;
  std::vector<roo::string_view> lines_;
  std::vector<uint16_t> line_widths_;  // In pixels.
  mutable int16_t max_line_width_;  // negative means need to be recalculated.
  int line_start_;
  int line_count_;
};

/// `Log` widget wrapped in a `ScrollablePanel`, with optional auto-scroll to
/// the bottom whenever a new line arrives.
///
/// Pick `AutoscrollMode::kAlways` for live log views; `kNone` lets the user
/// stay parked at an earlier position even as new lines are appended.
class ScrollableLog : public roo_windows::ScrollablePanel {
 public:
  enum class AutoscrollMode {
    kNone,   // Doesn't auto-scroll.
    kAlways  // Auto-scrolls to bottom whenever a new line gets
             // appended.
  };

  ScrollableLog(roo_windows::ApplicationContext& context,
                uint32_t buffer_size = 10 * 1024, size_t max_lines = 100);

  /// Clears all stored lines and resets the scroll position.
  void clear();

  /// Appends a new line and, when configured, scrolls to keep the bottom in
  /// view.
  void appendLine(roo::string_view line);

  /// Sets the font of the underlying `Log` widget.
  void setFont(const roo_display::Font* font);

  const roo_display::Font& font() const;

 private:
  void autoScroll();

  Log log_;
  AutoscrollMode autoscroll_mode_;
};

}  // namespace log
}  // namespace roo_windows
