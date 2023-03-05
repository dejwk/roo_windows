
#include "roo_windows/config.h"
#include "roo_windows/widgets/text_field.h"

#include "roo_material_icons.h"
#include "roo_material_icons/filled/18/action.h"
#include "roo_material_icons/filled/24/action.h"
#include "roo_material_icons/filled/36/action.h"
#include "roo_material_icons/filled/48/action.h"
#include "roo_windows/config.h"

namespace roo_windows {

using namespace roo_display;

void VisibilityToggle::paint(const Canvas& canvas) const {
  MaterialIcon icon(
      isOn() ? SCALED_ROO_ICON(filled, action_visibility)
             : SCALED_ROO_ICON(filled, action_visibility_off));
  Color color = parent()->defaultColor();
  icon.color_mode().setColor(color);
  canvas.drawTiled(icon, bounds(), kCenter | kMiddle, isInvalidated());
}

namespace {

// Just the text + highlight + cursor, in its original coordinates. The
// extents are externally provided, may be clipped and not contain all the text.
class TextFieldInterior : public Drawable {
 public:
  TextFieldInterior(const Font* font, const Box& extents, StringView text,
                    bool edited, bool starred, bool show_last_glyph,
                    int16_t x_offset, int16_t highlight_xmin,
                    int16_t highlight_xmax, Color text_color,
                    Color highlight_color)
      : font_(font),
        extents_(extents),
        text_(text),
        edited_(edited),
        starred_(starred),
        show_last_glyph_(show_last_glyph),
        x_offset_(x_offset),
        highlight_xmin_(highlight_xmin + x_offset),
        highlight_xmax_(highlight_xmax + x_offset),
        text_color_(text_color),
        highlight_color_(highlight_color) {}

  Box extents() const override { return extents_; }

 private:
  void drawTo(const Surface& s) const override {
    std::string starred_text;
    StringView text = text_;
    if (starred_ && !text_.empty()) {
      starred_text.resize(text_.size());
      for (size_t i = 0; i < starred_text.size() - 1; i++) {
        starred_text[i] = '*';
      }
      starred_text[text_.size() - 1] = show_last_glyph_ ? text_.back() : '*';
      text = starred_text;
    }

    auto tiled_text = MakeTileOf(StringViewLabel(text, *font_, text_color_),
                                 extents_, kOrigin.shiftBy(x_offset_));

    if (highlight_xmin_ > 0) {
      // Draw the text before the selection window.
      Box pre_highlight_clip(extents_.xMin(), extents_.yMin(),
                             highlight_xmin_ - 1, extents_.yMax());
      Surface news = s;
      news.clipToExtents(pre_highlight_clip);
      news.drawObject(tiled_text);
    }
    if (highlight_xmax_ >= highlight_xmin_) {
      // Draw the highlighted area.
      Box highlight_clip(highlight_xmin_, extents_.yMin(), highlight_xmax_,
                         extents_.yMax());
      Surface news = s;
      news.clipToExtents(highlight_clip);
      news.set_bgcolor(AlphaBlend(s.bgcolor(), highlight_color_));
      news.drawObject(tiled_text);
    }
    if (highlight_xmax_ < highlight_xmin_ ||
        highlight_xmax_ < extents_.xMax()) {
      // Draw post-highlighted area.
      Box post_highlight_clip(highlight_xmax_ + 1, extents_.yMin(),
                              extents_.xMax(), extents_.yMax());
      Surface news = s;
      news.clipToExtents(post_highlight_clip);
      news.drawObject(tiled_text);
    }
  }

