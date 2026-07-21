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
  Widget* top_bar = top_bar_;
  top_bar_ = nullptr;
  if (top_bar != nullptr) detachChild(top_bar);
  Widget* bottom_bar = bottom_bar_;
  bottom_bar_ = nullptr;
  if (bottom_bar != nullptr) detachChild(bottom_bar);
  Widget* leading_rail = leading_rail_;
  leading_rail_ = nullptr;
  if (leading_rail != nullptr) detachChild(leading_rail);
  Widget* trailing_rail = trailing_rail_;
  trailing_rail_ = nullptr;
  if (trailing_rail != nullptr) detachChild(trailing_rail);
  Widget* body = body_;
  body_ = nullptr;
  if (body != nullptr) detachChild(body);
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
    // A collapsed safety area must actively remove every child from layout;
    // keeping stale chrome bounds would leave invalid published geometry.
    layoutSlot(top_bar_, EmptyRect());
    layoutSlot(bottom_bar_, EmptyRect());
    layoutSlot(leading_rail_, EmptyRect());
    layoutSlot(trailing_rail_, EmptyRect());
    layoutSlot(body_, EmptyRect());
    clearLayoutMetrics(breakpoint);
    return;
  }

  // Bars reserve vertical space first. Rails and body then share the band
  // between them, so all chrome changes resolve in one coordinate system.
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

  // Publish the resolved rectangles only after every slot bound is known.
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

PaneLayout::PaneLayout(ApplicationContext& context)
    : Container(context),
      leading_(nullptr),
      main_(nullptr),
      trailing_(nullptr),
      policy_(&LayoutBreakpointPolicy::Default()),
      main_min_width_dp_(360),
      active_pane_(static_cast<uint8_t>(PaneRole::kMain)),
      multi_pane_enabled_(true),
      direction_(static_cast<uint8_t>(LayoutDirection::kLeftToRight)) {}

PaneLayout::~PaneLayout() {
  // Clear each fixed slot before detaching it: an owned detach can delete the
  // child, while Container invalidation may enumerate the remaining slots.
  Widget* leading = leading_;
  leading_ = nullptr;
  if (leading != nullptr) detachChild(leading);
  Widget* main = main_;
  main_ = nullptr;
  if (main != nullptr) detachChild(main);
  Widget* trailing = trailing_;
  trailing_ = nullptr;
  if (trailing != nullptr) detachChild(trailing);
}

void PaneLayout::setBreakpointPolicy(const LayoutBreakpointPolicy& policy) {
  if (policy_ == &policy) return;
  policy_ = &policy;
  requestLayout();
}

void PaneLayout::setLayoutDirection(LayoutDirection direction) {
  if (layoutDirection() == direction) return;
  direction_ = static_cast<uint8_t>(direction);
  requestLayout();
}

LayoutDirection PaneLayout::layoutDirection() const {
  return static_cast<LayoutDirection>(direction_);
}

void PaneLayout::setMainMinWidthDp(int16_t width_dp) {
  CheckScaledDimension(width_dp, "main pane minimum width");
  if (main_min_width_dp_ == width_dp) return;
  main_min_width_dp_ = width_dp;
  requestLayout();
}

void PaneLayout::setMultiPaneEnabled(bool enabled) {
  if (isMultiPaneEnabled() == enabled) return;
  multi_pane_enabled_ = enabled;
  requestLayout();
}

bool PaneLayout::isMultiPaneEnabled() const { return multi_pane_enabled_; }

bool PaneLayout::setActivePane(PaneRole role) {
  if (slotForRole(role) == nullptr) return false;
  if (activePane() == role) return true;
  active_pane_ = static_cast<uint8_t>(role);
  requestLayout();
  return true;
}

PaneRole PaneLayout::activePane() const {
  return static_cast<PaneRole>(active_pane_);
}

void PaneLayout::setLeadingPane(WidgetRef widget, PaneSpec spec) {
  CheckPaneSpec(spec);
  leading_spec_ = spec;
  replaceSlot(leading_, std::move(widget));
}

