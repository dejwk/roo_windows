#include "roo_windows/material3/text_field/text_field.h"

#include <algorithm>

#include "roo_display/ui/text_label.h"
#include "roo_icons/filled/alert.h"
#include "roo_windows/core/task.h"
#include "roo_windows/internal/single_line_text.h"

namespace roo_windows::material3 {
namespace {
using namespace roo_display;
struct Tokens {
  int16_t height = Scaled(56), pad = Scaled(16), icon_pad = Scaled(12);
  int16_t gap = Scaled(16), assist_gap = Scaled(4), radius = Scaled(4);
  int16_t notch = Scaled(4), idle_stroke = std::max(1, Scaled(1));
  int16_t focus_stroke = std::max(1, Scaled(2));
};
const Tokens kTokens;
const MonoIcon kErrorIcon = SCALED_ROO_ICON(filled, alert_error);
Rect Empty() { return Rect(0, 0, -1, -1); }
int16_t TextWidth(roo::string_view text, const TextStyle& style) {
  return style.font()
      .getHorizontalStringMetrics(text, style.fontOptions())
      .advance();
}
Color Opacity(Color color, int alpha, Color background) {
  color.set_a(alpha);
  return AlphaBlend(background, color);
}
}  // namespace
struct TextField::Slots {
  Rect container, label, viewport, prefix, suffix, leading, trailing, assist;
};
TextField::TextField(ApplicationContext& context, roo::string_view label,
                     TextFieldVariant variant)
    : BasicSurfaceWidget(context),
      label_(label),
      flags_(variant == TextFieldVariant::kOutlined ? kOutlined : 0) {}
TextField::~TextField() {
  // Stop the editor before the edit-target base and owned value are destroyed.
  if (isEdited()) getTask()->textFieldEditor().cancel();
}
TextFieldVariant TextField::variant() const {
  return flags_ & kOutlined ? TextFieldVariant::kOutlined
                            : TextFieldVariant::kFilled;
}
void TextField::setVariant(TextFieldVariant value) {
  if (variant() == value) return;
  flags_ ^= kOutlined;
  geometryChanged();
}
LayoutDirection TextField::layoutDirection() const {
  return flags_ & kRtl ? LayoutDirection::kRightToLeft
                       : LayoutDirection::kLeftToRight;
}
void TextField::setLayoutDirection(LayoutDirection value) {
  if (layoutDirection() == value) return;
  flags_ ^= kRtl;
  geometryChanged();
}
void TextField::geometryChanged() {
  invalidateInterior();
  requestLayout();
  updateScroll();
}
void TextField::setText(std::string value) {
  if (value_ == value) return;
  value_ = std::move(value);
  if (isEdited()) getTask()->textFieldEditor().resetMetrics();
  invalidateInterior();
  onTextChanged();
}
void TextField::setLabel(roo::string_view value) {
  if (label_ == value) return;
  label_ = value;
  geometryChanged();
}
void TextField::setPrefixText(roo::string_view value) {
  if (prefix_ == value) return;
  prefix_ = value;
  geometryChanged();
}
void TextField::setSuffixText(roo::string_view value) {
  if (suffix_ == value) return;
  suffix_ = value;
  geometryChanged();
}
roo::string_view TextField::assistiveText() const {
  return hasError() ? error_ : supporting_;
}
void TextField::setSupportingText(roo::string_view value) {
  if (supporting_ == value) return;
  bool had_row = !assistiveText().empty();
  supporting_ = value;
  if (hasError()) return;
  if (had_row != !assistiveText().empty())
    geometryChanged();
  else
    setDirty(slots().assist);
}
void TextField::setErrorText(roo::string_view value) {
  if (hasError() && error_ == value) return;
  bool had_error = hasError(), had_row = !assistiveText().empty();
  error_ = value;
  flags_ |= kError;
  if (!had_error || had_row != !assistiveText().empty())
    geometryChanged();
  else
    setDirty(slots().assist);
}
void TextField::clearError() {
  if (!hasError()) return;
  flags_ &= ~kError;
  geometryChanged();
}
void TextField::setLeadingIcon(const MonoIcon* value) {
  if (leading_ == value) return;
  leading_ = value;
  geometryChanged();
}
void TextField::setTrailingIcon(const MonoIcon* value) {
  if (trailing_ == value) return;
  trailing_ = value;
  geometryChanged();
}
const MonoIcon* TextField::effectiveTrailingIcon() const {
  return trailing_ != nullptr ? trailing_ : hasError() ? &kErrorIcon : nullptr;
}
void TextField::setReadOnly(bool value) {
  if (readOnly() == value) return;
  flags_ ^= kReadOnly;
  invalidateInterior();
  if (value && isEdited()) getTask()->textFieldEditor().cancel();
}
bool TextField::floated() const {
  return !value_.empty() || isFocused() || isEdited();
}
Dimensions TextField::getSuggestedMinimumDimensions() const {
  int height = kTokens.height;
  if (flags_ & kOutlined) height += text_style_body_small().lineHeight() / 2;
  if (!assistiveText().empty())
    height += kTokens.assist_gap + text_style_body_small().lineHeight();
  return Dimensions(Scaled(120), height);
}
PreferredSize TextField::getPreferredSize() const {
  return PreferredSize(
      PreferredSize::MatchParentWidth(),
      PreferredSize::ExactHeight(getSuggestedMinimumDimensions().height()));
}
TextField::Slots TextField::slots() const {
  const TextStyle& body = text_style_body_large();
  const TextStyle& small = text_style_body_small();
  int top = flags_ & kOutlined ? small.lineHeight() / 2 : 0;
  Slots s;
  s.container = Rect(0, top, width() - 1, top + kTokens.height - 1);
  int left = leading_ ? kTokens.icon_pad : kTokens.pad;
  int right =
      width() - (effectiveTrailingIcon() ? kTokens.icon_pad : kTokens.pad);
  int iy = top + (kTokens.height - ROO_WINDOWS_ICON_SIZE) / 2;
  s.leading = leading_ ? Rect(left, iy,
                              std::min(right, left + ROO_WINDOWS_ICON_SIZE) - 1,
                              iy + ROO_WINDOWS_ICON_SIZE - 1)
                       : Empty();
  if (leading_)
    left = std::min(right, left + ROO_WINDOWS_ICON_SIZE + kTokens.gap);
  s.trailing = effectiveTrailingIcon()
                   ? Rect(std::max(left, right - ROO_WINDOWS_ICON_SIZE), iy,
                          right - 1, iy + ROO_WINDOWS_ICON_SIZE - 1)
                   : Empty();
  if (effectiveTrailingIcon())
    right = std::max(left, right - ROO_WINDOWS_ICON_SIZE - kTokens.gap);
  bool floating = floated();
  int ty = top + (kTokens.height - body.lineHeight()) / 2;
  if (floating && !(flags_ & kOutlined))
    ty = top + (kTokens.height - body.lineHeight() - small.lineHeight()) / 2 +
         small.lineHeight();
  if (floating) {
    int ly = flags_ & kOutlined ? 0 : ty - small.lineHeight();
    int lw = std::min(std::max(0, right - left), (int)TextWidth(label_, small));
    s.label = Rect(left, ly, left + lw - 1, ly + small.lineHeight() - 1);
  } else
    s.label = Rect(left, ty, right - 1, ty + body.lineHeight() - 1);
  int pw = floating ? std::min(std::max(0, right - left),
                               (int)TextWidth(prefix_, body))
                    : 0;
  s.prefix = Rect(left, ty, left + pw - 1, ty + body.lineHeight() - 1);
  left += pw;
  int sw = floating ? std::min(std::max(0, right - left),
                               (int)TextWidth(suffix_, body))
                    : 0;
  s.suffix = Rect(right - sw, ty, right - 1, ty + body.lineHeight() - 1);
  right -= sw;
  s.viewport = Rect(left, ty, right - 1, ty + body.lineHeight() - 1);
  s.assist =
      Rect(kTokens.pad, top + kTokens.height + kTokens.assist_gap,
           width() - kTokens.pad - 1,
           top + kTokens.height + kTokens.assist_gap + small.lineHeight() - 1);
  if (flags_ & kRtl) {
    auto mirror = [&](Rect& r) {
      r = Rect(width() - 1 - r.xMax(), r.yMin(), width() - 1 - r.xMin(),
               r.yMax());
    };
    mirror(s.leading);
    mirror(s.trailing);
    mirror(s.label);
    mirror(s.prefix);
    mirror(s.suffix);
    mirror(s.viewport);
  }
  auto clip = [&](Rect& r) { r = Rect::Intersect(r, bounds()); };
  clip(s.container);
  clip(s.label);
  clip(s.viewport);
  clip(s.prefix);
  clip(s.suffix);
  clip(s.leading);
  clip(s.trailing);
  clip(s.assist);
  return s;
}
void TextField::paint(PaintContext& ctx) const {
  using namespace roo_display;
  const auto& colors = theme().material3Theme().color;
  const Slots s = slots();
  Color ancestor = ctx.bgcolor();
  bool outlined = flags_ & kOutlined;
  Color fill = outlined ? ancestor : colors.surfaceContainerHighest;
  if (!isEnabled() && !outlined)
    fill = Opacity(colors.onSurface, 10, ancestor);
  else if (isPressed())
    fill = Opacity(colors.onSurface, 26, fill);
  else if (isHover())
    fill = Opacity(colors.onSurface, 20, fill);
  Color input =
      isEnabled() ? colors.onSurface : Opacity(colors.onSurface, 97, fill);
  Color secondary = isEnabled() ? colors.onSurfaceVariant : input;
  Color accent = !isEnabled()                  ? input
                 : hasError()                  ? colors.error
                 : (isFocused() || isEdited()) ? colors.primary
                 : outlined                    ? colors.outline
                                               : colors.onSurfaceVariant;
  Color label_color =
      hasError() || isFocused() || isEdited() ? accent : secondary;
  auto text = [&](roo::string_view value, const TextStyle& style, Rect rect,
                  Color color, Color bg) {
    if (rect.empty()) return;
    PaintContext part = ctx.clipped(rect);
    part.setBgcolor(bg);
    part.drawTiled(
        StringViewLabel(value, style.font(), color, style.fontOptions()), rect,
        (flags_ & kRtl ? kRight : kLeft) |
            kBaseline.toTop().shiftBy(style.baselineOffset()));
    ctx.addExclusion(rect);
  };
  const TextStyle& body = text_style_body_large();
  const TextStyle& small = text_style_body_small();
  if (floated()) {
    Rect label = s.label;
    if (outlined && !label.empty())
      label = Rect::Intersect(bounds(),
                              Rect(label.xMin() - kTokens.notch, label.yMin(),
                                   label.xMax() + kTokens.notch, label.yMax()));
    // Notch background and label settle together before the outline.
    if (outlined && !label.empty()) {
      PaintContext part = ctx.clipped(label);
      part.setBgcolor(ancestor);
      part.drawTiled(
          StringViewLabel(label_, small.font(), label_color,
                          small.fontOptions()),
          label, kCenter | kBaseline.toTop().shiftBy(small.baselineOffset()));
      ctx.addExclusion(label);
    } else
      text(label_, small, label, label_color, fill);
    text(prefix_, body, s.prefix, secondary, fill);
    text(suffix_, body, s.suffix, secondary, fill);
    if (!s.viewport.empty()) {
      int16_t begin = 0, end = -1, offset = 0;
      bool recent = false;
      if (isEdited()) {
        const auto& editor = getTask()->textFieldEditor();
        auto advance = [&](int pos) {
          return pos == 0 ? 0 : editor.glyphs()[pos - 1].advance();
        };
        offset = editor.draw_xoffset();
        recent = editor.lastGlyphRecentlyEntered();
        if (editor.has_selection()) {
          begin = advance(editor.selection_begin());
          end = advance(editor.selection_end()) - 1;
        } else if (editor.isBlinkingCursorNowOn()) {
          begin = advance(editor.cursor_position());
          end = begin + 1;
        }
      }
      PaintContext part =
          ctx.clipped(s.viewport)
              .translated(s.viewport.xMin(),
                          s.viewport.yMin() + body.baselineOffset());
      part.setBgcolor(fill);
      Color highlight =
          isEdited() && getTask()->textFieldEditor().has_selection()
              ? Opacity(colors.primary, 64, fill)
              : accent;
      part.drawObject(internal::SingleLineText(
          body.font(), body.fontOptions(),
          Box(0, -body.baselineOffset(), s.viewport.width() - 1,
              body.lineHeight() - body.baselineOffset() - 1),
          value_, obscureText(), recent, offset, begin, end, input, highlight));
      ctx.addExclusion(s.viewport);
    }
  } else
    text(label_, body, s.label, label_color, fill);
  auto icon = [&](const MonoIcon* source, Rect rect, Color color) {
    if (source == nullptr || rect.empty()) return;
    MonoIcon icon = *source;
    icon.color_mode().setColor(color);
    PaintContext part = ctx.clipped(rect);
    part.setBgcolor(fill);
    part.drawTiled(icon, rect, kCenter | kMiddle);
    ctx.addExclusion(rect);
  };
  icon(leading_, s.leading, secondary);
  icon(effectiveTrailingIcon(), s.trailing, hasError() ? accent : secondary);
  // Assistive ellipsis borrows a UTF-8 prefix. Dots occupy their own
  // final-color region, avoiding a concatenation allocation.
  roo::string_view assist = assistiveText();
  if (!assist.empty() && !s.assist.empty()) {
    int available = s.assist.width();
    if (TextWidth(assist, small) <= available)
      text(assist, small, s.assist, hasError() ? accent : secondary, ancestor);
    else {
      roo::string_view dots = "...";
      while (!dots.empty() && TextWidth(dots, small) > available)
        dots.remove_suffix(1);
      int dw = TextWidth(dots, small);
      while (!assist.empty() && TextWidth(assist, small) > available - dw) {
        size_t n = assist.size() - 1;
        while (n > 0 && (static_cast<unsigned char>(assist[n]) & 0xc0) == 0x80)
          --n;
        assist = assist.substr(0, n);
      }
      Rect body_rect = s.assist, dots_rect = s.assist;
      if (flags_ & kRtl) {
        dots_rect = Rect(s.assist.xMin(), s.assist.yMin(),
                         s.assist.xMin() + dw - 1, s.assist.yMax());
        body_rect = Rect(dots_rect.xMax() + 1, s.assist.yMin(), s.assist.xMax(),
                         s.assist.yMax());
      } else {
        dots_rect = Rect(s.assist.xMax() - dw + 1, s.assist.yMin(),
                         s.assist.xMax(), s.assist.yMax());
        body_rect = Rect(s.assist.xMin(), s.assist.yMin(), dots_rect.xMin() - 1,
                         s.assist.yMax());
      }
      text(assist, small, body_rect, hasError() ? accent : secondary, ancestor);
      text(dots, small, dots_rect, hasError() ? accent : secondary, ancestor);
    }
  }
  int stroke = isEnabled() && (isFocused() || isEdited()) ? kTokens.focus_stroke
                                                          : kTokens.idle_stroke;
  if (!outlined && !s.container.empty()) {
    Rect indicator(0, s.container.yMax() - stroke + 1, width() - 1,
                   s.container.yMax());
    ctx.fillRect(indicator, accent);
    ctx.addExclusion(indicator);
  }
  PaintDecoration decoration;
  decoration.bounds = s.container;
  decoration.background = fill;
  uint8_t radius = std::min<int>(kTokens.radius, std::max<int>(0, width() / 2));
  decoration.corner_radii = {radius, radius,
                             static_cast<uint8_t>(outlined ? radius : 0),
                             static_cast<uint8_t>(outlined ? radius : 0)};
  decoration.outline_width = SmallNumber(outlined ? stroke : 0);
  decoration.outline_color = accent;
  if (!s.container.empty()) ctx.addDecoration(decoration);
  // Resolve the remaining ancestor/background/decoration pixels in one pass.
  ctx.clear();
}

bool TextField::isEdited() const {
  const Task* task = getTask();
  return task != nullptr && task->textFieldEditor().isEdited(this);
}
void TextField::startEditing(bool show_keyboard) {
  Task* task = getTask();
  if (!isEnabled() || readOnly() || task == nullptr) return;
  requestFocus();
  task->textFieldEditor().edit(this, show_keyboard);
}
void TextField::edit() { startEditing(true); }
void TextField::onClicked() {
  uint8_t tapped = flags_ & (kLeadingTap | kTrailingTap);
  flags_ &= ~(kLeadingTap | kTrailingTap);
  if (!isEnabled()) return;
  if (tapped == kLeadingTap && onLeadingAffordanceClicked()) return;
  if (tapped == kTrailingTap && onTrailingAffordanceClicked()) return;
  if (!readOnly()) edit();
  Widget::onClicked();
}
void TextField::onSingleTapUp(XDim x, YDim y) {
  Slots s = slots();
  if (!s.container.contains(x, y)) return;
  flags_ &= ~(kLeadingTap | kTrailingTap);
  auto hit = [&](Rect r) {
    return !r.empty() &&
           Rect::Intersect(s.container,
                           Rect(r.xMin() - Scaled(8), s.container.yMin(),
                                r.xMax() + Scaled(8), s.container.yMax()))
               .contains(x, y);
  };
  if (hit(s.leading))
    flags_ |= kLeadingTap;
  else if (hit(s.trailing))
    flags_ |= kTrailingTap;
  Widget::onSingleTapUp(x, y);
}
void TextField::onCancel() {
  flags_ &= ~(kLeadingTap | kTrailingTap | kActivationKey);
  Widget::onCancel();
}
void TextField::onFocusChanged(bool focused) {
  invalidateInterior();
  if (!focused) {
    flags_ &= ~kActivationKey;
    if (isEdited()) getTask()->textFieldEditor().cancel();
  }
}
void TextField::onLayout(bool changed, const Rect& rect) {
  if (isEdited() && !isFocused()) requestFocus();
  updateScroll();
}
void TextField::updateScroll() {
  if (isEdited())
    getTask()->textFieldEditor().ensureCursorVisible(slots().viewport.width());
}
void TextField::notifyEditVisualChange() {
  bool edited = isEdited();
  bool was_edited = flags_ & kLastEdited;
  if (edited)
    flags_ |= kLastEdited;
  else
    flags_ &= ~kLastEdited;
  updateScroll();
  if (edited != was_edited)
    invalidateInterior();
  else
    setDirty(slots().viewport);
}
void TextField::notifyTextChanged() {
  invalidateInterior();
  onTextChanged();
}
void TextField::maskingChanged() {
  if (isEdited()) getTask()->textFieldEditor().refreshMetrics();
  invalidateInterior();
}
void TextField::notifyStateChanged(uint16_t diff) {
  BasicSurfaceWidget::notifyStateChanged(diff);
  if (!isEnabled() && isEdited()) getTask()->textFieldEditor().cancel();
}
void TextField::onAnimationFrame(AnimationTag tag,
                                 const AnimationSample& sample) {
  if (tag == 0 && isEdited())
    getTask()->textFieldEditor().applyCursorFrame(*this, sample);
  else
    BasicSurfaceWidget::onAnimationFrame(tag, sample);
}
bool TextField::onKeyEvent(const KeyEvent& event) {
  if (!isEnabled()) return false;
  bool activation =
      event.code == KeyCode::kEnter || event.code == KeyCode::kSpace;
  if (event.phase == KeyPhase::kUp) {
    if (activation && (flags_ & kActivationKey)) {
      flags_ &= ~kActivationKey;
      return true;
    }
    return false;
  }
  if (event.phase != KeyPhase::kDown && event.phase != KeyPhase::kRepeat)
    return false;
  if (!isEdited()) {
    if (!activation) return false;
    if (event.phase == KeyPhase::kDown) {
      flags_ |= kActivationKey;
      if (readOnly())
        Widget::onClicked();
      else
        startEditing(false);
    }
    return true;
  }
  if (activation && (flags_ & kActivationKey)) return true;
  auto& editor = getTask()->textFieldEditor();
  bool shift = event.modifiers & kKeyModifierShift;
  switch (event.code) {
    case KeyCode::kCharacter:
      editor.rune(event.rune);
      return true;
    case KeyCode::kSpace:
      editor.rune(U' ');
      return true;
    case KeyCode::kEnter:
      flags_ |= kActivationKey;
      editor.enter();
      return true;
    case KeyCode::kEscape:
    case KeyCode::kBack:
      editor.cancel();
      return false;
    case KeyCode::kBackspace:
      editor.del();
      return true;
    case KeyCode::kDelete:
      editor.forwardDelete();
      return true;
    case KeyCode::kLeft:
      editor.moveLeft(shift);
      return true;
    case KeyCode::kRight:
      editor.moveRight(shift);
      return true;
    case KeyCode::kHome:
      editor.moveHome(shift);
      return true;
    case KeyCode::kEnd:
      editor.moveEnd(shift);
      return true;
    default:
      return false;
  }
}
}  // namespace roo_windows::material3
