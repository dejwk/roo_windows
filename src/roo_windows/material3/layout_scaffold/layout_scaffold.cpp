#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"

#include <algorithm>
#include <limits>

#include "roo_logging.h"
#include "roo_windows/core/theme.h"

namespace roo_windows::material3 {
namespace {

constexpr int BreakpointIndex(LayoutBreakpoint breakpoint) {
  return static_cast<int>(breakpoint);
}

int32_t ScaledDp(int16_t value) { return Scaled<int32_t>(value); }

void CheckScaledDimension(int16_t value, const char* name) {
  CHECK(value >= 0) << name << " must be non-negative";
  CHECK(ScaledDp(value) <= std::numeric_limits<XDim>::max())
      << name << " exceeds XDim after scaling";
}

void CheckTokens(const BreakpointTokens& tokens) {
  CHECK(tokens.columns >= 1) << "Layout breakpoint policy needs a column";
  CheckScaledDimension(tokens.outer_margin_dp, "outer margin");
  CheckScaledDimension(tokens.gutter_dp, "gutter");
}

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

int CountChildren(const Widget* const* children, int count) {
  int result = 0;
  for (int i = 0; i < count; ++i) result += children[i] != nullptr;
  return result;
}

}  // namespace

bool BreakpointRange::contains(LayoutBreakpoint breakpoint) const {
  return BreakpointIndex(min) <= BreakpointIndex(max) &&
         BreakpointIndex(breakpoint) >= BreakpointIndex(min) &&
         BreakpointIndex(breakpoint) <= BreakpointIndex(max);
}

XDim LayoutMetrics::columnStart(uint8_t column) const {
  if (content_bounds.empty() || column >= columns || column_width <= 0) {
    return 0;
  }
  const XDim stride = column_width + gutter;
  if (direction == LayoutDirection::kLeftToRight) {
    return content_bounds.xMin() + column * stride;
  }
  return content_bounds.xMax() - column_width - column * stride + 1;
}

Rect LayoutMetrics::spanBounds(uint8_t first_column, uint8_t span, YDim top,
                               YDim height) const {
  if (content_bounds.empty() || span == 0 || height <= 0 ||
      first_column >= columns || column_width <= 0) {
    return EmptyRect();
  }
  const uint8_t resolved_span =
      std::min<uint8_t>(span, static_cast<uint8_t>(columns - first_column));
  const XDim span_width =
      resolved_span * column_width + (resolved_span - 1) * gutter;
  const XDim first = columnStart(first_column);
  if (direction == LayoutDirection::kLeftToRight) {
    return Rect(first, top, first + span_width - 1, top + height - 1);
  }
  return Rect(first - span_width + column_width, top, first + column_width - 1,
              top + height - 1);
}

LayoutBreakpointPolicy::LayoutBreakpointPolicy(BreakpointThresholds thresholds,
                                               BreakpointTokens compact,
                                               BreakpointTokens medium,
                                               BreakpointTokens expanded,
                                               BreakpointTokens large,
                                               BreakpointTokens extra_large)
    : thresholds_(thresholds),
      tokens_{compact, medium, expanded, large, extra_large} {
  CHECK(thresholds_.medium_min_dp >= 0);
  CHECK(thresholds_.medium_min_dp < thresholds_.expanded_min_dp);
  CHECK(thresholds_.expanded_min_dp < thresholds_.large_min_dp);
  CHECK(thresholds_.large_min_dp < thresholds_.extra_large_min_dp);
  CheckScaledDimension(thresholds_.medium_min_dp, "medium threshold");
  CheckScaledDimension(thresholds_.expanded_min_dp, "expanded threshold");
  CheckScaledDimension(thresholds_.large_min_dp, "large threshold");
  CheckScaledDimension(thresholds_.extra_large_min_dp, "extra-large threshold");
  for (const BreakpointTokens& breakpoint_tokens : tokens_) {
    CheckTokens(breakpoint_tokens);
  }
}

const LayoutBreakpointPolicy& LayoutBreakpointPolicy::Default() {
  static const LayoutBreakpointPolicy policy;
  return policy;
}

LayoutBreakpoint LayoutBreakpointPolicy::resolveWidthPx(XDim width_px) const {
  const int32_t width = std::max<XDim>(0, width_px);
  if (width >= ScaledDp(thresholds_.extra_large_min_dp)) {
    return LayoutBreakpoint::kExtraLarge;
  }
  if (width >= ScaledDp(thresholds_.large_min_dp)) {
    return LayoutBreakpoint::kLarge;
  }
  if (width >= ScaledDp(thresholds_.expanded_min_dp)) {
    return LayoutBreakpoint::kExpanded;
  }
  if (width >= ScaledDp(thresholds_.medium_min_dp)) {
    return LayoutBreakpoint::kMedium;
  }
  return LayoutBreakpoint::kCompact;
}

const BreakpointTokens& LayoutBreakpointPolicy::tokens(
    LayoutBreakpoint breakpoint) const {
  return tokens_[BreakpointIndex(breakpoint)];
}

LayoutMetrics LayoutBreakpointPolicy::resolveMetrics(
    const Rect& safe_bounds, LayoutDirection direction) const {
  return resolveMetricsForBreakpoint(
      safe_bounds,
      resolveWidthPx(safe_bounds.empty() ? 0 : safe_bounds.width()), direction);
}

LayoutMetrics LayoutBreakpointPolicy::resolveMetricsForBreakpoint(
    const Rect& safe_bounds, LayoutBreakpoint breakpoint,
    LayoutDirection direction) const {
  LayoutMetrics result;
  result.safe_bounds = safe_bounds;
  result.direction = direction;
  result.breakpoint = breakpoint;
  if (safe_bounds.empty()) return result;

  const BreakpointTokens& selected_tokens = tokens(result.breakpoint);
  const int32_t safe_width = safe_bounds.width();
  const int32_t requested_margin = ScaledDp(selected_tokens.outer_margin_dp);
  const int32_t margin = std::min<int32_t>(
      requested_margin, std::max<int32_t>(0, (safe_width - 1) / 2));
  const int32_t requested_gutter = ScaledDp(selected_tokens.gutter_dp);
  uint8_t columns = selected_tokens.columns;
  while (columns > 1 &&
         safe_width - 2 * margin - (columns - 1) * requested_gutter < columns) {
    --columns;
  }

  const int32_t gutter = columns == 1 ? 0 : requested_gutter;
  const int32_t available_width =
      safe_width - 2 * margin - (columns - 1) * gutter;
  const int32_t column_width = available_width / columns;
  const int32_t remainder = available_width - column_width * columns;
  const int32_t leading_remainder = remainder / 2;
  const int32_t trailing_remainder = remainder - leading_remainder;
  const int32_t left_remainder = direction == LayoutDirection::kLeftToRight
                                     ? leading_remainder
                                     : trailing_remainder;
  const int32_t content_width = column_width * columns + gutter * (columns - 1);

  result.columns = columns;
  result.outer_margin = margin;
  result.gutter = gutter;
  result.column_width = column_width;
  const XDim content_left = safe_bounds.xMin() + margin + left_remainder;
  result.content_bounds =
      Rect(content_left, safe_bounds.yMin(), content_left + content_width - 1,
           safe_bounds.yMax());
  return result;
}

LayoutScaffold::LayoutScaffold(ApplicationContext& context)
    : Container(context),
      top_bar_(nullptr),
      bottom_bar_(nullptr),
      leading_rail_(nullptr),
      trailing_rail_(nullptr),
      body_(nullptr),
      policy_(&LayoutBreakpointPolicy::Default()),
      safety_insets_(Insets::Zero()),
      content_insets_(Insets::Zero()),
      bottom_bar_bounds_(EmptyRect()),
      top_bar_height_(0),
      bottom_bar_height_(0),
      leading_rail_width_(0),
      trailing_rail_width_(0),
      direction_(static_cast<uint8_t>(LayoutDirection::kLeftToRight)) {}

LayoutScaffold::~LayoutScaffold() {
  if (top_bar_ != nullptr) detachChild(top_bar_);
  if (bottom_bar_ != nullptr) detachChild(bottom_bar_);
  if (leading_rail_ != nullptr) detachChild(leading_rail_);
  if (trailing_rail_ != nullptr) detachChild(trailing_rail_);
  if (body_ != nullptr) detachChild(body_);
}

void LayoutScaffold::setBreakpointPolicy(const LayoutBreakpointPolicy& policy) {
  if (policy_ == &policy) return;
  policy_ = &policy;
  requestLayout();
}

void LayoutScaffold::setLayoutDirection(LayoutDirection direction) {
  if (layoutDirection() == direction) return;
  direction_ = static_cast<uint8_t>(direction);
  requestLayout();
}

LayoutDirection LayoutScaffold::layoutDirection() const {
  return static_cast<LayoutDirection>(direction_);
}

void LayoutScaffold::setSafetyInsets(Insets insets) {
  const Insets clamped = ClampInsets(insets);
  if (safety_insets_ == clamped) return;
  safety_insets_ = clamped;
  requestLayout();
}

void LayoutScaffold::setTopBar(WidgetRef widget, BreakpointRange visibility) {
  top_bar_visibility_ = visibility;
  replaceSlot(top_bar_, std::move(widget));
}

void LayoutScaffold::setTopBarVisibility(BreakpointRange visibility) {
  setSlotVisibility(top_bar_visibility_, visibility);
}

void LayoutScaffold::clearTopBar() { replaceSlot(top_bar_, WidgetRef()); }

void LayoutScaffold::setBottomBar(WidgetRef widget,
                                  BreakpointRange visibility) {
  bottom_bar_visibility_ = visibility;
  replaceSlot(bottom_bar_, std::move(widget));
}

void LayoutScaffold::setBottomBarVisibility(BreakpointRange visibility) {
  setSlotVisibility(bottom_bar_visibility_, visibility);
}

void LayoutScaffold::clearBottomBar() { replaceSlot(bottom_bar_, WidgetRef()); }

void LayoutScaffold::setLeadingRail(WidgetRef widget,
                                    BreakpointRange visibility) {
  leading_rail_visibility_ = visibility;
  replaceSlot(leading_rail_, std::move(widget));
}

void LayoutScaffold::setLeadingRailVisibility(BreakpointRange visibility) {
  setSlotVisibility(leading_rail_visibility_, visibility);
}

void LayoutScaffold::clearLeadingRail() {
  replaceSlot(leading_rail_, WidgetRef());
}

void LayoutScaffold::setTrailingRail(WidgetRef widget,
                                     BreakpointRange visibility) {
  trailing_rail_visibility_ = visibility;
  replaceSlot(trailing_rail_, std::move(widget));
}

void LayoutScaffold::setTrailingRailVisibility(BreakpointRange visibility) {
  setSlotVisibility(trailing_rail_visibility_, visibility);
}

void LayoutScaffold::clearTrailingRail() {
  replaceSlot(trailing_rail_, WidgetRef());
}

void LayoutScaffold::setBody(WidgetRef widget) {
  replaceSlot(body_, std::move(widget));
}

roo_display::Color LayoutScaffold::background() const {
  return theme().material3Theme().color.background;
}

::roo_windows::material3::ColorToken LayoutScaffold::containerRole() const {
  return ColorToken::kBackground;
}

PreferredSize LayoutScaffold::getPreferredSize() const {
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::MatchParentHeight());
}