void PaneLayout::clearLeadingPane() { replaceSlot(leading_, WidgetRef()); }

void PaneLayout::setMainPane(WidgetRef widget) {
  replaceSlot(main_, std::move(widget));
}

void PaneLayout::clearMainPane() { replaceSlot(main_, WidgetRef()); }

void PaneLayout::setTrailingPane(WidgetRef widget, PaneSpec spec) {
  CheckPaneSpec(spec);
  trailing_spec_ = spec;
  replaceSlot(trailing_, std::move(widget));
}

void PaneLayout::clearTrailingPane() { replaceSlot(trailing_, WidgetRef()); }

bool PaneLayout::isLeadingVisible() const {
  return leading_ != nullptr && leading_->isVisible();
}

bool PaneLayout::isMainVisible() const {
  return main_ != nullptr && main_->isVisible();
}

bool PaneLayout::isTrailingVisible() const {
  return trailing_ != nullptr && trailing_->isVisible();
}

PreferredSize PaneLayout::getPreferredSize() const {
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::MatchParentHeight());
}

Dimensions PaneLayout::onMeasure(WidthSpec width, HeightSpec height) {
  const XDim measured_width = width.resolveSize(width.value());
  const YDim measured_height = height.resolveSize(height.value());
  // Measure against the same complete plan later used by onLayout(). This
  // keeps visibility, exact child specs, and final geometry in agreement.
  const PanePlan plan =
      resolvePlan(Rect(0, 0, measured_width - 1, measured_height - 1));
  applyVisibility(plan);
  if (plan.leading_visible) {
    leading_->measure(WidthSpec::Exactly(plan.leading_bounds.width()),
                      HeightSpec::Exactly(plan.leading_bounds.height()));
  }
  if (plan.main_visible) {
    main_->measure(WidthSpec::Exactly(plan.main_bounds.width()),
                   HeightSpec::Exactly(plan.main_bounds.height()));
  }
  if (plan.trailing_visible) {
    trailing_->measure(WidthSpec::Exactly(plan.trailing_bounds.width()),
                       HeightSpec::Exactly(plan.trailing_bounds.height()));
  }
  return Dimensions(measured_width, measured_height);
}

void PaneLayout::onLayout(bool changed, const Rect& rect) {
  // resolvePlan() is deliberately side-effect free. Apply its complete answer
  // once so focus clearing and empty bounds cannot observe a partial layout.
  const PanePlan plan = resolvePlan(rect);
  applyVisibility(plan);
  layoutSlot(leading_, plan.leading_bounds);
  layoutSlot(main_, plan.main_bounds);
  layoutSlot(trailing_, plan.trailing_bounds);
  metrics_ = plan.metrics;
}

int PaneLayout::getChildrenCount() const {
  const Widget* children[] = {leading_, main_, trailing_};
  return CountChildren(children, 3);
}

const Widget& PaneLayout::getChild(int index) const {
  const Widget* children[] = {leading_, main_, trailing_};
  for (const Widget* child : children) {
    if (child != nullptr && index-- == 0) return *child;
  }
  LOG(FATAL) << "PaneLayout child index out of bounds";
  return *main_;
}

Widget& PaneLayout::getChild(int index) {
  Widget* children[] = {leading_, main_, trailing_};
  for (Widget* child : children) {
    if (child != nullptr && index-- == 0) return *child;
  }
  LOG(FATAL) << "PaneLayout child index out of bounds";
  return *main_;
}

void PaneLayout::CheckPaneSpec(const PaneSpec& spec) {
  CheckScaledDimension(spec.min_width_dp, "pane minimum width");
  CheckScaledDimension(spec.preferred_width_dp, "pane preferred width");
  CHECK(spec.preferred_width_dp >= spec.min_width_dp)
      << "pane preferred width must not be smaller than its minimum width";
}

