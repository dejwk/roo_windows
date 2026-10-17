
#include "roo_windows/material2/text_field.h"

#include <algorithm>

#include "roo_backport/string_view.h"
#include "roo_display/ui/text_label.h"
#include "roo_icons/filled/action.h"
#include "roo_io/text/unicode.h"
#include "roo_windows/config.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/task.h"
#include "roo_windows/internal/single_line_text.h"

namespace roo_windows::material2 {

using namespace roo_display;

void VisibilityToggle::paint(PaintContext& ctx) const {
  Pictogram icon(isOn() ? SCALED_ROO_ICON(filled, action_visibility)
                        : SCALED_ROO_ICON(filled, action_visibility_off));
  Color color = defaultColor();
  icon.color_mode().setColor(color);
  ctx.drawTiled(icon, bounds(), kCenter | kMiddle, isInvalidated());
}

void TextField::paint(PaintContext& ctx) const {
  const Canvas& canvas = ctx.canvas();
  const Theme& theme = parent()->theme();
  Color color = text_color_.a() == 0 ? theme.material2Theme().color.onSurface
                                     : text_color_;
  Color highlight_color = highlight_color_.a() == 0
                              ? theme.material2Theme().color.primary
                              : highlight_color_;
  if (!isEdited()) {
    highlight_color = theme.material2Theme().color.onSurface;
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
    my_canvas.clearRect(width() - padding.right(), text_ymin, width() - 1,
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
      canvas.clearRect(width() - padding.right(), min_y_undrawn + 3,
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
    Widget::onAnimationFrame(tag, sample);
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

}  // namespace roo_windows::material2
