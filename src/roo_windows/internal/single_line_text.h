#pragma once

#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_io/text/unicode.h"

namespace roo_windows::internal {
// Final-color text, selection and caret. No owned mask string or paint cache.
class SingleLineText : public roo_display::Drawable {
 public:
  SingleLineText(const roo_display::Font& font,
                 roo_display::Font::Options options, roo_display::Box bounds,
                 roo::string_view text, bool masked, bool show_last,
                 int16_t offset, int16_t highlight_begin, int16_t highlight_end,
                 roo_display::Color color, roo_display::Color highlight)
      : font_(font),
        options_(options),
        bounds_(bounds),
        text_(text),
        masked_(masked),
        show_last_(show_last),
        offset_(offset),
        begin_(highlight_begin + offset),
        end_(highlight_end + offset),
        color_(color),
        highlight_(highlight) {}
  roo_display::Box extents() const override { return bounds_; }

 private:
  void drawText(const roo_display::Surface& surface) const {
    using namespace roo_display;
    if (!masked_) {
      surface.drawObject(MakeTileOf(
          roo_display::StringViewLabel(text_, font_, color_, options_), bounds_,
          kOrigin.shiftBy(offset_) | kBaseline));
      return;
    }
    // Each glyph owns an advance cell. The mask has no kerning; tracking
    // belongs to the next cell, exactly as in the editor's metric cache.
    int16_t x = offset_;
    auto blank = [&](int16_t left, int16_t right) {
      if (right < left) return;
      surface.drawObject(MakeTileOf(
          roo_display::StringViewLabel("", font_, color_),
          Box(left, bounds_.yMin(), right, bounds_.yMax()), kNoAlign));
    };
    blank(bounds_.xMin(), std::min<int16_t>(x - 1, bounds_.xMax()));
    roo_io::Utf8Decoder decoder(text_);
    char32_t rune;
    bool first = true;
    while (decoder.next(rune)) {
      bool last = (const char*)decoder.data() == text_.data() + text_.size();
      char encoded[4];
      int count =
          roo_io::WriteUtf8Char(encoded, last && show_last_ ? rune : U'*');
      GlyphMetrics metrics = font_.getHorizontalStringMetrics(
          roo::string_view(encoded, count), options_);
      int16_t tracking = first ? 0 : options_.trackingPx();
      int16_t next = x + tracking + metrics.advance();
      Box cell(x, bounds_.yMin(), next - 1, bounds_.yMax());
      Surface clipped = surface;
      clipped.clipToExtents(bounds_);
      clipped.drawObject(MakeTileOf(
          roo_display::StringViewLabel(roo::string_view(encoded, count), font_,
                                       color_, options_),
          cell, kOrigin.shiftBy(x + tracking) | kBaseline));
      x = next;
      first = false;
    }
    blank(std::max<int16_t>(x, bounds_.xMin()), bounds_.xMax());
  }
  void drawTo(const roo_display::Surface& surface) const override {
    using namespace roo_display;
    auto band = [&](int16_t left, int16_t right, bool highlighted) {
      Box clip = Box::Intersect(
          bounds_, Box(left, bounds_.yMin(), right, bounds_.yMax()));
      if (clip.empty()) return;
      Surface part = surface;
      part.clipToExtents(clip);
      if (highlighted)
        part.set_bgcolor(AlphaBlend(surface.bgcolor(), highlight_));
      drawText(part);
    };
    if (end_ < begin_) {
      band(bounds_.xMin(), bounds_.xMax(), false);
      return;
    }
    band(bounds_.xMin(), begin_ - 1, false);
    band(begin_, end_, true);
    band(end_ + 1, bounds_.xMax(), false);
  }
  const roo_display::Font& font_;
  roo_display::Font::Options options_;
  roo_display::Box bounds_;
  roo::string_view text_;
  bool masked_, show_last_;
  int16_t offset_, begin_, end_;
  roo_display::Color color_, highlight_;
};
}  // namespace roo_windows::internal