PaneLayout::PanePlan PaneLayout::resolvePlan(const Rect& bounds) const {
  PanePlan plan;
  // Pane breakpoints are local to the body region; the outer scaffold may
  // have a different width class after its chrome has claimed space.
  const LayoutBreakpoint breakpoint =
      policy_->resolveWidthPx(bounds.empty() ? 0 : bounds.width());
  plan.metrics = policy_->resolveMetrics(bounds, layoutDirection());
  if (bounds.empty()) return plan;

  // Compact presentation is the baseline. A cleared active slot intentionally
  // yields no replacement pane until the application selects one.
  const PaneRole active = activePane();
  if (slotForRole(active) == nullptr) return plan;

  plan.leading_visible = active == PaneRole::kLeading;
  plan.main_visible = active == PaneRole::kMain;
  plan.trailing_visible = active == PaneRole::kTrailing;

  // Widen the compact baseline only with the canonical main/side candidates.
  // Side participation remains breakpoint-gated; the active role is exempt.
  if (isMultiPaneEnabled() && main_ != nullptr) {
    plan.main_visible = true;
    if (leading_ != nullptr && active != PaneRole::kLeading &&
        leading_spec_.simultaneous_visibility.contains(breakpoint)) {
      plan.leading_visible = true;
    }
    if (trailing_ != nullptr && active != PaneRole::kTrailing &&
        trailing_spec_.simultaneous_visibility.contains(breakpoint)) {
      plan.trailing_visible = true;
    }
  }

  const XDim gutter = ScaledDp(policy_->tokens(breakpoint).gutter_dp);
  const XDim leading_min = ScaledDp(leading_spec_.min_width_dp);
  const XDim main_min = ScaledDp(main_min_width_dp_);
  const XDim trailing_min = ScaledDp(trailing_spec_.min_width_dp);
  const auto minimum_total = [&]() {
    const int pane_count = static_cast<int>(plan.leading_visible) +
                           static_cast<int>(plan.main_visible) +
                           static_cast<int>(plan.trailing_visible);
    return (plan.leading_visible ? leading_min : 0) +
           (plan.main_visible ? main_min : 0) +
           (plan.trailing_visible ? trailing_min : 0) +
           std::max(0, pane_count - 1) * gutter;
  };
  // Degrade from the lowest preservation priority while never removing the
  // caller-selected active role. This loop has at most three iterations.
  while (minimum_total() > bounds.width()) {
    if (plan.trailing_visible && active != PaneRole::kTrailing) {
      plan.trailing_visible = false;
    } else if (plan.leading_visible && active != PaneRole::kLeading) {
      plan.leading_visible = false;
    } else if (plan.main_visible && active != PaneRole::kMain) {
      plan.main_visible = false;
    } else {
      break;
    }
  }

  const int pane_count = static_cast<int>(plan.leading_visible) +
                         static_cast<int>(plan.main_visible) +
                         static_cast<int>(plan.trailing_visible);
  if (pane_count == 0) return plan;

  XDim leading_width = 0;
  XDim main_width = 0;
  XDim trailing_width = 0;
  const XDim total_gutter = std::max(0, pane_count - 1) * gutter;
  // A single active pane occupies the whole body. When main participates it
  // remains flexible after fixed-width side panes and gutters are reserved.
  if (!plan.main_visible) {
    if (plan.leading_visible) {
      leading_width = bounds.width();
    } else {
      trailing_width = bounds.width();
    }
  } else {
    XDim side_budget =
        std::max<XDim>(0, bounds.width() - main_min - total_gutter);
    if (plan.leading_visible && plan.trailing_visible) {
      // Start both sides at their minima, share spare width toward their
      // preferred widths, then let main absorb any unclaimed remainder.
      leading_width = leading_min;
      trailing_width = trailing_min;
      XDim extra_width = side_budget - leading_width - trailing_width;
      const XDim leading_preferred = ScaledDp(leading_spec_.preferred_width_dp);
      const XDim trailing_preferred =
          ScaledDp(trailing_spec_.preferred_width_dp);
      const XDim shared_extra = extra_width / 2;
      const XDim leading_extra =
          std::min<XDim>(leading_preferred - leading_width, shared_extra);
      const XDim trailing_extra =
          std::min<XDim>(trailing_preferred - trailing_width, shared_extra);
      leading_width += leading_extra;
      trailing_width += trailing_extra;
      extra_width -= leading_extra + trailing_extra;
      const XDim remaining_leading = leading_preferred - leading_width;
      const XDim remaining_trailing = trailing_preferred - trailing_width;
      const XDim additional_leading =
          std::min<XDim>(remaining_leading, extra_width);
      leading_width += additional_leading;
      extra_width -= additional_leading;
      trailing_width += std::min<XDim>(remaining_trailing, extra_width);
    } else if (plan.leading_visible) {
      leading_width = std::min<XDim>(ScaledDp(leading_spec_.preferred_width_dp),
                                     side_budget);
    }
    if (plan.trailing_visible && !plan.leading_visible) {
      trailing_width = std::min<XDim>(
          ScaledDp(trailing_spec_.preferred_width_dp), side_budget);
    }
    main_width = bounds.width() - total_gutter - leading_width - trailing_width;
  }

  // Compute logical order once, then mirror only its physical cursor for RTL.
  // PaneRole identity never changes with the writing direction.
  if (layoutDirection() == LayoutDirection::kLeftToRight) {
    XDim left = bounds.xMin();
    if (plan.leading_visible) {
      plan.leading_bounds =
          Rect(left, bounds.yMin(), left + leading_width - 1, bounds.yMax());
      left += leading_width + gutter;
    }
    if (plan.main_visible) {
      plan.main_bounds =
          Rect(left, bounds.yMin(), left + main_width - 1, bounds.yMax());
      left += main_width + gutter;
    }
    if (plan.trailing_visible) {
      plan.trailing_bounds =
          Rect(left, bounds.yMin(), left + trailing_width - 1, bounds.yMax());
    }
  } else {
    XDim right = bounds.xMax();
    if (plan.leading_visible) {
      plan.leading_bounds =
          Rect(right - leading_width + 1, bounds.yMin(), right, bounds.yMax());
      right -= leading_width + gutter;
    }
    if (plan.main_visible) {
      plan.main_bounds =
          Rect(right - main_width + 1, bounds.yMin(), right, bounds.yMax());
      right -= main_width + gutter;
    }
    if (plan.trailing_visible) {
      plan.trailing_bounds =
          Rect(right - trailing_width + 1, bounds.yMin(), right, bounds.yMax());
    }
  }
  return plan;
}

Widget* PaneLayout::slotForRole(PaneRole role) const {
  switch (role) {
    case PaneRole::kLeading:
      return leading_;
    case PaneRole::kMain:
      return main_;
    case PaneRole::kTrailing:
      return trailing_;
  }
  return nullptr;
}

void PaneLayout::replaceSlot(Widget*& slot, WidgetRef widget) {
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

void PaneLayout::applyVisibility(const PanePlan& plan) {
  // Visibility is applied after planning so a hidden pane clears interaction
  // state before it receives the matching empty layout rectangle.
  if (leading_ != nullptr) {
    leading_->setVisibility(plan.leading_visible ? Visibility::kVisible
                                                 : Visibility::kGone);
  }
  if (main_ != nullptr) {
    main_->setVisibility(plan.main_visible ? Visibility::kVisible
                                           : Visibility::kGone);
  }
  if (trailing_ != nullptr) {
    trailing_->setVisibility(plan.trailing_visible ? Visibility::kVisible
                                                   : Visibility::kGone);
  }
}

void PaneLayout::layoutSlot(Widget* widget, const Rect& bounds) {
  if (widget != nullptr) widget->layout(bounds);
}

}  // namespace roo_windows::material3
