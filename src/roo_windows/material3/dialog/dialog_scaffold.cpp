#include "roo_windows/material3/dialog/dialog_scaffold.h"

#include <algorithm>

#include "roo_display/ui/alignment.h"
#include "roo_display/ui/tile.h"
#include "roo_logging.h"
#include "roo_windows/core/display_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::internal {
namespace {

constexpr int16_t kScaffoldPadding = Scaled(24);
constexpr int16_t kSectionGap = Scaled(16);
constexpr int16_t kActionGap = Scaled(8);

DialogShowResult MapStartResult(PresentationStartResult result) {
  switch (result) {
    case PresentationStartResult::kStarted:
      return DialogShowResult::kShown;
    case PresentationStartResult::kHostBusy:
    case PresentationStartResult::kReentrantReplacement:
      return DialogShowResult::kHostBusy;
    case PresentationStartResult::kInteractionOwnerUnavailable:
      return DialogShowResult::kInteractionOwnerUnavailable;
    case PresentationStartResult::kSurfaceUnavailable:
      return DialogShowResult::kSurfaceUnavailable;
  }
  return DialogShowResult::kSurfaceUnavailable;
}

class FixedBoundsPreparation final
    : public ::roo_windows::internal::TransientSurfacePreparation {
 public:
  FixedBoundsPreparation(DialogScaffoldBase& scaffold, const Rect& bounds)
      : scaffold_(scaffold), bounds_(bounds) {}

 private:
  bool createAndResolveBounds(Rect& root_bounds_in_window) override {
    if (bounds_.empty()) return false;
    scaffold_.measure(WidthSpec::Exactly(bounds_.width()),
                      HeightSpec::Exactly(bounds_.height()));
    // Children need valid local rectangles before focus activation. The host
    // subsequently installs the same root size at its window-coordinate
    // offset without disturbing this internal layout.
    scaffold_.layout(Rect(0, 0, bounds_.width() - 1, bounds_.height() - 1));
    root_bounds_in_window = bounds_;
    return true;
  }

  void deleteAfterFailedAdmission() override {}

  DialogScaffoldBase& scaffold_;
  Rect bounds_;
};

class BasicDialogPreparation final
    : public ::roo_windows::internal::TransientSurfacePreparation {
 public:
  BasicDialogPreparation(DialogScaffoldBase& scaffold, Task& owner)
      : scaffold_(scaffold), owner_(owner) {}

 private:
  bool createAndResolveBounds(Rect& root_bounds_in_window) override {
    const MainWindow& window = owner_.window().root();
    const XDim edge = window.width() <= Scaled(600) ? Scaled(24) : Scaled(56);
    const XDim available_width = std::max<XDim>(1, window.width() - 2 * edge);
    const YDim available_height = std::max<YDim>(1, window.height() - 2 * edge);
    const XDim maximum_width = std::min<XDim>(Scaled(560), available_width);
    Dimensions natural = scaffold_.measure(
        WidthSpec::AtMost(maximum_width), HeightSpec::AtMost(available_height));
    const XDim minimum_width = std::min<XDim>(Scaled(280), available_width);
    const XDim width = std::max<XDim>(minimum_width, natural.width());
    Dimensions measured = scaffold_.measure(
        WidthSpec::Exactly(width), HeightSpec::AtMost(available_height));
    const XDim left = (window.width() - measured.width()) / 2;
    const YDim top = (window.height() - measured.height()) / 2;
    scaffold_.layout(Rect(0, 0, measured.width() - 1, measured.height() - 1));
    root_bounds_in_window = Rect(left, top, left + measured.width() - 1,
                                 top + measured.height() - 1);
    return true;
  }

  void deleteAfterFailedAdmission() override {}

  DialogScaffoldBase& scaffold_;
  Task& owner_;
};

}  // namespace

DialogScaffoldBase::DialogScaffoldBase(ApplicationContext& context,
                                       WidgetRef body,
                                       DialogScaffoldVariant variant)
    : Container(context),
      variant_(variant),
      title_(context, "", text_style_headline_small()),
      top_divider_(context),
      body_scroller_(context, *this),
      bottom_divider_(context),
      registration_(*this) {
  title_.setWrapMode(TextWrapMode::kWordWrap);
  title_.setMaxLines(2);
  title_.setEllipsize(true);
  title_.setVisibility(Visibility::kGone);
  top_divider_.setVisibility(Visibility::kGone);
  bottom_divider_.setVisibility(Visibility::kGone);
  attachChild(title_);
  attachChild(top_divider_);
  attachChild(body_scroller_);
  attachChild(bottom_divider_);
  setDialogBody(std::move(body));
}

