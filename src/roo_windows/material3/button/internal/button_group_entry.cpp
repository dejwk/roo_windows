#include "roo_windows/material3/button/internal/button_group_entry.h"

#include <algorithm>
#include <climits>
#include <cmath>

#include "roo_display/ui/text_label.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/button/internal/button_group_base.h"
#include "roo_windows/material3/button/internal/icon_appearance.h"
#include "roo_windows/material3/button/internal/icon_geometry.h"
#include "roo_windows/material3/internal/density.h"
#include "roo_windows/material3/typography.h"
namespace roo_windows::material3::internal {
namespace {
// Match button alignment including painted ink outside text advance bounds.
class GroupLabel : public roo_display::StringViewLabel {
 public:
  using StringViewLabel::StringViewLabel;
  roo_display::Box anchorExtents() const override {
    roo_display::Box original = StringViewLabel::anchorExtents();
    roo_display::Box horizontal = ButtonTextHorizontalExtents(metrics());
    return {horizontal.xMin(), original.yMin(), horizontal.xMax(),
            original.yMax()};
  }
};
// Converts the contained style encoding to the existing momentary family.
ButtonVariant Variant(uint8_t style) {
  return static_cast<ButtonVariant>(style + 1);
}
}  // namespace
ButtonGroupEntry::ButtonGroupEntry(ApplicationContext& context,
                                   GroupStyle style)
    : SurfaceWidget(context), style_(static_cast<uint8_t>(style)) {}
bool ButtonGroupEntry::effectivelyEnabled() const {
  return isEnabled() && (group_ == nullptr || group_->isEnabled());
}
bool ButtonGroupEntry::eligible() const {
  if (group_ == nullptr || presentationState() != PresentationState::kPresented)
    return false;
  for (const Widget* ancestor = this; ancestor != nullptr;
       ancestor = ancestor->parent()) {
    if (!ancestor->isEnabled() || !ancestor->isVisible()) return false;
  }
  return true;
}
bool ButtonGroupEntry::isClickable() const {
  return group_ != nullptr && effectivelyEnabled();
}
int ButtonGroupEntry::outlineInset() const {
  return static_cast<GroupStyle>(style_) == GroupStyle::kOutlined
             ? Scaled(SmallNumber(1)).ceil()
             : 0;
}
Dimensions ButtonGroupEntry::entryDimensions(int16_t* content_width) const {
  ButtonSize size = group_ == nullptr ? ButtonSize::kSmall : group_->size();
  int8_t level = ResolveDensityLevel(theme().material3Theme().density);
  ButtonContentMetrics content =
      ResolveButtonContentMetrics(contentLabel(), contentIcon(), size, level);
  Padding padding = ResolveButtonPadding(size, SmallButtonPadding::kReduced,
                                         content.content_height, level);
  int32_t outline = 2 * outlineInset();
  int32_t width =
      iconOnly()
          ? std::max<int>(IconContainerWidth(size, IconButtonWidth::kUniform),
                          content.icon_slot_width) +
                outline
          : content.content_width + padding.left() + padding.right() + outline;
  int32_t height =
      content.content_height + padding.top() + padding.bottom() + outline;
  CHECK_LE(width, INT16_MAX);
  CHECK_LE(height, INT16_MAX);
  CHECK_GE(width, 0);
  CHECK_GE(height, 0);
  if (content_width != nullptr) {
    CHECK_LE(content.content_width + outline, INT16_MAX);
    *content_width = content.content_width + outline;
  }
  return {static_cast<int16_t>(width), height};
}
Dimensions ButtonGroupEntry::onMeasure(WidthSpec width, HeightSpec height) {
  Dimensions natural = entryDimensions();
  return {width.resolveSize(natural.width()),
          height.resolveSize(natural.height())};
}
PreferredSize ButtonGroupEntry::getPreferredSize() const {
  return {PreferredSize::WrapContentWidth(),
          PreferredSize::WrapContentHeight()};
}
ColorToken ButtonGroupEntry::containerRole() const {
  if (group_ != nullptr && group_->connected())
    return ResolveConnectedContainerRole(
        theme(), static_cast<GroupStyle>(style_),
        group_->selected(group_->indexOf(*this)));
  return ResolveButtonContainerRole(theme(), Variant(style_));
}
Color ButtonGroupEntry::background() const {
  if (group_ != nullptr && group_->connected())
    return ResolveConnectedAppearance(theme(), static_cast<GroupStyle>(style_),
                                      group_->selected(group_->indexOf(*this)),
                                      effectivelyEnabled())
        .container;
  return ResolveButtonAppearance(theme(), Variant(style_), effectivelyEnabled())
      .container;
}
Color ButtonGroupEntry::getOutlineColor() const {
  if (group_ != nullptr && group_->connected())
    return ResolveConnectedAppearance(theme(), static_cast<GroupStyle>(style_),
                                      group_->selected(group_->indexOf(*this)),
                                      effectivelyEnabled())
        .outline;
  return ResolveButtonAppearance(theme(), Variant(style_), effectivelyEnabled())
      .outline;
}
BorderStyle ButtonGroupEntry::resolveBorder(bool selected, bool pressed,
                                            float progress,
                                            bool enabled) const {
  ButtonSize size = group_ == nullptr ? ButtonSize::kSmall : group_->size();
  ButtonShape shape = group_ == nullptr ? ButtonShape::kRound : group_->shape();
  bool morph =
      group_ != nullptr && group_->shapeMorph() != ButtonShapeMorph::kDisabled;
  if (!enabled) {
    pressed = false;
    progress = -1;
  }
  if (group_ != nullptr && group_->connected()) {
    bool first = true;
    bool last = true;
    bool seen = false;
    for (ButtonGroupEntry* entry : group_->entries_) {
      if (entry->isGone()) continue;
      if (entry == this)
        seen = true;
      else if (seen)
        last = false;
      else
        first = false;
    }
    float emphasis = !morph          ? 0
                     : progress >= 0 ? std::min(1.0f, 3 * progress)
                     : pressed       ? 1
                                     : 0;
    return ResolveConnectedBorder(
        size, shape, group_->layoutDirection(), first, last, selected, emphasis,
        {width(), height()},
        static_cast<GroupStyle>(style_) == GroupStyle::kOutlined && !selected
            ? Scaled(SmallNumber(1))
            : SmallNumber(0));
  }
  if (iconOnly()) {
    const IconGeometryTokens& geometry = IconGeometryFor(size);
    int half = std::max(0, std::min<int>(width(), height()) / 2);
    int resting = shape == ButtonShape::kRound
                      ? half
                      : Scaled(geometry.square_corner_radius_dp);
    float emphasis = !morph          ? 0
                     : progress >= 0 ? std::min(1.0f, 3 * progress)
                     : pressed       ? 1
                                     : 0;
    int radius = std::lround(
        resting +
        (Scaled(geometry.pressed_corner_radius_dp) - resting) * emphasis);
    return BorderStyle(std::min({radius, half, 255}),
                       static_cast<GroupStyle>(style_) == GroupStyle::kOutlined
                           ? Scaled(SmallNumber(1))
                           : SmallNumber(0));
  }
  return ResolveButtonBorderStyle(size, shape, Variant(style_), morph, pressed,
                                  progress, {width(), height()});
}
BorderStyle ButtonGroupEntry::getBorderStyle() const {
  const ClickAnimation* animation = getClickAnimation();
  return resolveBorder(
      group_ != nullptr && group_->selected(group_->indexOf(*this)),
      isPressed(), animation == nullptr ? -1 : animation->progress(),
      effectivelyEnabled());
}
uint8_t ButtonGroupEntry::getElevation() const {
  return ResolveButtonElevation(Variant(style_), effectivelyEnabled(),
                                isPressed());
}
Widget::OverlayType ButtonGroupEntry::getOverlayType() const {
  return effectivelyEnabled() ? OVERLAY_AREA : OVERLAY_NONE;
}
ColorToken ButtonGroupEntry::effectiveOverlayColorRole() const {
  ColorToken role = containerRole();
  if (static_cast<GroupStyle>(style_) == GroupStyle::kElevated ||
      role == ColorToken::kNone ||
      (group_ != nullptr && group_->connected() &&
       static_cast<GroupStyle>(style_) == GroupStyle::kFilled &&
       !group_->selected(group_->indexOf(*this))))
    return ColorToken::kSurfaceVariant;
  return role;
}
bool ButtonGroupEntry::usesHighlighterColor() const {
  return static_cast<GroupStyle>(style_) == GroupStyle::kElevated &&
         (group_ == nullptr || !group_->selected(group_->indexOf(*this)));
}
void ButtonGroupEntry::contentChanged() {
  invalidateInterior();
  requestLayout();
}
void ButtonGroupEntry::changeStyle(GroupStyle style) {
  if (style_ == static_cast<uint8_t>(style)) return;
  BorderStyle previous = getBorderStyle();
  uint8_t elevation = getElevation();
  style_ = static_cast<uint8_t>(style);
  borderStyleChanged(previous);
  elevationChanged(std::max(elevation, getElevation()));
  contentChanged();
}
void ButtonGroupEntry::selectionChanged(bool previous_selected) {
  const ClickAnimation* animation = getClickAnimation();
  borderStyleChanged(resolveBorder(
      previous_selected, isPressed(),
      animation == nullptr ? -1 : animation->progress(), effectivelyEnabled()));
  invalidateInterior();
}
void ButtonGroupEntry::groupEnabledChanged(bool previous_effectively_enabled) {
  const ClickAnimation* animation = getClickAnimation();
  borderStyleChanged(resolveBorder(
      group_ != nullptr && group_->selected(group_->indexOf(*this)),
      isPressed(), animation == nullptr ? -1 : animation->progress(),
      previous_effectively_enabled));
  // The old elevated shadow must be restored even when disabling removes it.
  elevationChanged(std::max(
      getElevation(),
      ResolveButtonElevation(Variant(style_), previous_effectively_enabled,
                             isPressed())));
  invalidateInterior();
}
void ButtonGroupEntry::notifyStateChanged(uint16_t diff) {
  if (group_ != nullptr && group_->connected() &&
      (diff & kWidgetSelected) != 0) {
    bool canonical = group_->selected(group_->indexOf(*this));
    if (isSelected() != canonical) {
      Widget::setSelected(canonical);
      return;
    }
  }
  if ((diff & kWidgetPressed) != 0) {
    const ClickAnimation* animation = getClickAnimation();
    borderStyleChanged(resolveBorder(
        group_ != nullptr && group_->selected(group_->indexOf(*this)),
        !isPressed(), animation == nullptr ? -1 : animation->progress(),
        effectivelyEnabled()));
  }
  if ((diff & kWidgetEnabled) != 0) {
    groupEnabledChanged(!isEnabled() &&
                        (group_ == nullptr || group_->isEnabled()));
    if (!isEnabled()) Widget::onCancel();
  }
  SurfaceWidget::notifyStateChanged(diff);
  if (group_ != nullptr && (diff & (kWidgetPressed | kWidgetEnabled)) != 0)
    group_->entryFeedbackChanged();
}
void ButtonGroupEntry::onClickAnimationChanged(float previous_progress) {
  borderStyleChanged(resolveBorder(
      group_ != nullptr && group_->selected(group_->indexOf(*this)),
      isPressed(), previous_progress, effectivelyEnabled()));
  if (group_ != nullptr) group_->entryFeedbackChanged();
}
ClickActivationPolicy ButtonGroupEntry::getClickActivationPolicy() const {
  return group_ != nullptr && group_->connected()
             ? ClickActivationPolicy::kImmediateContinueAnimation
             : ClickActivationPolicy::kAfterNaturalAnimation;
}
void ButtonGroupEntry::onClicked() {
  if (!eligible()) return;
  group_->activateEntry(*this);
}
void ButtonGroupEntry::paint(PaintContext& context) const {
  const Canvas& canvas = context.canvas();
  int inset = outlineInset();
  Rect full = bounds();
  Rect b(full.xMin() + inset, full.yMin() + inset, full.xMax() - inset,
         full.yMax() - inset);
  Color content =
      group_ != nullptr && group_->connected()
          ? ResolveConnectedAppearance(theme(), static_cast<GroupStyle>(style_),
                                       group_->selected(group_->indexOf(*this)),
                                       effectivelyEnabled())
                .content
          : ResolveButtonAppearance(theme(), Variant(style_),
                                    effectivelyEnabled())
                .content;
  if (inset > 0) {
    canvas.clearRect(full.xMin(), full.yMin(), full.xMax(), b.yMin() - 1);
    canvas.clearRect(full.xMin(), b.yMax() + 1, full.xMax(), full.yMax());
    canvas.clearRect(full.xMin(), b.yMin(), b.xMin() - 1, b.yMax());
    canvas.clearRect(b.xMax() + 1, b.yMin(), full.xMax(), b.yMax());
  }
  if (b.empty()) return;
  roo::string_view label = contentLabel();
  const MonoIcon* icon = contentIcon();
  const TextStyle& text = text_style_label_large();
  if (icon == nullptr) {
    if (label.empty())
      canvas.clearRect(b);
    else
      canvas.drawTiled(
          GroupLabel(label, text.font(), content, text.fontOptions()), b,
          roo_display::kCenter | roo_display::kMiddle);
    return;
  }
  MonoIcon painted = *icon;
  painted.color_mode().setColor(content);
  if (label.empty()) {
    canvas.drawTiled(painted, b, roo_display::kCenter | roo_display::kMiddle);
    return;
  }
  ButtonContentMetrics metrics = ResolveButtonContentMetrics(
      label, icon, group_ == nullptr ? ButtonSize::kSmall : group_->size(),
      ResolveDensityLevel(theme().material3Theme().density));
  int x = b.xMin() + (b.width() - metrics.content_width) / 2;
  bool rtl = group_ != nullptr &&
             group_->layoutDirection() == LayoutDirection::kRightToLeft;
  int first = rtl ? metrics.text_width : metrics.icon_slot_width;
  int second = rtl ? metrics.icon_slot_width : metrics.text_width;
  canvas.clearRect(b.xMin(), b.yMin(), x - 1, b.yMax());
  Rect a(x, b.yMin(), x + first - 1, b.yMax());
  GroupLabel painted_label(label, text.font(), content, text.fontOptions());
  if (rtl)
    canvas.drawTiled(painted_label, a,
                     roo_display::kCenter | roo_display::kMiddle);
  else
    canvas.drawTiled(painted, a, roo_display::kCenter | roo_display::kMiddle);
  x += first;
  canvas.clearRect(x, b.yMin(), x + metrics.gap - 1, b.yMax());
  x += metrics.gap;
  Rect c(x, b.yMin(), x + second - 1, b.yMax());
  if (rtl)
    canvas.drawTiled(painted, c, roo_display::kCenter | roo_display::kMiddle);
  else
    canvas.drawTiled(painted_label, c,
                     roo_display::kCenter | roo_display::kMiddle);
  canvas.clearRect(x + second, b.yMin(), b.xMax(), b.yMax());
}
ButtonGroupLabelEntry::ButtonGroupLabelEntry(ApplicationContext& context,
                                             roo::string_view label,
                                             GroupStyle style)
    : ButtonGroupEntry(context, style), label_(label) {}
void ButtonGroupLabelEntry::setLabel(roo::string_view label) {
  if (label_.data() == label.data() && label_.size() == label.size()) return;
  label_ = label;
  contentChanged();
}
void ButtonGroupLabelEntry::setIcon(const MonoIcon* icon) {
  if (icon_ == icon) return;
  icon_ = icon;
  contentChanged();
}
ButtonGroupIconEntry::ButtonGroupIconEntry(ApplicationContext& context,
                                           const MonoIcon& icon,
                                           GroupStyle style)
    : ButtonGroupEntry(context, style), icon_(&icon) {}
void ButtonGroupIconEntry::setIcon(const MonoIcon& icon) {
  if (icon_ == &icon) return;
  icon_ = &icon;
  contentChanged();
}
}  // namespace roo_windows::material3::internal
