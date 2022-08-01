#include "roo_windows/widgets/text_field.h"

#include "roo_material_icons.h"
#include "roo_material_icons/filled/24/action.h"

namespace roo_windows {

bool VisibilityToggle::paint(const Surface& s) {
  roo_display::MaterialIcon icon(isOn() ? ic_filled_24_action_visibility()
                                        : ic_filled_24_action_visibility_off());
  roo_display::Color color = parent()->defaultColor();
  icon.color_mode().setColor(color);
  roo_display::Tile tile(&icon, bounds(), roo_display::HAlign::Center(),
                         roo_display::VAlign::Middle());
  s.drawObject(tile);
  return true;
}

namespace {

class TextFieldInterior : public roo_display::Drawable {
 public:
  TextFieldInterior(const roo_display::Font* font,
                    const roo_display::Box& extents, const std::string* text,
                    bool edited, bool starred, bool show_last_glyph,
                    int16_t text_xmin, int16_t text_xmax,
                    int16_t selection_xmin, int16_t selection_xmax,
                    int16_t cursor_x, roo_display::Color text_color,
                    roo_display::Color highlight_color, int16_t offset,
                    TextField::Decoration decoration)
      : font_(font),
        extents_(extents),
        text_(text),
        edited_(edited),
        starred_(starred),
        show_last_glyph_(show_last_glyph),
        text_xmin_(text_xmin),
        text_xmax_(text_xmax),
        selection_xmin_(selection_xmin),
        selection_xmax_(selection_xmax),
        cursor_x_(cursor_x),
        text_color_(text_color),
        highlight_color_(highlight_color),
        offset_(offset),
        decoration_(decoration) {}

  Box extents() const override { return extents_; }

 private:
  void drawTo(const roo_display::Surface& s) const override {
    std::string starred_text;
    const std::string* text = text_;
    if (starred_ && !text_->empty()) {
      starred_text.resize(text_->size());
      for (size_t i = 0; i < starred_text.size() - 1; i++) {
        starred_text[i] = '*';
      }
      starred_text[text_->size() - 1] = show_last_glyph_ ? text_->back() : '*';
      text = &starred_text;
    }
    int16_t text_height = font_->metrics().maxHeight();
    int16_t text_ymin = extents_.yMin();
    int16_t text_ymax = text_ymin + text_height - 1;
    if (selection_xmax_ >= selection_xmin_) {
      // There's a selection, so we will not draw the cursor, but we will draw
      // the selection.
      if (selection_xmin_ > 0) {
        // Draw the text before the selection window.
        roo_display::Box pre_selection_clip(extents_.xMin(), text_ymin,
                                            selection_xmin_ - 1, text_ymax);
        roo_display::Surface news = s;
        news.clipToExtents(pre_selection_clip);
        news.set_dx(news.dx() + offset_);
        font_->drawHorizontalString(news, *text, text_color_);
      }
      // Draw the highlighted area.
      {
        roo_display::Box selection_clip(selection_xmin_, text_ymin,
                                        selection_xmax_, text_ymax);
        roo_display::Surface news = s;
        news.clipToExtents(selection_clip);
        news.set_dx(news.dx() + offset_);
        Color bg = highlight_color_;
        bg.set_a(0x80);
        news.set_bgcolor(roo_display::alphaBlend(s.bgcolor(), bg));
        font_->drawHorizontalString(news, *text, text_color_);
      }
      if (selection_xmax_ < extents_.xMax()) {
        // Draw post-selection area.
        // Draw the text before the selection window.
        roo_display::Box post_selection_clip(selection_xmax_ + 1, text_ymin,
                                             extents_.xMax(), text_ymax);
        roo_display::Surface news = s;
        news.clipToExtents(post_selection_clip);
        news.set_dx(news.dx() + offset_);
        font_->drawHorizontalString(news, *text, text_color_);
      }
      return;
    }
    // There's no selection; we may need to draw a cursor though.
    if (cursor_x_ > 0) {
      // Draw the text before the cursor.
      roo_display::Box pre_cursor(extents_.xMin(), text_ymin, cursor_x_ - 1,
                                  text_ymax);
      roo_display::Surface news = s;
      news.clipToExtents(pre_cursor);
      news.set_dx(news.dx() + offset_);
      font_->drawHorizontalString(news, *text, text_color_);
    }
    // The x coordinate of the first pixel that will not be overwritten by text.
    int past_glyph = text_xmax_ + 1;
    {
      // Draw the cursor.
      roo_display::Box cursor(cursor_x_, text_ymin, cursor_x_ + 1, text_ymax);
      roo_display::Surface news = s;
      news.clipToExtents(cursor);
      news.set_dx(news.dx() + offset_);
      Color bg = highlight_color_;
      news.set_bgcolor(roo_display::alphaBlend(s.bgcolor(), bg));
      font_->drawHorizontalString(news, *text, text_color_);
      if (past_glyph <= cursor_x_ + 1) {
        // Must mean that the cursor is past the last character, and not
        // entirely drawn yet.
        if (past_glyph < cursor_x_) {
          // White space at the end of the text.
          s.drawObject(roo_display::FilledRect(
              past_glyph, text_ymin, cursor_x_ + 1, text_ymax, s.bgcolor()));
        }
        s.drawObject(roo_display::FilledRect(cursor_x_, text_ymin,
                                             cursor_x_ + 1, text_ymax, bg));
      }
    }
    int16_t xstart_remaining = cursor_x_ + 2;
    if (xstart_remaining <= extents_.xMax()) {
      // Draw post-cursor.
      roo_display::Box post_cursor(xstart_remaining, text_ymin, extents_.xMax(),
                                   text_ymax);
      roo_display::Surface news = s;
      news.clipToExtents(post_cursor);
      news.set_dx(news.dx() + offset_);
      if (past_glyph > xstart_remaining) {
        font_->drawHorizontalString(news, *text, text_color_);
        xstart_remaining = past_glyph;
      }
      if (xstart_remaining <= extents_.xMax()) {
        // Draw the entirely empty area to the right.
        s.drawObject(roo_display::FilledRect(xstart_remaining, text_ymin,
                                             extents_.xMax(), text_ymax,
                                             s.bgcolor()));
      }
    }
    // Draw the decoration (underline) if present.
    if (decoration_ == TextField::UNDERLINE) {
      // We have extra 6 pixels at the bottom to fill.
      s.drawObject(roo_display::FilledRect(extents_.xMin(), extents_.yMax() - 5,
                                           extents_.xMax(), extents_.yMax() - 3,
                                           s.bgcolor()));
      s.drawObject(roo_display::FilledRect(extents_.xMin(), extents_.yMax() - 2,
                                           extents_.xMax(), extents_.yMax(),
                                           highlight_color_));
    }
  }

