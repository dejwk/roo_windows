#include "roo_windows/widgets/text_block.h"

#include <algorithm>
#include <limits>
#include <vector>

#include "roo_backport/string_view.h"
#include "roo_display/shape/basic.h"
#include "roo_io/text/unicode.h"

namespace roo_windows {

namespace {

inline bool IsWrapWhitespace(char c) { return c == ' ' || c == '\t'; }

int16_t MeasureText(const roo_display::Font& font, roo::string_view text) {
  if (text.empty()) return 0;
  auto m = font.getHorizontalStringMetrics(text.data(), text.size());
  return m.advance();
}

size_t TrimTrailingWhitespace(const std::string& text, size_t start,
                              size_t end) {
  while (end > start && IsWrapWhitespace(text[end - 1])) {
    --end;
  }
  return end;
}

TextAlign AlignFromHorizontal(roo_display::HAlign align) {
  switch (align.dst()) {
    case roo_display::Anchor::kMid:
      return TextAlign::kCenter;
    case roo_display::Anchor::kMax:
      return TextAlign::kEnd;
    case roo_display::Anchor::kMin:
    case roo_display::Anchor::kOrigin:
    default:
      return TextAlign::kStart;
  }
}

int16_t ResolveLineXOffset(int16_t line_width, int16_t layout_width,
                           TextAlign align) {
  if (layout_width <= line_width) return 0;
  switch (align) {
    case TextAlign::kCenter:
      return (layout_width - line_width) / 2;
    case TextAlign::kEnd:
      return layout_width - line_width;
    case TextAlign::kStart:
    case TextAlign::kJustify:
    default:
      return 0;
  }
}

uint16_t CountSpaces(roo::string_view text) {
  uint16_t count = 0;
  for (char c : text) {
    if (c == ' ') ++count;
  }
  return count;
}

void EllipsizeLine(TextBlock::LineLayout& line, const roo_display::Font& font,
                   int16_t max_width) {
  line.ellipsis_chars = 0;
  if (max_width <= 0) {
    line.text = roo::string_view();
    line.spaces = 0;
    line.width = 0;
    return;
  }

  static constexpr const char kDots[] = "...";
  int16_t dots_width[4] = {0, 0, 0, 0};
  for (int i = 1; i <= 3; ++i) {
    dots_width[i] = MeasureText(font, roo::string_view(kDots, i));
  }

  uint8_t dots_count = 3;
  while (dots_count > 0 && dots_width[dots_count] > max_width) {
    --dots_count;
  }
  if (dots_count == 0) {
    line.text = roo::string_view();
    line.spaces = 0;
    line.width = 0;
    return;
  }

  const char* data = line.text.data();
  std::vector<size_t> utf8_ends;
  utf8_ends.push_back(0);
  roo_io::Utf8Decoder boundary_decoder(line.text);
  char32_t decoded;
  while (boundary_decoder.next(decoded)) {
    size_t offset = static_cast<size_t>(
        reinterpret_cast<const char*>(boundary_decoder.data()) - data);
    utf8_ends.push_back(offset);
  }

  size_t boundary_idx = utf8_ends.empty() ? 0 : utf8_ends.size() - 1;
  size_t len = line.text.size();
  while (true) {
    roo::string_view candidate(data, len);
    int16_t candidate_width =
        MeasureText(font, candidate) + dots_width[dots_count];
    if (candidate_width <= max_width) {
      line.text = candidate;
      line.ellipsis_chars = dots_count;
      line.spaces = CountSpaces(candidate);
      line.width = candidate_width;
      return;
    }
    if (len == 0) {
      line.text = roo::string_view();
      line.ellipsis_chars = dots_count;
      line.spaces = 0;
      line.width = dots_width[dots_count];
      return;
    }
    if (boundary_idx == 0) {
      len = 0;
    } else {
      len = utf8_ends[--boundary_idx];
    }
  }
}

void AppendParagraphLines(const std::string& text, size_t start, size_t end,
                          const roo_display::Font& font, TextWrapMode wrap_mode,
                          int16_t width_limit,
                          std::vector<TextBlock::LineLayout>& out) {
  size_t before = out.size();
  if (start == end) {
    out.push_back(TextBlock::LineLayout{roo::string_view(), 0, 0, 0, true});
    return;
  }

  if (wrap_mode == TextWrapMode::kNoWrap || width_limit <= 0) {
    // Fast path: keep paragraph on one line when wrapping is disabled.
    roo::string_view line_text(text.data() + start, end - start);
    out.push_back(TextBlock::LineLayout{line_text, 0, CountSpaces(line_text),
                                        MeasureText(font, line_text), true});
    return;
  }

  size_t pos = start;
  while (pos < end) {
    while (pos < end && IsWrapWhitespace(text[pos])) ++pos;
    if (pos >= end) break;

    size_t line_start = pos;
    size_t idx = pos;
    size_t last_fit_end = line_start;
    size_t last_space_end = std::string::npos;
    // Scan UTF-8 code points and track the farthest fitting boundary.
    roo_io::Utf8Decoder decoder(text.data() + idx, end - idx);

    while (idx < end) {
      char32_t code_point;
      if (!decoder.next(code_point)) break;
      size_t cp_end = static_cast<size_t>(
          reinterpret_cast<const char*>(decoder.data()) - text.data());
      int16_t w = MeasureText(font, roo::string_view(text.data() + line_start,
                                                     cp_end - line_start));
      if (w <= width_limit) {
        last_fit_end = cp_end;
        if (code_point == U' ' || code_point == U'\t') {
          last_space_end = cp_end;
        }
        idx = cp_end;
        continue;
      }
      break;
    }

    size_t line_end = line_start;
    // Prefer breaking at trailing-trimmed whitespace, then hard-fit boundary,
    // and finally force one code point to guarantee progress.
    if (idx >= end) {
      line_end = TrimTrailingWhitespace(text, line_start, last_fit_end);
      if (line_end == line_start) line_end = last_fit_end;
    } else if (last_space_end != std::string::npos &&
               last_space_end > line_start) {
      line_end = TrimTrailingWhitespace(text, line_start, last_space_end);
      if (line_end == line_start) line_end = last_space_end;
    } else if (last_fit_end > line_start) {
      line_end = last_fit_end;
    } else {
      roo_io::Utf8Decoder one_cp(text.data() + line_start, end - line_start);
      char32_t ignored;
      if (one_cp.next(ignored)) {
        line_end = static_cast<size_t>(
            reinterpret_cast<const char*>(one_cp.data()) - text.data());
      } else {
        line_end = std::min(end, line_start + 1);
      }
    }

    roo::string_view line_text(text.data() + line_start, line_end - line_start);
    out.push_back(TextBlock::LineLayout{line_text, 0, CountSpaces(line_text),
                                        MeasureText(font, line_text), false});
    pos = line_end;
  }

  if (out.size() == before) {
    out.push_back(TextBlock::LineLayout{roo::string_view(), 0, 0, 0, true});
  }
  out.back().ends_paragraph = true;
}

class Interior : public roo_display::Drawable {
 public:
  Interior(roo_display::Box extents,
           const std::vector<TextBlock::LineLayout>& lines,
           const roo_display::Font& font, roo_display::Color color,
           TextAlign text_align)
      : extents_(extents),
        lines_(lines),
        font_(font),
        color_(color),
        text_align_(text_align) {}