Dimensions LayoutScaffold::onMeasure(WidthSpec width, HeightSpec height) {
  const XDim measured_width = width.resolveSize(width.value());
  const YDim measured_height = height.resolveSize(height.value());
  const Rect outer(0, 0, measured_width - 1, measured_height - 1);
  const LayoutBreakpoint breakpoint = policy_->resolveWidthPx(measured_width);
  updateChromeVisibility(breakpoint);

  top_bar_height_ = 0;
  bottom_bar_height_ = 0;
  leading_rail_width_ = 0;
  trailing_rail_width_ = 0;
  const Rect safe = ApplyInsets(outer, safety_insets_);
  if (safe.empty()) return Dimensions(measured_width, measured_height);

  if (top_bar_ != nullptr && top_bar_->isVisible()) {
    top_bar_height_ = std::min<YDim>(
        safe.height(), top_bar_
                           ->measure(WidthSpec::Exactly(safe.width()),
                                     HeightSpec::AtMost(safe.height()))
                           .height());
  }
  const YDim band_height = std::max<YDim>(0, safe.height() - top_bar_height_);
  if (bottom_bar_ != nullptr && bottom_bar_->isVisible()) {
    bottom_bar_height_ = std::min<YDim>(
        band_height, bottom_bar_
                         ->measure(WidthSpec::Exactly(safe.width()),
                                   HeightSpec::AtMost(band_height))
                         .height());
  }
  const YDim rail_height = std::max<YDim>(0, band_height - bottom_bar_height_);
  if (leading_rail_ != nullptr && leading_rail_->isVisible()) {
    leading_rail_width_ = std::min<XDim>(
        safe.width(), leading_rail_
                          ->measure(WidthSpec::AtMost(safe.width()),
                                    HeightSpec::Exactly(rail_height))
                          .width());
  }
  const XDim remaining_width =
      std::max<XDim>(0, safe.width() - leading_rail_width_);
  if (trailing_rail_ != nullptr && trailing_rail_->isVisible()) {
    trailing_rail_width_ = std::min<XDim>(
        remaining_width, trailing_rail_
                             ->measure(WidthSpec::AtMost(remaining_width),
                                       HeightSpec::Exactly(rail_height))
                             .width());
  }
  if (body_ != nullptr && !body_->isGone()) {
    body_->measure(WidthSpec::Exactly(std::max<XDim>(
                       0, remaining_width - trailing_rail_width_)),
                   HeightSpec::Exactly(rail_height));
  }
  return Dimensions(measured_width, measured_height);
}