DialogScaffoldBase::~DialogScaffoldBase() {
  prepareForDerivedDestruction();
  detachChild(&bottom_divider_);
  detachChild(&body_scroller_);
  detachChild(&top_divider_);
  detachChild(&title_);
}

ColorToken DialogScaffoldBase::containerRole() const {
  return ColorToken::kSurfaceContainerHigh;
}

Color DialogScaffoldBase::background() const {
  return theme().material3Theme().color.surfaceContainerHigh;
}

BorderStyle DialogScaffoldBase::getBorderStyle() const {
  return BorderStyle(variant_ == DialogScaffoldVariant::kBasic ? Scaled(28) : 0,
                     0);
}

void DialogScaffoldBase::paint(PaintContext& ctx) const {
  if (icon_ == nullptr || icon_height_ == 0) return;
  roo_display::Pictogram icon(*icon_);
  icon.color_mode().setColor(roo_display::AlphaBlend(
      ctx.bgcolor(), theme().material3Theme().color.secondary));
  ctx.drawTiled(
      icon,
      Rect(content_inset_, content_inset_, width() - content_inset_ - 1,
           content_inset_ + icon_height_ - 1),
      roo_display::kCenter | roo_display::kMiddle, isInvalidated());
}

Widget* DialogScaffoldBase::preferredFocusChild() {
  if (body_ != nullptr) {
    Widget* preferred = body_->preferredFocusChild();
    if (preferred != nullptr) return preferred;
    if (body_->isFocusable()) return body_;
  }
  return preferredChromeFocusChild();
}

void DialogScaffoldBase::attachDerivedChrome(DialogChromeSlot slot,
                                             Widget& chrome) {
  const uint8_t idx = static_cast<uint8_t>(slot);
  CHECK(chrome_[idx] == nullptr);
  chrome_[idx] = &chrome;
  attachChild(chrome);
  requestLayout();
}

void DialogScaffoldBase::detachDerivedChrome(DialogChromeSlot slot) {
  const uint8_t idx = static_cast<uint8_t>(slot);
  if (chrome_[idx] == nullptr) return;
  detachChild(chrome_[idx]);
  chrome_[idx] = nullptr;
  requestLayout();
}

void DialogScaffoldBase::setDialogBody(WidgetRef body) {
  if (body_ == body.get() &&
      (body_ == nullptr || body_->isOwnedByParent() == body.is_owned())) {
    return;
  }
  clearDialogBody();
  body_ = body.get();
  if (body_ != nullptr) body_scroller_.setContents(std::move(body));
  requestLayout();
}

void DialogScaffoldBase::setDialogTitle(std::string title) {
  const bool empty = title.empty();
  title_.setText(std::move(title));
  title_.setVisibility(empty ? Visibility::kGone : Visibility::kVisible);
}

void DialogScaffoldBase::setDialogIcon(const MonoIcon* icon) {
  if (icon_ == icon) return;
  icon_ = icon;
  invalidateInterior();
  requestLayout();
}

void DialogScaffoldBase::setDialogLayoutDirection(LayoutDirection direction) {
  if (direction_ == direction) return;
  direction_ = direction;
  requestLayout();
}

DialogShowResult DialogScaffoldBase::showDialogSurface(
    Task& interaction_owner, const Rect& bounds_in_window,
    TransientBarrierPaint barrier) {
  if (registration_.isActive()) return DialogShowResult::kAlreadyPresented;
  const TransientSurfaceSpec spec{
      barrier, TransientAdmissionPolicy::kRejectIfBusy,
      OutsideInteractionPolicy::kAbsorb,
      TransientPresentationPolicy(true, true), false};
  FixedBoundsPreparation preparation(*this, bounds_in_window);
  return MapStartResult(
      ::roo_windows::internal::GetTransientSurfaceHost(interaction_owner)
          .showPrepared(registration_, interaction_owner, *this, focus_scope_,
                        spec, preparation));
}