  roo_display::Box extents() const override { return extents_; }

 private:
  void drawJustifiedLine(const roo_display::Surface& s,
                         const TextBlock::LineLayout& line, uint16_t spaces,
                         int16_t space_width) const {
    if (spaces <= 0 || line.width >= extents_.width()) {
      font_.drawHorizontalString(s, line.text.data(), line.text.size(), color_);
      return;
    }

    // Distribute remaining width across spaces as quotient + remainder.
    int16_t extra = extents_.width() - line.width;
    int16_t extra_per_space = extra / spaces;
    int16_t extra_remainder = extra % spaces;

    int16_t x = 0;
    size_t start = 0;
    while (start < line.text.size()) {
      size_t space = start;
      while (space < line.text.size() && line.text[space] != ' ') {
        ++space;
      }
      if (space > start) {
        roo_display::Surface part = s;
        part.set_dx(s.dx() + x);
        size_t len = space - start;
        font_.drawHorizontalString(part, line.text.data() + start, len, color_);
        x +=
            MeasureText(font_, roo::string_view(line.text.data() + start, len));
      }
      if (space >= line.text.size()) break;

      size_t run_end = space;
      while (run_end < line.text.size() && line.text[run_end] == ' ') {
        ++run_end;
      }
      // Draw each space so we can apply variable spacing deterministically.
      for (size_t i = space; i < run_end; ++i) {
        roo_display::Surface part = s;
        part.set_dx(s.dx() + x);
        font_.drawHorizontalString(part, " ", 1, color_);
        x += space_width + extra_per_space;
        if (extra_remainder > 0) {
          ++x;
          --extra_remainder;
        }
      }
      start = run_end;
    }
  }