void LayoutScaffold::onLayout(bool changed, const Rect& rect) {
  const LayoutBreakpoint breakpoint = policy_->resolveWidthPx(rect.width());
  updateChromeVisibility(breakpoint);
  const Rect safe = ApplyInsets(rect, safety_insets_);
  if (safe.empty()) {
    layoutSlot(top_bar_, EmptyRect());
    layoutSlot(bottom_bar_, EmptyRect());
    layoutSlot(leading_rail_, EmptyRect());
    layoutSlot(trailing_rail_, EmptyRect());
    layoutSlot(body_, EmptyRect());
    clearLayoutMetrics(breakpoint);
    return;
  }

  const YDim top = safe.yMin();
  const Rect top_bounds =
      top_bar_ != nullptr && top_bar_->isVisible()
          ? Rect(safe.xMin(), top, safe.xMax(), top + top_bar_height_ - 1)
          : EmptyRect();
  const YDim bottom = safe.yMax();
  const Rect bottom_bounds =
      bottom_bar_ != nullptr && bottom_bar_->isVisible()
          ? Rect(safe.xMin(), bottom - bottom_bar_height_ + 1, safe.xMax(),
                 bottom)
          : EmptyRect();
  const Rect rail_band(safe.xMin(), top + top_bar_height_, safe.xMax(),
                       bottom - bottom_bar_height_);
  const bool rtl = layoutDirection() == LayoutDirection::kRightToLeft;
  const XDim leading_left =
      rtl ? rail_band.xMax() - leading_rail_width_ + 1 : rail_band.xMin();
  const Rect leading_bounds =
      leading_rail_ != nullptr && leading_rail_->isVisible()
          ? Rect(leading_left, rail_band.yMin(),
                 leading_left + leading_rail_width_ - 1, rail_band.yMax())
          : EmptyRect();
  const XDim trailing_left =
      rtl ? rail_band.xMin() : rail_band.xMax() - trailing_rail_width_ + 1;
  const Rect trailing_bounds =
      trailing_rail_ != nullptr && trailing_rail_->isVisible()
          ? Rect(trailing_left, rail_band.yMin(),
                 trailing_left + trailing_rail_width_ - 1, rail_band.yMax())
          : EmptyRect();
  const XDim body_left = rtl ? rail_band.xMin() + trailing_rail_width_
                             : rail_band.xMin() + leading_rail_width_;
  const XDim body_right = rtl ? rail_band.xMax() - leading_rail_width_
                              : rail_band.xMax() - trailing_rail_width_;
  const Rect body_bounds(body_left, rail_band.yMin(), body_right,
                         rail_band.yMax());

  layoutSlot(top_bar_, top_bounds);
  layoutSlot(bottom_bar_, bottom_bounds);
  layoutSlot(leading_rail_, leading_bounds);
  layoutSlot(trailing_rail_, trailing_bounds);
  layoutSlot(body_, body_ != nullptr ? body_bounds : EmptyRect());
  bottom_bar_bounds_ = bottom_bounds;
  if (body_ == nullptr || body_bounds.empty()) {
    metrics_ = policy_->resolveMetricsForBreakpoint(EmptyRect(), breakpoint,
                                                    layoutDirection());
    content_insets_ = Insets::Zero();
    return;
  }
  metrics_ = policy_->resolveMetricsForBreakpoint(body_bounds, breakpoint,
                                                  layoutDirection());
  content_insets_ = Insets(
      body_bounds.xMin() - rect.xMin(), body_bounds.yMin() - rect.yMin(),
      rect.xMax() - body_bounds.xMax(), rect.yMax() - body_bounds.yMax());
}