DialogShowResult DialogScaffoldBase::showBasicDialogSurface(
    Task& interaction_owner) {
  if (registration_.isActive()) return DialogShowResult::kAlreadyPresented;
  const TransientSurfaceSpec spec{
      TransientBarrierPaint::kScrim, TransientAdmissionPolicy::kRejectIfBusy,
      OutsideInteractionPolicy::kAbsorb,
      TransientPresentationPolicy(true, true), false};
  BasicDialogPreparation preparation(*this, interaction_owner);
  return MapStartResult(
      ::roo_windows::internal::GetTransientSurfaceHost(interaction_owner)
          .showPrepared(registration_, interaction_owner, *this, focus_scope_,
                        spec, preparation));
}

void DialogScaffoldBase::finishDialog(PresentationFinishReason reason) {
  registration_.finish(reason);
}

void DialogScaffoldBase::prepareForDerivedDestruction() {
  if (registration_.isActive()) {
    registration_.disablePresentationInput();
    registration_.cancelPresentation();
  }
  clearDialogBody();
  detachDerivedChrome(DialogChromeSlot::kSecondary);
  detachDerivedChrome(DialogChromeSlot::kPrimary);
}

Widget* DialogScaffoldBase::preferredChromeFocusChild() { return nullptr; }

BackResult DialogScaffoldBase::onDialogBackRequested(BackSource) {
  finishDialog(PresentationFinishReason::kBack);
  return BackResult::kHandled;
}

int DialogScaffoldBase::getChildrenCount() const {
  return 4 + (chrome_[0] != nullptr ? 1 : 0) + (chrome_[1] != nullptr ? 1 : 0);
}

const Widget& DialogScaffoldBase::getChild(int idx) const {
  return const_cast<DialogScaffoldBase*>(this)->getChild(idx);
}

Widget& DialogScaffoldBase::getChild(int idx) {
  switch (idx) {
    case 0:
      return title_;
    case 1:
      return body_scroller_;
    case 2:
      return top_divider_;
    case 3:
      return bottom_divider_;
    default:
      idx -= 4;
      if (chrome_[0] != nullptr) {
        if (idx == 0) return *chrome_[0];
        --idx;
      }
      CHECK(chrome_[1] != nullptr && idx == 0);
      return *chrome_[1];
  }
}

Dimensions DialogScaffoldBase::onMeasure(WidthSpec width, HeightSpec height) {
  content_inset_ = std::min<int16_t>(
      kScaffoldPadding,
      std::max<int16_t>(0,
                        std::min<int32_t>(width.value(), height.value()) / 4));
  const int16_t horizontal_padding = 2 * content_inset_;
  const int16_t vertical_padding = 2 * content_inset_;
  const int16_t available_width =
      std::max<int16_t>(0, width.value() - horizontal_padding);
  const int16_t available_height =
      std::max<int16_t>(0, height.value() - vertical_padding);

  int16_t fixed_height = 0;
  int16_t desired_width = 0;
  title_height_ = 0;
  icon_height_ = icon_ == nullptr ? 0 : AnchorDimensionsOf(*icon_).height();
  if (icon_height_ > 0) fixed_height += icon_height_ + kSectionGap;
  chrome_height_[0] = 0;
  chrome_height_[1] = 0;
  if (!title_.isGone()) {
    Dimensions measured = title_.measure(WidthSpec::AtMost(available_width),
                                         HeightSpec::AtMost(available_height));
    desired_width = std::max(desired_width, measured.width());
    title_height_ = measured.height();
    fixed_height += measured.height() + kSectionGap;
  }
  for (uint8_t i = 0; i < 2; ++i) {
    Widget* chrome = chrome_[i];
    if (chrome == nullptr || chrome->isGone()) continue;
    Dimensions measured = chrome->measure(WidthSpec::AtMost(available_width),
                                          HeightSpec::AtMost(available_height));
    desired_width = std::max(desired_width, measured.width());
    chrome_height_[i] = measured.height();
    fixed_height += measured.height() + kSectionGap;
  }
  const int16_t body_height =
      std::max<int16_t>(0, available_height - fixed_height);
  Dimensions body = body_scroller_.measure(WidthSpec::AtMost(available_width),
                                           HeightSpec::AtMost(body_height));
  desired_width = std::max(desired_width, body.width());
  return Dimensions(
      width.resolveSize(desired_width + horizontal_padding),
      height.resolveSize(body.height() + fixed_height + vertical_padding));
}