  void drawTo(const roo_display::Surface& s) const override {
    if (lines_.empty()) return;

    static constexpr const char kDots[] = "...";
    // Precompute dot widths once per paint for ellipsis overlay.
    int16_t dots_width[4] = {0, 0, 0, 0};
    for (int i = 1; i <= 3; ++i) {
      dots_width[i] = MeasureText(font_, roo::string_view(kDots, i));
    }

    int16_t line_height = font_.metrics().maxHeight();
    int16_t row_y_min = -font_.metrics().glyphYMax();
    int16_t row_y_max = -font_.metrics().glyphYMin();
    int16_t space_width = MeasureText(font_, roo::string_view(" ", 1));
    int16_t clip_y_min = s.clip_box().yMin() - s.dy();
    int16_t clip_y_max = s.clip_box().yMax() - s.dy();

    for (size_t i = 0; i < lines_.size(); ++i) {
      int16_t y = static_cast<int16_t>(i) * line_height;
      int16_t y_end = y + line_height - 1;
      // Keep partially visible rows; only skip rows fully outside clip.
      if (y_end < clip_y_min || y > clip_y_max) continue;
      const auto& line = lines_[i];
      roo_display::Surface row_surface = s;
      row_surface.set_dy(s.dy() + y + font_.metrics().glyphYMax());
      // Clear the full glyph band for this row before drawing text.
      row_surface.drawObject(
          roo_display::FilledRect(0, row_y_min, extents_.width() - 1, row_y_max,
                                  row_surface.bgcolor()));

      // Only justify non-final lines of a paragraph.
      bool justify = text_align_ == TextAlign::kJustify &&
                     !line.ends_paragraph && line.width < extents_.width() &&
                     line.spaces > 0;
      if (justify) {
        drawJustifiedLine(row_surface, line, line.spaces, space_width);
      } else {
        int16_t line_x =
            ResolveLineXOffset(line.width, extents_.width(), text_align_);
        row_surface.set_dx(s.dx() + line_x);
        font_.drawHorizontalString(row_surface, line.text.data(),
                                   line.text.size(), color_);
        if (line.ellipsis_chars > 0) {
          roo_display::Surface dots_surface = row_surface;
          dots_surface.set_dx(row_surface.dx() + line.width -
                              dots_width[line.ellipsis_chars]);
          font_.drawHorizontalString(dots_surface, kDots, line.ellipsis_chars,
                                     color_);
        }
      }
    }
  }

