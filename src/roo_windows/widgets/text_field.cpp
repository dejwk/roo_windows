
#include "roo_windows/widgets/text_field.h"

#include <algorithm>
#include "roo_windows/internal/single_line_text.h"

#include "roo_backport/string_view.h"
#include "roo_display/ui/text_label.h"
#include "roo_icons/filled/action.h"
#include "roo_io/text/unicode.h"
#include "roo_windows/config.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"

namespace roo_windows {

using namespace roo_display;

void VisibilityToggle::paint(PaintContext& ctx) const {
  Pictogram icon(isOn() ? SCALED_ROO_ICON(filled, action_visibility)
                        : SCALED_ROO_ICON(filled, action_visibility_off));
  Color color = parent()->defaultColor();
  icon.color_mode().setColor(color);
  ctx.drawTiled(icon, bounds(), kCenter | kMiddle, isInvalidated());
}

void TextField::paint(PaintContext& ctx) const {
  const Canvas& canvas = ctx.canvas();
  const FrameworkColorScheme& colors = parent()->theme().framework.color;
  Color color = text_color_.a() == 0
                    ? colors.resolve(FrameworkColorRole::kContent)
                    : text_color_;
  Color highlight_color = highlight_color_.a() == 0
                              ? colors.resolve(FrameworkColorRole::kEmphasis)
                              : highlight_color_;
  if (!isEdited()) {
    highlight_color = colors.resolve(FrameworkColorRole::kContent);
    highlight_color.set_a(0x20);
  }
  roo::string_view text = value_;
  bool starred = isStarred();
  if (text.empty()) {
    // Show hint with half the opacity.
    text = hint_;
    color.set_a(color.a() / 2);
    color = AlphaBlend(canvas.bgcolor(), color);
    starred = false;
  }
  Padding padding = getPadding();

  // Determine our vertical position.
  YDim text_ymin = -font_.metrics().glyphYMax();
  XDim text_ymax = -font_.metrics().glyphYMin();
  YDim decorated_text_ymin = text_ymin;
  XDim decorated_text_ymax = text_ymax;
  switch (decoration_) {
    case UNDERLINE: {
      decorated_text_ymax += 6;
      break;
    }
    default: {
    }
  }
  Alignment padded = adjustAlignment(alignment_);
  int16_t offsetTop = padded.v().resolveOffset((YDim)0, height() - 1,
                                               decorated_text_ymin - text_ymin,
                                               decorated_text_ymax - text_ymin);
  int16_t available_width = width() - padding.left() - padding.right();
  int16_t advance_width;
  if (isEdited()) {
    // The metrics are captured in the editor.
    const auto& glyphs = editor().glyphs();
    advance_width = glyphs.empty() ? 0 : glyphs.back().advance();
    if (editor().cursor_position() == (int)editor().glyphs().size()) {
      // Leave two pixels for the cursor at the end of text.
      advance_width += 2;
    }
  } else if (!text.empty()) {
    // Calculate the glyph metrics now.
    if (starred) {
      // Calculate the bounds as-if the string is all-star.
      GlyphMetrics metrics;
      font_.getGlyphMetrics('*', FontLayout::kHorizontal, &metrics);
      roo_io::Utf8Decoder decoder(text);
      int length = 0;
      char32_t ignored;
      while (decoder.next(ignored)) {
        ++length;
      }
      advance_width = metrics.advance() * (length - 1);
    } else {
      // Calculate the bounds based on the actual string content.
      GlyphMetrics metrics = font_.getHorizontalStringMetrics(text);
      advance_width = metrics.advance();
    }
  } else {
    advance_width = 0;
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

  if (isInvalidated() && offsetTop > 0) {
    canvas.clearRect(0, 0, width() - 1, offsetTop - 1);
  }

  Canvas my_canvas = canvas;
  my_canvas.shift(0, offsetTop - text_ymin);
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
  my_canvas.drawObject(internal::SingleLineText(
      font_, {}, text_clip_box, text, isStarred() && !value_.empty(),
      editor().lastGlyphRecentlyEntered(), xoffset, highlight_xmin,
      highlight_xmax, color, highlight_color));

  int16_t min_y_undrawn = offsetTop + text_clip_box.height();
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

TextFieldEditor::~TextFieldEditor() { cancel(); }

void TextFieldEditor::edit(internal::TextEditTarget* target,
                           bool show_software_keyboard) {
  if (target == nullptr) { finish(false); return; }
  if (target_ == target) {
    application_.activateTextInput(*this);
    if (target_ != target) return;
    application_.setTextEditorKeyboardVisibility(show_software_keyboard);
    restartCursor();
    return;
  }
  internal::TextEditTarget* old = target_;
  if (old != nullptr) stopCursor(*old);
  last_glyph_hider_.cancel();
  last_glyph_recently_entered_ = false;
  target_ = target;
  draw_xoffset_ = 0;
  if (old != nullptr) old->notifyEditVisualChange();
  measure();
  // Publish the new session before callbacks. Application activation can end
  // another task's session and that callback may detach this target.
  application_.activateTextInput(*this);
  if (target_ == target) {
    application_.setTextEditorKeyboardVisibility(show_software_keyboard);
    restartCursor();
  }
  // Completion is terminal: callbacks may destroy targets or start a session.
  if (old != nullptr) old->onEditFinished(false);
}

bool TextFieldEditor::isEdited(const internal::TextEditTarget* target) const {
  return (target_ == target);
}

bool TextFieldEditor::targetInSubtree(const Widget& subtree) const {
  for (const Widget* current = target_ == nullptr ? nullptr : &target_->editWidget(); current != nullptr;
       current = current->parent()) {
    if (current == &subtree) return true;
  }
  return false;
}

void TextFieldEditor::setSelection(int16_t selection_begin,
                                   int16_t selection_end) {
  if (target_ == nullptr) return;
  if (selection_end > (int)glyphs_.size()) {
    selection_end = (int)glyphs_.size();
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
  selection_anchor_ = selection_begin;
  cursor_position_ = selection_end;
  target_->notifyEditVisualChange();
}

// Decode offsets independently of visual masking; retain byte-sized capacity
// so reveal/expiry never allocates for the active buffer.
void TextFieldEditor::measure() {
  const std::string& value = target_->textBuffer();
  glyphs_.resize(value.size());
  offsets_.resize(value.size());
  roo_io::Utf8Decoder decoder(value);
  char32_t rune;
  size_t count = 0;
  while (true) {
    size_t offset = (const char*)decoder.data() - value.data();
    if (!decoder.next(rune)) break;
    offsets_[count++] = offset;
  }
  offsets_.resize(count);
  glyphs_.resize(count);
  if (count != 0 && !target_->obscureText()) {
    target_->textFont().getHorizontalStringGlyphMetrics(
        value, glyphs_.data(), 0, count, target_->textFontOptions());
  } else if (count != 0) {
    GlyphMetrics mask;
    target_->textFont().getGlyphMetrics('*', FontLayout::kHorizontal, &mask);
    int16_t x = 0;
    for (size_t i = 0; i < count; ++i) {
      GlyphMetrics glyph = mask;
      if (i + 1 == count && last_glyph_recently_entered_) {
        roo_io::Utf8Decoder last(roo::string_view(value).substr(offsets_[i]));
        last.next(rune);
        target_->textFont().getGlyphMetrics(rune, FontLayout::kHorizontal, &glyph);
      }
      if (i != 0) x += target_->textFontOptions().trackingPx();
      glyphs_[i] = GlyphMetrics(glyph.glyphXMin() + x, glyph.glyphYMin(),
          glyph.glyphXMax() + x, glyph.glyphYMax(), glyph.advance() + x);
      x += glyph.advance();
    }
  }
  selection_begin_ = selection_end_ = selection_anchor_ = 0;
  cursor_position_ = count;
}

void TextFieldEditor::resetMetrics() {
  if (target_ == nullptr) return;
  last_glyph_recently_entered_ = false;
  last_glyph_hider_.cancel();
  draw_xoffset_ = 0;
  measure();
  target_->notifyEditVisualChange();
}

void TextFieldEditor::changed() {
  // No target access after the application hook, which can delete the field.
  target_->notifyEditVisualChange();
  target_->notifyTextChanged();
}

void TextFieldEditor::restartLastGlyphRecentlyEntered() {
  if (target_ == nullptr) return;
  // Note: need to check for empty as a special case, because we may
  // be showing the hint.
  if (target_->textBuffer().empty() ||
      static_cast<size_t>(cursor_position()) == glyphs_.size()) {
    last_glyph_recently_entered_ = true;
    last_glyph_hider_.scheduleAfter(kShowLastGlyphInterval);
  }
}

void TextFieldEditor::hideLastGlyph() {
  if (!last_glyph_recently_entered_) return;
  last_glyph_recently_entered_ = false;
  if (target_ != nullptr) refreshMetrics();
}

void TextFieldEditor::restartCursor() {
  if (target_ == nullptr) return;
  application_.context().animations().cancel(target_->editWidget(), 0);
  blinking_cursor_is_on_ = true;
  target_->notifyEditVisualChange();
  if (target_->editWidget().presentationState() != PresentationState::kPresented) return;
  AnimationSpec spec = AnimationSpec::CustomTime();
  spec.minimum_interval = kCursorBlinkInterval;
  application_.context().animations().start(target_->editWidget(), 0, spec);
}

void TextFieldEditor::stopCursor(internal::TextEditTarget& target) {
  application_.context().animations().cancel(target.editWidget(), 0);
  blinking_cursor_is_on_ = false;
}

void TextFieldEditor::applyCursorFrame(internal::TextEditTarget& target,
                                       const AnimationSample& sample) {
  if (target_ != &target) return;
  bool cursor_on =
      (sample.elapsed.inMillis() / kCursorBlinkInterval.inMillis()) % 2 == 0;
  if (cursor_on == blinking_cursor_is_on_) return;
  blinking_cursor_is_on_ = cursor_on;
  target.notifyEditVisualChange();
}

void TextFieldEditor::rune(uint32_t rune) {
  if (target_ == nullptr || rune < 0x20 ||
      (rune >= 0x7f && rune <= 0x9f) || rune > 0x10ffff ||
      (rune >= 0xd800 && rune <= 0xdfff) || rune == 0x2028 || rune == 0x2029) return;
  restartCursor();
  last_glyph_hider_.cancel();
  last_glyph_recently_entered_ = false;
  if (!has_selection()) restartLastGlyphRecentlyEntered();
  std::string& val = target_->textBuffer();
  size_t insert_at = cursor_position_ == (int)offsets_.size()
      ? val.size() : offsets_[cursor_position_];
  if (has_selection()) {
    // Delete the selected text, remove the selection, and set the cursor
    // where there was the selection.
    val.erase(val.begin() + offsets_[selection_begin_],
              selection_end_ == static_cast<int16_t>(offsets_.size())
                  ? val.end()
                  : val.begin() + offsets_[selection_end_]);
    insert_at = offsets_[selection_begin_];
    cursor_position_ = selection_begin_;
  }
  char encoded[4];
  int count = roo_io::WriteUtf8Char(encoded, rune);
  val.insert(insert_at, encoded, count);

  // Need to re-measure, because the glyphs use absolute coordinates, and
  // also, kerning makes it not trivial.
  int16_t saved_pos = cursor_position_ + 1;
  measure();
  cursor_position_ = saved_pos;
  changed();
}

void TextFieldEditor::finish(bool confirmed) {
  if (target_ == nullptr) return;
  internal::TextEditTarget* old = target_;
  last_glyph_recently_entered_ = false;
  last_glyph_hider_.cancel();
  stopCursor(*old);
  target_ = nullptr;
  application_.deactivateTextInput(*this);
  application_.setTextEditorKeyboardVisibility(false);
  old->notifyEditVisualChange();
  old->onEditFinished(confirmed);
}

void TextFieldEditor::enter() { finish(true); }
void TextFieldEditor::cancel() { finish(false); }

void TextFieldEditor::moveCursor(int16_t position, bool extend_selection) {
  if (target_ == nullptr) return;
  position = std::max<int16_t>(
      0, std::min<int16_t>(position, static_cast<int16_t>(glyphs_.size())));
  if (extend_selection) {
    if (!has_selection()) selection_anchor_ = cursor_position_;
    selection_begin_ = std::min(selection_anchor_, position);
    selection_end_ = std::max(selection_anchor_, position);
  } else {
    selection_begin_ = 0;
    selection_end_ = 0;
    selection_anchor_ = position;
  }
  cursor_position_ = position;
  restartCursor();
  target_->notifyEditVisualChange();
}

void TextFieldEditor::moveLeft(bool extend_selection) {
  if (!extend_selection && has_selection()) {
    moveCursor(selection_begin_, false);
  } else {
    moveCursor(cursor_position_ - 1, extend_selection);
  }
}

void TextFieldEditor::moveRight(bool extend_selection) {
  if (!extend_selection && has_selection()) {
    moveCursor(selection_end_, false);
  } else {
    moveCursor(cursor_position_ + 1, extend_selection);
  }
}

void TextFieldEditor::moveHome(bool extend_selection) {
  moveCursor(0, extend_selection);
}

void TextFieldEditor::moveEnd(bool extend_selection) {
  moveCursor(glyphs_.size(), extend_selection);
}

void TextFieldEditor::del() {
  if (target_ == nullptr) return;
  if (target_->textBuffer().empty()) return;
  last_glyph_recently_entered_ = false;
  restartCursor();
  if (has_selection()) {
    // Delete the selected text, remove the selection, and set the cursor
    // where there was the selection.
    std::string& val = target_->textBuffer();
    val.erase(val.begin() + offsets_[selection_begin_],
              selection_end_ == static_cast<int16_t>(offsets_.size())
                  ? val.end()
                  : val.begin() + offsets_[selection_end_]);
    // Need to re-measure, because the glyphs use absolute coordinates, and
    // also, kerning makes it not trivial.
    int16_t saved_pos = selection_begin_;
    measure();
    cursor_position_ = saved_pos;
    changed();
  } else if (cursor_position_ > 0) {
    std::string& val = target_->textBuffer();
    if (static_cast<size_t>(cursor_position_) == offsets_.size()) {
      val.erase(val.begin() + offsets_[cursor_position_ - 1], val.end());
    } else {
      val.erase(val.begin() + offsets_[cursor_position_ - 1],
                val.begin() + offsets_[cursor_position_]);
    }
    int16_t saved_pos = cursor_position_ - 1;
    measure();
    cursor_position_ = saved_pos;
    changed();
  }
}

void TextFieldEditor::forwardDelete() {
  if (target_ == nullptr || target_->textBuffer().empty()) return;
  if (has_selection()) {
    del();
    return;
  }
  if (cursor_position_ >= static_cast<int16_t>(offsets_.size())) return;
  last_glyph_recently_entered_ = false;
  restartCursor();
  std::string& val = target_->textBuffer();
  int16_t saved_pos = cursor_position_;
  val.erase(val.begin() + offsets_[cursor_position_],
            cursor_position_ + 1 == static_cast<int16_t>(offsets_.size())
                ? val.end()
                : val.begin() + offsets_[cursor_position_ + 1]);
  measure();
  cursor_position_ = saved_pos;
  selection_anchor_ = saved_pos;
  changed();
}

void TextField::onFocusChanged(bool focused) {
  Task* task = getTask();
  if (task == nullptr) return;
  TextFieldEditor& text_editor = task->textFieldEditor();
  if (focused) {
    // TextField::edit() may have already opened the software keyboard before
    // this field receives its first layout bounds. Do not hide that keyboard
    // when the deferred focus request succeeds.
    if (editable_ && !text_editor.isEdited(this)) {
      text_editor.edit(this, false);
    }
  } else if (isEdited()) {
    text_editor.edit(nullptr, false);
  }
}

TextFieldEditor& TextField::editor() {
  Task* task = getTask();
  CHECK_NOTNULL(task);
  return task->textFieldEditor();
}

const TextFieldEditor& TextField::editor() const {
  const Task* task = getTask();
  CHECK_NOTNULL(task);
  return task->textFieldEditor();
}

bool TextField::isEdited() const {
  const Task* task = getTask();
  return task != nullptr && task->textFieldEditor().isEdited(this);
}

void TextField::edit() {
  Task* task = getTask();
  if (!editable_ || task == nullptr) return;

  // A pointer- or programmatically-started edit must own the task's physical
  // keyboard focus as well as the software editor session. A just-attached
  // field has empty bounds until its first layout, so leave the editor active
  // in that case; onLayout() will claim focus once it becomes eligible.
  requestFocus();
  task->textFieldEditor().edit(this);
}

void TextField::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  (void)rect;
  if (isEdited() && !isFocused()) requestFocus();
}

void TextField::onAnimationFrame(AnimationTag tag,
                                 const AnimationSample& sample) {
  if (tag != kCaret) {
    BasicWidget::onAnimationFrame(tag, sample);
    return;
  }
  Task* task = getTask();
  if (task == nullptr) return;
  TextFieldEditor& text_editor = task->textFieldEditor();
  if (text_editor.isEdited(this)) {
    text_editor.applyCursorFrame(*this, sample);
  }
}

void TextField::setEditable(bool editable) {
  if (editable_ == editable) return;
  editable_ = editable;
  if (!editable_ && isEdited()) editor().edit(nullptr, false);
  setDirty();
}

bool TextField::onKeyEvent(const KeyEvent& event) {
  if (!editable_ ||
      (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat) ||
      getTask() == nullptr) {
    return false;
  }
  TextFieldEditor& text_editor = editor();
  const bool extend = (event.modifiers & kKeyModifierShift) != 0;
  switch (event.code) {
    case KeyCode::kCharacter:
      if (event.rune == 0) return true;
      text_editor.edit(this, false);
      text_editor.rune(event.rune);
      return true;
    case KeyCode::kSpace:
      text_editor.edit(this, false);
      text_editor.rune(U' ');
      return true;
    case KeyCode::kEnter:
      if (!isEdited()) text_editor.edit(this, false);
      text_editor.enter();
      return true;
    case KeyCode::kEscape:
    case KeyCode::kBack:
      text_editor.cancel();
      // Preserve task-level Back/Escape semantics after ending this edit.
      return false;
    case KeyCode::kBackspace:
      text_editor.del();
      return true;
    case KeyCode::kDelete:
      text_editor.forwardDelete();
      return true;
    case KeyCode::kLeft:
      text_editor.moveLeft(extend);
      return true;
    case KeyCode::kRight:
      text_editor.moveRight(extend);
      return true;
    case KeyCode::kHome:
      text_editor.moveHome(extend);
      return true;
    case KeyCode::kEnd:
      text_editor.moveEnd(extend);
      return true;
    default:
      return false;
  }
}

}  // namespace roo_windows

namespace roo_windows {
void TextFieldEditor::refreshMetrics() {
  if (target_ == nullptr) return;
  int16_t cursor = cursor_position_, begin = selection_begin_, end = selection_end_;
  int16_t anchor = selection_anchor_, offset = draw_xoffset_;
  measure();
  cursor_position_ = std::min<int16_t>(cursor, glyphs_.size());
  selection_begin_ = std::min<int16_t>(begin, glyphs_.size());
  selection_end_ = std::min<int16_t>(end, glyphs_.size());
  selection_anchor_ = std::min<int16_t>(anchor, glyphs_.size());
  draw_xoffset_ = offset;
  target_->notifyEditVisualChange();
}
void TextFieldEditor::ensureCursorVisible(int16_t width) {
  int16_t caret = cursor_position_ == 0 ? 0 : glyphs_[cursor_position_ - 1].advance();
  width = std::max<int16_t>(2, width);
  if (caret + draw_xoffset_ > width - 2) draw_xoffset_ = width - 2 - caret;
  if (caret + draw_xoffset_ < 0) draw_xoffset_ = -caret;
}
}