void DialogScaffoldBase::onLayout(bool, const Rect& rect) {
  const int16_t left = content_inset_;
  const int16_t right =
      std::max<int16_t>(left - 1, rect.width() - content_inset_ - 1);
  int16_t top = content_inset_;
  int16_t bottom = rect.height() - content_inset_;

  for (int idx = 1; idx >= 0; --idx) {
    Widget* chrome = chrome_[idx];
    if (chrome == nullptr || chrome->isGone()) continue;
    const YDim h = chrome_height_[idx];
    bottom -= h;
    chrome->layout(Rect(left, bottom, right, bottom + h - 1));
    bottom -= kSectionGap;
  }
  if (icon_height_ > 0) top += icon_height_ + kSectionGap;
  if (!title_.isGone()) {
    const YDim h = title_height_;
    title_.layout(Rect(left, top, right, top + h - 1));
    top += h + kSectionGap;
  }
  const int16_t body_bottom = std::max<int16_t>(top - 1, bottom - 1);
  body_scroller_.layout(Rect(left, top, right, body_bottom));
  top_divider_.layout(Rect(left, top, right, top + 1));
  bottom_divider_.layout(Rect(left, body_bottom - 1, right, body_bottom));
  updateDividers();
}

void DialogScaffoldBase::clearDialogBody() {
  if (body_ == nullptr) return;
  focus_scope_.clearRememberedFocus();
  body_scroller_.clearContents();
  body_ = nullptr;
}

void DialogScaffoldBase::updateDividers() {
  const Widget* contents = body_scroller_.contents();
  const bool clipped =
      contents != nullptr && contents->height() > body_scroller_.height();
  const SimpleScrollablePanel::ScrollPosition position =
      body_scroller_.getScrollPosition();
  top_divider_.setVisibility(clipped && position.y > 0 ? Visibility::kVisible
                                                       : Visibility::kGone);
  bottom_divider_.setVisibility(
      clipped && position.y < contents->height() - body_scroller_.height()
          ? Visibility::kVisible
          : Visibility::kGone);
}

void DialogScaffoldBase::Registration::detachPresentation(
    PresentationFinishReason) {
  // The host detaches the complete root. Persistent body and chrome remain
  // assembled so reopening preserves state and remembered focus.
}

void DialogScaffoldBase::Registration::onFinished(
    PresentationFinishReason reason) {
  owner_.onDialogPresentationFinished(reason);
}

BackResult DialogScaffoldBase::Registration::onBackRequested(
    BackSource source) {
  return owner_.onDialogBackRequested(source);
}

DialogActionStrip::ActionButton::ActionButton(ApplicationContext& context,
                                              DialogActionStrip& strip,
                                              uint8_t slot)
    : Button(context, {}, ButtonVariant::kText), strip_(strip), slot_(slot) {}

void DialogActionStrip::ActionButton::onClicked() {
  strip_.invoke(slot_);
  Button::onClicked();
}

DialogActionStrip::DialogActionStrip(ApplicationContext& context,
                                     DialogActionDelegate& owner,
                                     const DialogActionSpec* actions,
                                     uint8_t action_count)
    : Container(context),
      owner_(owner),
      buttons_{{context, *this, 0}, {context, *this, 1}},
      action_count_(action_count) {
  CHECK(actions != nullptr);
  CHECK(action_count == 1 || action_count == 2);
  if (action_count == 1) {
    CHECK(actions[0].role == DialogActionRole::kAcknowledge);
    CHECK(actions[0].enabled);
  } else {
    CHECK(actions[0].id != actions[1].id);
    const bool valid_roles = (actions[0].role == DialogActionRole::kDismiss &&
                              actions[1].role == DialogActionRole::kConfirm) ||
                             (actions[1].role == DialogActionRole::kDismiss &&
                              actions[0].role == DialogActionRole::kConfirm);
    CHECK(valid_roles);
    for (uint8_t i = 0; i < 2; ++i) {
      if (actions[i].role == DialogActionRole::kDismiss) {
        CHECK(actions[i].enabled);
      }
    }
  }
  for (uint8_t i = 0; i < action_count_; ++i) {
    actions_[i] = actions[i];
    buttons_[i].setLabel(actions_[i].label);
    buttons_[i].setEnabled(actions_[i].enabled);
    attachChild(buttons_[i]);
  }
}

DialogActionStrip::~DialogActionStrip() {
  for (int i = action_count_ - 1; i >= 0; --i) detachChild(&buttons_[i]);
}

void DialogActionStrip::setActionEnabled(uint8_t action_id, bool enabled) {
  for (uint8_t i = 0; i < action_count_; ++i) {
    if (actions_[i].id != action_id) continue;
    CHECK(enabled || actions_[i].role == DialogActionRole::kConfirm);
    actions_[i].enabled = enabled;
    buttons_[i].setEnabled(enabled);
    return;
  }
  CHECK(false) << "unknown dialog action ID";
}