int LayoutScaffold::getChildrenCount() const {
  const Widget* children[] = {top_bar_, bottom_bar_, leading_rail_,
                              trailing_rail_, body_};
  return CountChildren(children, 5);
}

const Widget& LayoutScaffold::getChild(int index) const {
  const Widget* children[] = {top_bar_, bottom_bar_, leading_rail_,
                              trailing_rail_, body_};
  for (const Widget* child : children) {
    if (child != nullptr && index-- == 0) return *child;
  }
  LOG(FATAL) << "LayoutScaffold child index out of bounds";
  return *body_;
}

Widget& LayoutScaffold::getChild(int index) {
  Widget* children[] = {top_bar_, bottom_bar_, leading_rail_, trailing_rail_,
                        body_};
  for (Widget* child : children) {
    if (child != nullptr && index-- == 0) return *child;
  }
  LOG(FATAL) << "LayoutScaffold child index out of bounds";
  return *body_;
}

Insets LayoutScaffold::ClampInsets(Insets insets) {
  return Insets(std::max<int16_t>(0, insets.left()),
                std::max<int16_t>(0, insets.top()),
                std::max<int16_t>(0, insets.right()),
                std::max<int16_t>(0, insets.bottom()));
}

Rect LayoutScaffold::ApplyInsets(const Rect& rect, Insets insets) {
  if (rect.empty()) return EmptyRect();
  const int32_t width = rect.width() - insets.left() - insets.right();
  const int32_t height = rect.height() - insets.top() - insets.bottom();
  if (width <= 0 || height <= 0) return EmptyRect();
  return Rect(rect.xMin() + insets.left(), rect.yMin() + insets.top(),
              rect.xMax() - insets.right(), rect.yMax() - insets.bottom());
}

