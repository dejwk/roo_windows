#pragma once

#include <stdint.h>

#include <vector>

#include "roo_display/core/utf8.h"
#include "roo_display/font/font.h"
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
  void appendLine(roo_display::StringView line);

  void setFont(const roo_display::Font* font);

  roo_windows::Dimensions getSuggestedMinimumDimensions() const override;

  roo_windows::PreferredSize getPreferredSize() const override;

  roo_windows::Dimensions onMeasure(roo_windows::WidthSpec width,
                                    roo_windows::HeightSpec height) override;

  void paint(const roo_windows::Canvas& s) const override;

 private:
  int16_t maxLineWidth() const;
  int line_pos(int idx) const;
  void drop_first_line();

  size_t buffer_size_;
  std::unique_ptr<uint8_t[]> buffer_;
  const roo_display::Font* font_;

  uint8_t* cursor_;
  std::vector<roo_display::StringView> lines_;
  std::vector<uint16_t> line_widths_;  // In pixels.
  mutable int16_t max_line_width_;  // negative means need to be recalculated.
  int line_start_;
  int line_count_;
};

}  // namespace log
}  // namespace roo_windows