  const roo_display::Font* font_;
  roo_display::Box extents_;
  const std::string* text_;
  bool edited_;
  bool starred_;
  bool show_last_glyph_;
  int16_t text_xmin_;
  int16_t text_xmax_;
  int16_t selection_xmin_;
  int16_t selection_xmax_;
  int16_t cursor_x_;
  roo_display::Color text_color_;
  roo_display::Color highlight_color_;
  int16_t offset_;
  TextField::Decoration decoration_;
};

}  // namespace

bool TextField::paint(const roo_display::Surface& s) {
  roo_display::Color color =
      text_color_.a() == 0 ? parent()->defaultColor() : text_color_;
  roo_display::Color highlight_color = highlight_color_.a() == 0
                                           ? parent()->theme().color.secondary
                                           : highlight_color_;
  const std::string* val = &value_;
  bool starred = isStarred();
  bool show_last_glyph = false;
  if (val->empty()) {
    // Show hint with half the opacity.
    val = &hint_;
    color.set_a(color.a() / 2);
    color = roo_display::alphaBlend(s.bgcolor(), color);
    starred = false;
  }
  // Calculate the position of the undecorated text within the bounds.
  int16_t decoration_height_offset = 0;
  switch (decoration_) {
    case UNDERLINE: {
      decoration_height_offset = 6;
      break;
    }
    default: {
    }
  }

  int16_t xMin = 0;
  int16_t xMax = -1;
  int16_t selection_xmin = 0;
  int16_t selection_xmax = -1;
  int16_t draw_xoffset = 0;
  int16_t cursor_x = -2;
  if (isEdited()) {
    show_last_glyph = editor().lastGlyphRecentlyEntered();
    if (!val->empty()) {
      // Figure out if the text will fit. If so, ignore the editor's offset.
      // If not, respect the editor's offset.
      xMin = editor().glyphs().front().glyphXMin();
      xMax = editor().glyphs().back().glyphXMax();
    }

    int16_t w = xMax - xMin + 1;
    int16_t draw_xoffset = editor().draw_xoffset();
    if (w < width()) {
      draw_xoffset = 0;
    } else {
      w = width();
    }

    int16_t selection_begin = editor().selection_begin();
    int16_t selection_end = editor().selection_end();

    if (selection_begin > selection_end) {
      selection_xmin =
          (selection_begin == 0)
              ? 0
              : ((editor().glyphs()[selection_begin - 1].advance())) +
                    draw_xoffset;
      selection_xmax =
          editor().glyphs()[selection_end - 1].advance() - 1 + draw_xoffset;
    }

    if (editor().isBlinkingCursorNowOn()) {
      cursor_x =
          (editor().cursor_position() == 0
               ? 0
               : editor().glyphs()[editor().cursor_position() - 1].advance()) +
          draw_xoffset;
    }
  } else if (!val->empty()) {
    if (starred) {
      // Calculate the bounds as-if the string is all-star.
      roo_display::GlyphMetrics metrics;
      font_.getGlyphMetrics('*', roo_display::FONT_LAYOUT_HORIZONTAL, &metrics);
      roo_display::Utf8Decoder decoder((const uint8_t*)val->c_str(),
                                       val->size());
      int length = 0;
      while (decoder.has_next()) {
        decoder.next();
        ++length;
      }
      xMin = metrics.glyphXMin();
      xMax = metrics.glyphXMax() + (length - 1) * metrics.advance();
    } else {
      // Calculate the bounds based on the actual string content.
      roo_display::GlyphMetrics metrics = font_.getHorizontalStringMetrics(*val);
      xMin = metrics.glyphXMin();
      xMax = metrics.glyphXMax();
    }
  }

  Color c = highlight_color;
  if (!isEdited()) {
    c = theme().color.onSurface;
    c.set_a(0x20);
    c = roo_display::alphaBlend(s.bgcolor(), c);
  }

  TextFieldInterior interior(
      &font_,
      Box(0, -font().metrics().glyphYMax(), width() - 1,
          -font().metrics().glyphYMin() + decoration_height_offset),
      val, isEdited(), starred, show_last_glyph, xMin, xMax, selection_xmin,
      selection_xmax, cursor_x, color, c, draw_xoffset, decoration_);
  s.drawObject(roo_display::MakeTileOf(interior, bounds(), halign_, valign_,
                                       roo_display::color::Transparent,
                                       s.fill_mode()));
  return true;
}

void TextFieldEditor::edit(TextField* target) {
  if (target_ == target) return;
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
    roo_display::Utf8Decoder decoder((const uint8_t*)s.c_str(), s.size());
    offsets_.resize(actual_size);
    for (int16_t i = 0; i < actual_size; ++i) {
      offsets_[i] = decoder.data() - (const uint8_t*)s.c_str();
      decoder.next();
    }
  } else {
    roo_display::GlyphMetrics star_metrics;
    target_->font().getGlyphMetrics('*', roo_display::FONT_LAYOUT_HORIZONTAL,
                                    &star_metrics);
    roo_display::GlyphMetrics last_rune_metrics = star_metrics;
    // Determine how many stars.
    roo_display::Utf8Decoder decoder((const uint8_t*)s.c_str(), s.size());
    while (true) {
      ++actual_size;
      roo_display::unicode_t rune = decoder.next();
      if (!decoder.has_next()) {
        // Last one; perhaps actually measure.
        if (last_glyph_recently_entered_) {
          target_->font().getGlyphMetrics(
              rune, roo_display::FONT_LAYOUT_HORIZONTAL, &last_rune_metrics);
        }
        break;
      }
    }
    glyphs_.resize(actual_size);
    offsets_.resize(actual_size);
    int16_t xpos = 0;
    for (size_t i = 0; i < actual_size - 1; ++i) {
      glyphs_[i] = roo_display::GlyphMetrics(
          star_metrics.glyphXMin() + xpos, star_metrics.glyphYMin(),
          star_metrics.glyphXMax() + xpos, star_metrics.glyphYMax(),
          star_metrics.advance() + xpos);
      xpos += star_metrics.advance();
      offsets_[i] = i;
    }
    glyphs_[actual_size - 1] = roo_display::GlyphMetrics(
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
    // Delete the selected text, remove the selection, and set the cursor where
    // there was the selection.
    val.erase(val.begin() + offsets_[selection_begin_],
              val.begin() + offsets_[selection_end_]);
    cursor_position_ = selection_begin_;
  }
  uint8_t encoded[4];
  int count = roo_display::EncodeRuneAsUtf8(rune, encoded);
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
  last_glyph_recently_entered_ = false;
  restartCursor();
  if (has_selection()) {
    // Delete the selected text, remove the selection, and set the cursor where
    // there was the selection.
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