  const Font* font_;
  Box extents_;
  const StringView text_;
  bool edited_;
  bool starred_;
  bool show_last_glyph_;
  int16_t x_offset_;
  int16_t highlight_xmin_;
  int16_t highlight_xmax_;
  Color text_color_;
  Color highlight_color_;
};

}  // namespace

void TextField::paint(const Canvas& canvas) const {
  Color color = text_color_.a() == 0 ? parent()->defaultColor() : text_color_;
  Color highlight_color = highlight_color_.a() == 0
                              ? parent()->theme().color.secondary
                              : highlight_color_;
  if (!isEdited()) {
    highlight_color = parent()->theme().color.onSurface;
    highlight_color.set_a(0x20);
  }
  StringView text = value_;
  bool starred = isStarred();
  // bool show_last_glyph = false;
  if (text.empty()) {
    // Show hint with half the opacity.
    text = hint_;
    color.set_a(color.a() / 2);
    color = AlphaBlend(canvas.bgcolor(), color);
    starred = false;
  }
  Padding padding = getPadding();

  // Determine our vertical position.
  int16_t text_ymin = -font_.metrics().glyphYMax();
  int16_t text_ymax = -font_.metrics().glyphYMin();
  int16_t decorated_text_ymin = text_ymin;
  int16_t decorated_text_ymax = text_ymax;
  switch (decoration_) {
    case UNDERLINE: {
      decorated_text_ymax += 6;
      break;
    }
    default: {
    }
  }
  Alignment padded = adjustAlignment(alignment_);
  int16_t yOffset =
      padded.v().resolveOffset(0, height() - 1, decorated_text_ymin - text_ymin,
                               decorated_text_ymax - text_ymin);
  int16_t available_width = width() - padding.left() - padding.right();
  int16_t advance_width;
  if (isEdited()) {
    // The metrics are captured in the editor.
    advance_width = editor().glyphs().back().advance();
    if (editor().cursor_position() == (int)editor().glyphs().size()) {
      // Leave two pixels for the cursor at the end of text.
      advance_width += 2;
    }
  } else if (!text.empty()) {
    // Calculate the glyph metrics now.
    if (starred) {
      // Calculate the bounds as-if the string is all-star.
      GlyphMetrics metrics;
      font_.getGlyphMetrics('*', FONT_LAYOUT_HORIZONTAL, &metrics);
      Utf8Decoder decoder(text);
      int length = 0;
      while (decoder.has_next()) {
        decoder.next();
        ++length;
      }
      advance_width = metrics.advance() * (length - 1);
    } else {
      // Calculate the bounds based on the actual string content.
      GlyphMetrics metrics = font_.getHorizontalStringMetrics(text);
      advance_width = metrics.advance();
    }
  }
  bool is_x_offset = (advance_width > available_width);
  int16_t highlight_xmin = 0;
  int16_t highlight_xmax = -1;

  if (isEdited()) {
    if (editor().has_selection()) {
      int16_t selection_begin = editor().selection_begin();
      int16_t selection_end = editor().selection_end();
      highlight_xmin =
          (selection_begin == 0)
              ? 0
              : ((editor().glyphs()[selection_begin - 1].advance()));
      highlight_xmax = editor().glyphs()[selection_end - 1].advance() - 1;
    } else if (editor().isBlinkingCursorNowOn()) {
      highlight_xmin =
          (editor().cursor_position() == 0
               ? 0
               : editor().glyphs()[editor().cursor_position() - 1].advance());
      highlight_xmax = highlight_xmin + 1;
    }
  }

  if (isInvalidated() && yOffset > 0) {
    canvas.clearRect(0, 0, width() - 1, yOffset - 1);
  }

  Canvas my_canvas = canvas;
  my_canvas.shift(0, yOffset - text_ymin);
  // int16_t text_height = font_.metrics().maxHeight();
  Box text_clip_box(0, text_ymin, width() - 1, text_ymax);
  if (is_x_offset) {
    // We truncate the text on the advance boundaries, so that it appears
    // aligned with the decoration.
    my_canvas.clearRect(0, text_ymin, padding.left() - 1, text_ymax);
    my_canvas.clearRect(text_ymax - padding.right() + 1, text_ymin, width() - 1,
                        text_ymax);
    text_clip_box =
        Box(padding.left(), text_clip_box.yMin(),
            text_clip_box.xMax() - padding.right(), text_clip_box.yMax());
  }
  XDim xoffset =
      padded.h().resolveOffset<XDim>(0, width() - 1, 0, advance_width) +
      editor().draw_xoffset();
  my_canvas.drawObject(TextFieldInterior(
      &font_, text_clip_box, text, isEdited(), isStarred() && !value_.empty(),
      editor().lastGlyphRecentlyEntered(), xoffset, highlight_xmin,
      highlight_xmax, color, highlight_color));

  int16_t min_y_undrawn = yOffset + text_clip_box.height();
  // Draw the decoration.
  switch (decoration_) {
    case UNDERLINE: {
      // We have extra 6 pixels at the bottom to fill.
      canvas.clearRect(0, min_y_undrawn, width() - 1, min_y_undrawn + 2);
      canvas.clearRect(0, min_y_undrawn + 3, padding.left() - 1,
                       min_y_undrawn + 5);
      canvas.fillRect(padding.left(), min_y_undrawn + 3,
                      width() - 1 - padding.right(), min_y_undrawn + 5,
                      highlight_color);
      canvas.clearRect(width() - padding.right() - 1, min_y_undrawn + 3,
                       width() - 1, min_y_undrawn + 5);
      min_y_undrawn += 6;
      break;
    }
    default: {
    }
  }
  if (isInvalidated() && min_y_undrawn < height()) {
    canvas.clearRect(0, min_y_undrawn, width() - 1, height() - 1);
  }
}

void TextFieldEditor::edit(TextField* target) {
  if (target_ == target) return;
  last_glyph_recently_entered_ = false;
  TextField* old_target = target_;
  target_ = target;
  if (target == nullptr) {
    old_target->onEditFinished(false);
    keyboard_.hide();
    keyboard_.setListener(nullptr);
    old_target->invalidateInterior();
    return;
  }
  keyboard_.show();
  keyboard_.setListener(this);
  target->invalidateInterior();
  last_glyph_recently_entered_ = false;
  measure();
  restartCursor();
}

bool TextFieldEditor::isEdited(const TextField* target) const {
  return (target_ == target);
}

void TextFieldEditor::setSelection(int16_t selection_begin,
                                   int16_t selection_end) {
  if (target_ == nullptr) return;
  if (selection_end > glyphs_.size()) {
    selection_end = glyphs_.size();
  }
  if (selection_end < selection_begin) {
    selection_begin = selection_end;
  }
  if (selection_begin < 0) {
    selection_begin = 0;
  }
  if (selection_end < 0) {
    selection_end = 0;
  }
  selection_begin_ = selection_begin;
  selection_end_ = selection_end;
  target_->markDirty();
}

// Sets up glyphs_ to contain all the metrics of the value.
void TextFieldEditor::measure() {
  // The total number of glyphs can't exceed the string length.
  bool empty = target_->content().empty();
  const std::string& s = empty ? target_->hint() : target_->content();
  selection_begin_ = 0;
  selection_end_ = 0;
  int max_count = s.size();
  glyphs_.resize(max_count);
  // Special case for an empty string, because begin() might not return
  // a valid pointer.
  if (max_count == 0) {
    cursor_position_ = 0;
    offsets_.clear();
    return;
  }
  int actual_size = 0;
  if (empty || !target_->isStarred()) {
    actual_size = target_->font().getHorizontalStringGlyphMetrics(
        s, &*glyphs_.begin(), 0, max_count);
    glyphs_.resize(actual_size);
    Utf8Decoder decoder((const uint8_t*)s.c_str(), s.size());
    offsets_.resize(actual_size);
    for (int16_t i = 0; i < actual_size; ++i) {
      offsets_[i] = decoder.data() - (const uint8_t*)s.c_str();
      decoder.next();
    }
  } else {
    GlyphMetrics star_metrics;
    target_->font().getGlyphMetrics('*', FONT_LAYOUT_HORIZONTAL, &star_metrics);
    GlyphMetrics last_rune_metrics = star_metrics;
    // Determine how many stars.
    Utf8Decoder decoder((const uint8_t*)s.c_str(), s.size());
    while (true) {
      ++actual_size;
      unicode_t rune = decoder.next();
      if (!decoder.has_next()) {
        // Last one; perhaps actually measure.
        if (last_glyph_recently_entered_) {
          target_->font().getGlyphMetrics(rune, FONT_LAYOUT_HORIZONTAL,
                                          &last_rune_metrics);
        }
        break;
      }
    }
    glyphs_.resize(actual_size);
    offsets_.resize(actual_size);
    int16_t xpos = 0;
    for (size_t i = 0; i < actual_size - 1; ++i) {
      glyphs_[i] = GlyphMetrics(
          star_metrics.glyphXMin() + xpos, star_metrics.glyphYMin(),
          star_metrics.glyphXMax() + xpos, star_metrics.glyphYMax(),
          star_metrics.advance() + xpos);
      xpos += star_metrics.advance();
      offsets_[i] = i;
    }
    glyphs_[actual_size - 1] = GlyphMetrics(
        last_rune_metrics.glyphXMin() + xpos, last_rune_metrics.glyphYMin(),
        last_rune_metrics.glyphXMax() + xpos, last_rune_metrics.glyphYMax(),
        last_rune_metrics.advance() + xpos);
    xpos += last_rune_metrics.advance();
    offsets_[actual_size - 1] = actual_size - 1;
  }
  cursor_position_ = empty ? 0 : glyphs_.size();
}

void TextFieldEditor::blinkCursor() {
  if (target_ == nullptr) return;
  auto now = roo_time::Uptime::Now();
  if (now - last_cursor_shown_time_ > 2 * kCursorBlinkInterval) {
    restartCursor();
    target_->markDirty();
  }
  if (now - last_cursor_shown_time_ > kCursorBlinkInterval) {
    blinking_cursor_is_on_ = false;
    target_->markDirty();
    cursor_blinker_.scheduleAfter(kCursorBlinkInterval);
  }
}

void TextFieldEditor::restartLastGlyphRecentlyEntered() {
  if (target_ == nullptr) return;
  // Note: need to check for empty as a special case, because we may
  // be showing the hint.
  if (target_->content().empty() || cursor_position() == glyphs_.size()) {
    last_glyph_recently_entered_ = true;
    last_glyph_hider_.scheduleAfter(kShowLastGlyphInterval);
  }
}

void TextFieldEditor::hideLastGlyph() {
  if (!last_glyph_recently_entered_) return;
  last_glyph_recently_entered_ = false;
  measure();
}

void TextFieldEditor::restartCursor() {
  last_cursor_shown_time_ = roo_time::Uptime::Now();
  blinking_cursor_is_on_ = true;
  cursor_blinker_.scheduleAfter(kCursorBlinkInterval);
}

void TextFieldEditor::rune(uint32_t rune) {
  if (target_ == nullptr) return;
  restartCursor();
  restartLastGlyphRecentlyEntered();
  std::string& val = target_->value_;
  if (has_selection()) {
    // Delete the selected text, remove the selection, and set the cursor
    // where there was the selection.
    val.erase(val.begin() + offsets_[selection_begin_],
              val.begin() + offsets_[selection_end_]);
    cursor_position_ = selection_begin_;
  }
  uint8_t encoded[4];
  int count = EncodeRuneAsUtf8(rune, encoded);
  if (cursor_position_ == offsets_.size()) {
    // At end of string.
    val.insert(val.end(), encoded, encoded + count);
  } else {
    val.insert(val.begin() + offsets_[cursor_position_], encoded,
               encoded + count);
  }

  // Need to re-measure, because the glyphs use absolute coordinates, and
  // also, kerning makes it not trivial.
  int16_t saved_pos = cursor_position_ + 1;
  measure();
  cursor_position_ = saved_pos;
  target_->markDirty();
}

void TextFieldEditor::enter() {
  if (target_ == nullptr) return;
  last_glyph_recently_entered_ = false;
  TextField* old_target = target_;
  target_ = nullptr;
  old_target->onEditFinished(true);
  keyboard_.hide();
  keyboard_.setListener(nullptr);
  old_target->invalidateInterior();
  return;
}

void TextFieldEditor::del() {
  if (target_ == nullptr) return;
  if (target_->value_.empty()) return;
  last_glyph_recently_entered_ = false;
  restartCursor();
  if (has_selection()) {
    // Delete the selected text, remove the selection, and set the cursor
    // where there was the selection.
    std::string& val = target_->value_;
    val.erase(val.begin() + offsets_[selection_begin_],
              val.begin() + offsets_[selection_end_]);
    // Need to re-measure, because the glyphs use absolute coordinates, and
    // also, kerning makes it not trivial.
    int16_t saved_pos = selection_begin_;
    measure();
    cursor_position_ = saved_pos;
    target_->markDirty();
  } else if (cursor_position_ > 0) {
    std::string& val = target_->value_;
    if (cursor_position_ == offsets_.size()) {
      val.erase(val.begin() + offsets_[cursor_position_ - 1], val.end());
    } else {
      val.erase(val.begin() + offsets_[cursor_position_ - 1],
                val.begin() + offsets_[cursor_position_]);
    }
    int16_t saved_pos = cursor_position_ - 1;
    measure();
    cursor_position_ = saved_pos;
    target_->markDirty();
  }
}

}  // namespace roo_windows