void DialogActionStrip::setLayoutDirection(LayoutDirection direction) {
  if (direction_ == direction) return;
  direction_ = direction;
  requestLayout();
}

const DialogActionSpec& DialogActionStrip::action(uint8_t index) const {
  CHECK_LT(index, action_count_);
  return actions_[index];
}

Widget& DialogActionStrip::actionButton(uint8_t index) {
  CHECK_LT(index, action_count_);
  return buttons_[index];
}

const Widget& DialogActionStrip::actionButton(uint8_t index) const {
  return const_cast<DialogActionStrip*>(this)->actionButton(index);
}

Widget* DialogActionStrip::preferredFocusChild() {
  for (uint8_t i = 0; i < action_count_; ++i) {
    if (buttons_[i].isEnabled()) return &buttons_[i];
  }
  return nullptr;
}

int DialogActionStrip::getChildrenCount() const { return action_count_; }

const Widget& DialogActionStrip::getChild(int idx) const {
  CHECK_GE(idx, 0);
  CHECK_LT(idx, action_count_);
  return buttons_[idx];
}

Widget& DialogActionStrip::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const DialogActionStrip*>(this)->getChild(idx));
}

Dimensions DialogActionStrip::onMeasure(WidthSpec width, HeightSpec height) {
  XDim horizontal_width = 0;
  YDim horizontal_height = 0;
  for (uint8_t i = 0; i < action_count_; ++i) {
    button_dimensions_[i] =
        buttons_[i].measure(WidthSpec::AtMost(width.value()), height);
    horizontal_width += button_dimensions_[i].width();
    horizontal_height =
        std::max(horizontal_height, button_dimensions_[i].height());
  }
  if (action_count_ == 2) horizontal_width += kActionGap;
  stacked_ = action_count_ == 2 && horizontal_width > width.value();
  if (!stacked_) {
    return Dimensions(width.resolveSize(horizontal_width),
                      height.resolveSize(horizontal_height));
  }
  const YDim stacked_height = button_dimensions_[0].height() +
                              button_dimensions_[1].height() + kActionGap;
  return Dimensions(width.resolveSize(std::max(button_dimensions_[0].width(),
                                               button_dimensions_[1].width())),
                    height.resolveSize(stacked_height));
}

void DialogActionStrip::onLayout(bool, const Rect& rect) {
  if (action_count_ == 1) {
    const Dimensions d = button_dimensions_[0];
    const int16_t x = direction_ == LayoutDirection::kLeftToRight
                          ? rect.width() - d.width()
                          : 0;
    buttons_[0].layout(Rect(x, 0, x + d.width() - 1, d.height() - 1));
    return;
  }
  const uint8_t dismiss =
      actions_[0].role == DialogActionRole::kDismiss ? 0 : 1;
  const uint8_t confirm = 1 - dismiss;
  if (stacked_) {
    const Dimensions confirm_dims = button_dimensions_[confirm];
    const Dimensions dismiss_dims = button_dimensions_[dismiss];
    buttons_[confirm].layout(Rect(rect.width() - confirm_dims.width(), 0,
                                  rect.width() - 1, confirm_dims.height() - 1));
    const int16_t y = confirm_dims.height() + kActionGap;
    buttons_[dismiss].layout(Rect(rect.width() - dismiss_dims.width(), y,
                                  rect.width() - 1,
                                  y + dismiss_dims.height() - 1));
    return;
  }
  const uint8_t first =
      direction_ == LayoutDirection::kLeftToRight ? dismiss : confirm;
  const uint8_t second = 1 - first;
  const Dimensions first_dims = button_dimensions_[first];
  const Dimensions second_dims = button_dimensions_[second];
  int16_t x =
      rect.width() - first_dims.width() - kActionGap - second_dims.width();
  buttons_[first].layout(
      Rect(x, 0, x + first_dims.width() - 1, first_dims.height() - 1));
  x += first_dims.width() + kActionGap;
  buttons_[second].layout(
      Rect(x, 0, x + second_dims.width() - 1, second_dims.height() - 1));
}

void DialogActionStrip::invoke(uint8_t slot) {
  CHECK_LT(slot, action_count_);
  if (!actions_[slot].enabled) return;
  owner_.invokeDialogAction(actions_[slot].id, actions_[slot].role);
}

}  // namespace roo_windows::material3::internal
