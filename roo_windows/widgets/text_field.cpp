#include "roo_windows/widgets/text_field.h"

namespace roo_windows {

namespace {

class TextFieldInterior : public roo_display::Drawable {
 public:
  TextFieldInterior(const roo_display::Font* font,
                    const roo_display::Box& extents, const std::string* text,
                    const std::vector<roo_display::GlyphMetrics>* glyphs,
                    int16_t selection_begin, int16_t selection_end,
                    int16_t cursor_position, roo_display::Color text_color,
                    roo_display::Color highlight_color, int16_t offset)
      : font_(font),
        extents_(extents),
        text_(text),
        glyphs_(glyphs),
        selection_begin_(selection_begin),
        selection_end_(selection_end),
        cursor_position_(cursor_position),
        text_color_(text_color),
        highlight_color_(highlight_color),
        offset_(offset) {}

  Box extents() const override { return extents_; }

 private:
  void drawTo(const roo_display::Surface& s) const override {
    if (selection_end_ > selection_begin_) {
      // There's a selection, so we will not draw the cursor, but we will draw
      // the selection.
      int16_t selection_xmin =
          (selection_begin_ == 0
               ? 0
               : ((*glyphs_)[selection_begin_ - 1].advance())) +
          offset_;
      int16_t selection_xmax =
          (*glyphs_)[selection_end_ - 1].advance() - 1 + offset_;
      if (selection_xmin > 0) {
        // Draw the text before the selection window.
        roo_display::Box pre_selection_clip(extents_.xMin(), extents_.yMin(),
                                            selection_xmin - 1,
                                            extents_.yMax());
        roo_display::Surface news = s;
        news.clipToExtents(pre_selection_clip);
        news.set_dx(news.dx() + offset_);
        font_->drawHorizontalString(news, (const uint8_t*)text_->c_str(),
                                    text_->size(), text_color_);
      }
      // Draw the highlighted area.
      {
        roo_display::Box selection_clip(selection_xmin, extents_.yMin(),
                                        selection_xmax, extents_.yMax());
        roo_display::Surface news = s;
        news.clipToExtents(selection_clip);
        news.set_dx(news.dx() + offset_);
        Color bg = highlight_color_;
        bg.set_a(0x80);
        news.set_bgcolor(roo_display::alphaBlend(s.bgcolor(), bg));
        font_->drawHorizontalString(news, (const uint8_t*)text_->c_str(),
                                    text_->size(), text_color_);
      }
      if (selection_xmax < extents_.xMax()) {
        // Draw post-selection area.
        // Draw the text before the selection window.
        roo_display::Box post_selection_clip(selection_xmax + 1,
                                             extents_.yMin(), extents_.xMax(),
                                             extents_.yMax());
        roo_display::Surface news = s;
        news.clipToExtents(post_selection_clip);
        news.set_dx(news.dx() + offset_);
        font_->drawHorizontalString(news, (const uint8_t*)text_->c_str(),
                                    text_->size(), text_color_);
      }
      return;
    }
    // There's no selection; we will draw a cursor though.
    int16_t cursor_x =
        (cursor_position_ == 0 ? 0
                               : (*glyphs_)[cursor_position_ - 1].advance()) +
        offset_;
    if (cursor_x > 0) {
      // Draw the text before the cursor.
      roo_display::Box pre_cursor(extents_.xMin(), extents_.yMin(),
                                  cursor_x - 1, extents_.yMax());
      roo_display::Surface news = s;
      news.clipToExtents(pre_cursor);
      news.set_dx(news.dx() + offset_);
      font_->drawHorizontalString(news, (const uint8_t*)text_->c_str(),
                                  text_->size(), text_color_);
    }
    {
      // Draw the cursor.
      roo_display::Box cursor(cursor_x, extents_.yMin(), cursor_x + 1,
                              extents_.yMax());
      roo_display::Surface news = s;
      news.clipToExtents(cursor);
      news.set_dx(news.dx() + offset_);
      Color bg = highlight_color_;
      news.set_bgcolor(roo_display::alphaBlend(s.bgcolor(), bg));
      font_->drawHorizontalString(news, (const uint8_t*)text_->c_str(),
                                  text_->size(), text_color_);
      int past_glyph = (glyphs_->empty() ? 0 : glyphs_->back().glyphXMax() + 1);
      if (past_glyph <= extents_.xMax()) {
        // Must mean that the cursor is past the last character, and not
        // entirely drawn yet.
        if (past_glyph <= extents_.xMax() - 2) {
          // White space at the end of the text.
          s.drawObject(roo_display::FilledRect(past_glyph, extents_.yMin(),
                                               extents_.xMax() - 2,
                                               extents_.yMax(), s.bgcolor()));
        }
        s.drawObject(roo_display::FilledRect(extents_.xMax() - 1,
                                             extents_.yMin(), extents_.xMax(),
                                             extents_.yMax(), bg));
      }
    }
    if (cursor_x + 1 < extents_.xMax()) {
      // Draw post-cursor.
      roo_display::Box post_cursor(cursor_x + 2, extents_.yMin(),
                                   extents_.xMax(), extents_.yMax());
      roo_display::Surface news = s;
      news.clipToExtents(post_cursor);
      news.set_dx(news.dx() + offset_);
      font_->drawHorizontalString(news, (const uint8_t*)text_->c_str(),
                                  text_->size(), text_color_);
    }
  }