  roo_display::Box extents_;
  const std::vector<TextBlock::LineLayout>& lines_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  TextAlign text_align_;
};

}  // namespace

TextBlock::TextBlock(const Environment& env, std::string value,
                     const roo_display::Font& font, roo_display::Color color,
                     roo_display::Alignment alignment)
    : BasicWidget(env),
      value_(),
      text_dims_(0, 0),
      layout_dims_(0, 0),
      layout_lines_(),
      layout_width_limit_(-1),
      layout_valid_(false),
      font_(font),
      color_(color),
      alignment_(alignment),
      wrap_mode_(TextWrapMode::kWordWrap),
      text_align_(AlignFromHorizontal(alignment.h())),
      max_lines_(0),
      ellipsize_(false) {
  setText(std::move(value));
}

void TextBlock::invalidateLayoutCache() {
  layout_valid_ = false;
  layout_width_limit_ = -1;
}

void TextBlock::recalculateNaturalDimensions() {
  ensureLayout(0);
  text_dims_ = layout_dims_;
}

void TextBlock::ensureLayout(XDim width_limit) const {
  if (width_limit < 0) width_limit = 0;
  // Layout is cached by width constraint because wrapping depends on it.
  if (layout_valid_ && layout_width_limit_ == width_limit) return;

  layout_lines_.clear();
  layout_dims_ = Dimensions(0, 0);
  layout_width_limit_ = width_limit;

  if (!value_.empty()) {
    size_t start = 0;
    while (start <= value_.size()) {
      size_t end = start;
      while (end < value_.size() && value_[end] != '\n') {
        ++end;
      }

      // Process one newline-delimited paragraph into one or more lines.
      AppendParagraphLines(value_, start, end, font_, wrap_mode_, width_limit,
                           layout_lines_);

      if (end == value_.size()) break;
      start = end + 1;
    }

    bool truncated = false;
    if (max_lines_ > 0 && layout_lines_.size() > max_lines_) {
      layout_lines_.resize(max_lines_);
      truncated = true;
      // Truncation turns the visible last line into paragraph-final.
      layout_lines_.back().ends_paragraph = true;
    }

    int16_t max_line_width = 0;
    for (const auto& line : layout_lines_) {
      max_line_width = std::max(max_line_width, line.width);
    }

    if (ellipsize_ && truncated && !layout_lines_.empty()) {
      // Ellipsize only after max-lines truncation to preserve full layout data.
      int16_t ellipsis_limit = width_limit > 0 ? width_limit : max_line_width;
      EllipsizeLine(layout_lines_.back(), font_, ellipsis_limit);
      max_line_width =
          std::max<int16_t>(max_line_width, layout_lines_.back().width);
    }

    int16_t block_width = max_line_width;
    if (wrap_mode_ == TextWrapMode::kWordWrap && width_limit > 0) {
      block_width = width_limit;
    }
    int16_t block_height = layout_lines_.size() * font_.metrics().maxHeight();
    layout_dims_ = Dimensions(block_width, block_height);
  }

  layout_valid_ = true;
}

void TextBlock::paint(const Canvas& canvas) const {
  if (width() <= 0 || height() <= 0) return;

  XDim width_limit = wrap_mode_ == TextWrapMode::kWordWrap
                         ? width()
                         : (ellipsize_ ? width() : 0);
  ensureLayout(width_limit);
  if (layout_dims_.width() == 0 || layout_dims_.height() == 0) return;

  roo_display::Color color =
      color_.a() == 0 ? parent()->defaultColor() : color_;

  canvas.drawTiled(Interior(roo_display::Box(0, 0, layout_dims_.width() - 1,
                                             layout_dims_.height() - 1),
                            layout_lines_, font_, color, text_align_),
                   bounds(), adjustAlignment(alignment_));
}

Dimensions TextBlock::getSuggestedMinimumDimensions() const {
  return text_dims_;
}

Dimensions TextBlock::onMeasure(WidthSpec width, HeightSpec height) {
  XDim width_limit = (wrap_mode_ == TextWrapMode::kWordWrap || ellipsize_) &&
                             width.kind() != UNSPECIFIED
                         ? width.value()
                         : 0;
  ensureLayout(width_limit);
  const Dimensions& desired = layout_dims_;
  return Dimensions(width.resolveSize(desired.width()),
                    height.resolveSize(desired.height()));
}

void TextBlock::setText(std::string value) {
  if (value_ == value) return;
  value_ = std::move(value);
  invalidateLayoutCache();
  recalculateNaturalDimensions();
  setDirty();
  requestLayout();
}

void TextBlock::setColor(roo_display::Color color) {
  if (color_ == color) return;
  color_ = color;
  setDirty();
}

void TextBlock::setAlignment(roo_display::Alignment alignment) {
  if (alignment_ == alignment) return;
  alignment_ = alignment;
  setDirty();
}

void TextBlock::setWrapMode(TextWrapMode wrap_mode) {
  if (wrap_mode_ == wrap_mode) return;
  wrap_mode_ = wrap_mode;
  invalidateLayoutCache();
  recalculateNaturalDimensions();
  setDirty();
  requestLayout();
}

void TextBlock::setTextAlign(TextAlign text_align) {
  if (text_align_ == text_align) return;
  text_align_ = text_align;
  setDirty();
}

void TextBlock::setMaxLines(uint16_t max_lines) {
  if (max_lines_ == max_lines) return;
  max_lines_ = max_lines;
  invalidateLayoutCache();
  recalculateNaturalDimensions();
  setDirty();
  requestLayout();
}

void TextBlock::setEllipsize(bool ellipsize) {
  if (ellipsize_ == ellipsize) return;
  ellipsize_ = ellipsize;
  invalidateLayoutCache();
  recalculateNaturalDimensions();
  setDirty();
  requestLayout();
}

}  // namespace roo_windows