Rect LayoutScaffold::EmptyRect() {
  return ::roo_windows::material3::EmptyRect();
}

void LayoutScaffold::replaceSlot(Widget*& slot, WidgetRef widget) {
  Widget* incoming = widget.get();
  if (incoming == slot) return;
  if (slot != nullptr) detachChild(slot);
  slot = incoming;
  if (slot != nullptr) {
    CHECK(slot->parent() == nullptr);
    attachChild(std::move(widget));
  }
  requestLayout();
}

void LayoutScaffold::setSlotVisibility(BreakpointRange& target,
                                       BreakpointRange visibility) {
  target = visibility;
  requestLayout();
}

void LayoutScaffold::updateChromeVisibility(LayoutBreakpoint breakpoint) {
  Widget* widgets[] = {top_bar_, bottom_bar_, leading_rail_, trailing_rail_};
  const BreakpointRange ranges[] = {top_bar_visibility_, bottom_bar_visibility_,
                                    leading_rail_visibility_,
                                    trailing_rail_visibility_};
  for (int i = 0; i < 4; ++i) {
    if (widgets[i] == nullptr) continue;
    widgets[i]->setVisibility(ranges[i].contains(breakpoint)
                                  ? Visibility::kVisible
                                  : Visibility::kGone);
  }
}

void LayoutScaffold::clearLayoutMetrics(LayoutBreakpoint breakpoint) {
  metrics_ = policy_->resolveMetricsForBreakpoint(EmptyRect(), breakpoint,
                                                  layoutDirection());
  content_insets_ = Insets::Zero();
  bottom_bar_bounds_ = EmptyRect();
}

void LayoutScaffold::layoutSlot(Widget* widget, const Rect& bounds) {
  if (widget != nullptr) widget->layout(bounds);
}

}  // namespace roo_windows::material3