  const roo_display::Font* font_;
  roo_display::Box extents_;
  const std::string* text_;
  const std::vector<roo_display::GlyphMetrics>* glyphs_;
  int16_t selection_begin_;
  int16_t selection_end_;
  int16_t cursor_position_;
  roo_display::Color text_color_;
  roo_display::Color highlight_color_;
  int16_t offset_;
};

}  // namespace

bool TextField::paint(const roo_display::Surface& s) {
  roo_display::Color color =
      text_color_.a() == 0 ? parent()->defaultColor() : text_color_;
  roo_display::Color highlight_color = highlight_color_.a() == 0
                                           ? parent()->theme().color.secondary
                                           : highlight_color_;
  const std::string* val = &value_;
  if (val->empty()) {
    // Show hint with half the opacity.
    val = &hint_;
    color.set_a(color.a() / 2);
  }
  if (!isEdited()) {
    Color c = roo_display::alphaBlend(s.bgcolor(), color);
    s.drawObject(roo_display::MakeTileOf(
        roo_display::TextLabel(font_, *val, c), bounds(), halign_, valign_,
        roo_display::color::Transparent, s.fill_mode()));
    return true;
  }

  int16_t xMin = 0;
  int16_t xMax = 2;
  if (!val->empty()) {
    // Figure out if the text will fit. If so, ignore the editor's offset.
    // If not, respect the editor's offset.
    xMin = editor().glyphs().front().glyphXMin();
    xMax = editor().glyphs().back().glyphXMax();
    if (!editor().has_selection() &&
        editor().cursor_position() == editor().glyphs().size() &&
        xMax < editor().glyphs().back().advance() + 2) {
      xMax = editor().glyphs().back().advance() + 2;
    }
  }

  int16_t w = xMax - xMin + 1;
  int16_t draw_xoffset = editor().draw_xoffset();
  if (w < width()) {
    draw_xoffset = 0;
  } else {
    w = width();
  }
  TextFieldInterior interior(
      &font_,
      Box(0, -font().metrics().glyphYMax(), w - 1,
          -font().metrics().glyphYMin()),
      val, &editor().glyphs(), editor().selection_begin(),
      editor().selection_end(), editor().cursor_position(), color,
      highlight_color, draw_xoffset);
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
    return;
  }
  keyboard_.show();
  keyboard_.setListener(this);
  measure();
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
  const std::string& s = target_->content();
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
  int actual_size = target_->font().getHorizontalStringGlyphMetrics(
      (const uint8_t*)s.c_str(), s.size(), &*glyphs_.begin(), 0, max_count);
  glyphs_.resize(actual_size);
  roo_display::Utf8Decoder decoder((const uint8_t*)s.c_str(), s.size());
  offsets_.resize(actual_size);
  for (int16_t i = 0; i < actual_size; ++i) {
    offsets_[i] = decoder.data() - (const uint8_t*)s.c_str();
    decoder.next();
  }

  cursor_position_ = glyphs_.size();
}

void TextFieldEditor::rune(uint32_t rune) {
  if (target_ == nullptr) return;
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
  keyboard_.hide();
  target_->onEditFinished(true);
}

void TextFieldEditor::del() {
  if (target_ == nullptr) return;
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
