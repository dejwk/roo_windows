#pragma once

#include <stdint.h>

#include <vector>

#include "roo_backport.h"
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
  Log(const roo_windows::Environment& env, uint32_t buffer_size = 10 * 1024,
      size_t max_lines = 100);

  void clear();
  void appendLine(roo::string_view line);

  void setFont(const roo_display::Font* font);

  roo_windows::Dimensions getSuggestedMinimumDimensions() const override;

  roo_windows::PreferredSize getPreferredSize() const override;

  roo_windows::Dimensions onMeasure(roo_windows::WidthSpec width,
                                    roo_windows::HeightSpec height) override;

  void paint(const roo_windows::Canvas& s) const override;

  const roo_display::Font& font() const { return *font_; }

 private:
  int16_t maxLineWidth() const;
  int line_pos(int idx) const;
  void drop_first_line();

  size_t buffer_size_;
  std::unique_ptr<char[]> buffer_;
  const roo_display::Font* font_;

  char* cursor_;
  std::vector<roo_display::StringView> lines_;
  std::vector<uint16_t> line_widths_;  // In pixels.
  mutable int16_t max_line_width_;  // negative means need to be recalculated.
  int line_start_;
  int line_count_;
};

class ScrollableLog : public roo_windows::ScrollablePanel {
 public:
  enum AutoscrollMode {
    AUTOSCROLL_NONE,     // Doesn't auto-scroll.
    // AUTOSCROLL_DEFAULT,  // Auto-scrolls to bottom only if the content is at the
    //                      // bottom.
    AUTOSCROLL_ALWAYS    // Auto-scrolls to bottom whenever a new line gets
                         // appended.
  };

  ScrollableLog(const roo_windows::Environment& env,
                uint32_t buffer_size = 10 * 1024, size_t max_lines = 100);

  void clear();

  void appendLine(roo_display::StringView line);

  void setFont(const roo_display::Font* font);

  const roo_display::Font& font() const;

 private:
  void autoScroll();

  Log log_;
  AutoscrollMode autoscroll_mode_;
};

}  // namespace log
}  // namespace roo_windows
