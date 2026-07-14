#include "roo_windows/material3/app_bar/app_bar.h"

#include <algorithm>
#include <utility>

#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/shape/smooth.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_logging.h"
#include "roo_windows/material3/app_bar/app_bar_tokens.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {
namespace {

void CheckTrailingIndex(uint8_t index) {
  CHECK(index < 2) << "Material 3 app-bar trailing index must be 0 or 1";
}

// Counts only populated fixed child slots without allocating storage.
template <size_t N>
int ChildCount(Widget* const (&slots)[N]) {
  int result = 0;
  for (Widget* slot : slots) result += slot != nullptr;
  return result;
}

// Returns a populated fixed child slot or terminates on an invalid index.
template <size_t N>
const Widget& ChildAt(Widget* const (&slots)[N], int idx, const char* owner) {
  for (Widget* slot : slots) {
    if (slot != nullptr && idx-- == 0) return *slot;
  }
  LOG(FATAL) << owner << " child index out of bounds";
  return *slots[0];
}

// Presentation children (the passive search glyph and display text) must not
// swallow a search-surface tap. A supplied descendant still gets first refusal
// when it is itself interactive, so callers can attach independent actions.
bool AppendInteractiveChildTouchTarget(Widget& child, XDim x, YDim y,
                                       bool sloppy,
                                       std::vector<Widget*>& path) {
  const size_t old_size = path.size();
  const bool hit = sloppy
                       ? child.fillSloppyTouchTargetPath(
                             x - child.offsetLeft(), y - child.offsetTop(), path)
                       : child.fillTouchTargetPath(
                             x - child.offsetLeft(), y - child.offsetTop(), path);
  if (hit && !path.empty() && path.back()->isClickable()) return true;
  path.resize(old_size);
  return false;
}

}  // namespace

Dimensions internal::AppBarText::getSuggestedMinimumDimensions() const {
  if (text_.empty()) return Dimensions(0, 0);
  const roo_display::Font& font = font_ == nullptr ? font_body1() : *font_;
  return Dimensions(font.getHorizontalStringMetrics(text_).width(),
                    font.metrics().maxHeight());
}

void internal::AppBarText::paint(PaintContext& ctx) const {
  if (text_.empty()) return;
  const roo_display::Font& font = font_ == nullptr ? font_body1() : *font_;
  ctx.canvas().drawTiled(
      roo_display::StringViewLabel(
          text_, font, use_on_surface_variant_
                           ? theme().material3Theme().color.onSurfaceVariant
                           : theme().material3Theme().color.onSurface),
      bounds(), alignment_);
}

AppBar::AppBar(ApplicationContext& context, AppBarVariant variant)
    : Container(context),
      title_widget_(context),
      subtitle_widget_(context),
      leading_(nullptr),
      trailing_{nullptr, nullptr},
      variant_(variant),
      title_alignment_(AppBarTitleAlignment::kLeading),
      surface_state_(AppBarSurfaceState::kFlat) {
  // The title children are by-value, so a title-only bar has no dynamic
  // allocations and text updates follow the regular child invalidation path.
  title_widget_.setFont(titleFont());
  subtitle_widget_.setFont(font_subtitle1());
  title_widget_.setVisibility(Visibility::kGone);
  subtitle_widget_.setVisibility(Visibility::kGone);
  attachChild(title_widget_);
  attachChild(subtitle_widget_);
}

AppBar::~AppBar() {
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
  detachChild(&subtitle_widget_);
  detachChild(&title_widget_);
}

const internal::AppBarVariantTokens& AppBar::tokens() const {
  switch (variant_) {
    case AppBarVariant::kSmall:
      return internal::kSmallAppBarTokens;
    case AppBarVariant::kMediumFlexible:
      return internal::kMediumFlexibleAppBarTokens;
    case AppBarVariant::kLargeFlexible:
      return internal::kLargeFlexibleAppBarTokens;
  }
  return internal::kSmallAppBarTokens;
}

const roo_display::Font& AppBar::titleFont() const {
  switch (variant_) {
    case AppBarVariant::kSmall:
      return font_h6();
    case AppBarVariant::kMediumFlexible:
      return font_h5();
    case AppBarVariant::kLargeFlexible:
      return font_h4();
  }
  return font_h6();
}

int16_t AppBar::containerHeightDp() const {
  const internal::AppBarVariantTokens& variant_tokens = tokens();
  return variant_tokens.supports_subtitle && !subtitle_widget_.text().empty()
             ? variant_tokens.subtitle_container_height_dp
             : variant_tokens.container_height_dp;
}

void AppBar::setVariant(AppBarVariant variant) {
  if (variant_ == variant) return;
  variant_ = variant;
  title_widget_.setFont(titleFont());
  if (!tokens().supports_subtitle) {
    subtitle_widget_.setVisibility(Visibility::kGone);
  } else if (!subtitle_widget_.text().empty()) {
    subtitle_widget_.setVisibility(Visibility::kVisible);
  }
  invalidateInterior();
  requestLayout();
}

void AppBar::setTitleAlignment(AppBarTitleAlignment alignment) {
  if (title_alignment_ == alignment) return;
  title_alignment_ = alignment;
  const roo_display::Alignment text_alignment =
      (alignment == AppBarTitleAlignment::kCentered ? roo_display::kCenter
                                                    : roo_display::kLeft) |
      roo_display::kMiddle;
  title_widget_.setAlignment(text_alignment);
  subtitle_widget_.setAlignment(text_alignment);
  invalidateInterior();
  requestLayout();
}

void AppBar::setSurfaceState(AppBarSurfaceState state) {
  if (surface_state_ == state) return;
  surface_state_ = state;
  invalidateInterior();
}

void AppBar::setTitle(roo::string_view title) {
  if (title_widget_.text() == title) return;
  title_widget_.setText(title);
  title_widget_.setVisibility(title.empty() ? Visibility::kGone
                                             : Visibility::kVisible);
  invalidateInterior();
  requestLayout();
}

void AppBar::setSubtitle(roo::string_view subtitle) {
  if (subtitle_widget_.text() == subtitle) return;
  subtitle_widget_.setText(subtitle);
  subtitle_widget_.setVisibility(
      !subtitle.empty() && tokens().supports_subtitle ? Visibility::kVisible
                                                       : Visibility::kGone);
  invalidateInterior();
  requestLayout();
}

void AppBar::replaceSlot(Widget*& slot, WidgetRef widget) {
  Widget* incoming = widget.get();
  if (incoming == slot) return;
  if (slot) detachChild(slot);
  slot = incoming;
  if (slot) {
    CHECK(slot->parent() == nullptr);
    attachChild(std::move(widget));
  }
  invalidateInterior();
  requestLayout();
}

void AppBar::setLeading(WidgetRef widget) {
  replaceSlot(leading_, std::move(widget));
}

void AppBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int AppBar::getChildrenCount() const {
  return (!title_widget_.isGone()) + (!subtitle_widget_.isGone()) +
         (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& AppBar::getChild(int idx) const {
  if (!title_widget_.isGone() && idx-- == 0) return title_widget_;
  if (!subtitle_widget_.isGone() && idx-- == 0) return subtitle_widget_;
  if (leading_ && idx-- == 0) return *leading_;
  return ChildAt(trailing_, idx, "AppBar");
}

Widget& AppBar::getChild(int idx) {
  if (!title_widget_.isGone() && idx-- == 0) return title_widget_;
  if (!subtitle_widget_.isGone() && idx-- == 0) return subtitle_widget_;
  if (leading_ && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "AppBar child index out of bounds";
  return *leading_;
}

Color AppBar::background() const {
  const auto& colors = theme().material3Theme().color;
  return surface_state_ == AppBarSurfaceState::kFlat ? colors.surface
                                                      : colors.surfaceContainer;
}

Dimensions AppBar::onMeasure(WidthSpec width, HeightSpec height) {
  const int16_t row_height = Scaled(internal::kActionTapTargetDp);
  const int16_t container_height = Scaled(containerHeightDp());
  const int16_t available_width = width.value();

  // Measure all children even when an exact app-bar width leaves them no room;
  // this preserves the normal child layout-request lifecycle.
  if (leading_ != nullptr) {
    leading_->measure(WidthSpec::AtMost(row_height), HeightSpec::Exactly(row_height));
  }
  for (Widget* slot : trailing_) {
    if (slot != nullptr) {
      slot->measure(WidthSpec::AtMost(row_height), HeightSpec::Exactly(row_height));
    }
  }
  title_widget_.measure(WidthSpec::AtMost(std::max<int16_t>(0, available_width)),
                        HeightSpec::Unspecified(container_height));
  subtitle_widget_.measure(WidthSpec::AtMost(std::max<int16_t>(0, available_width)),
                           HeightSpec::Unspecified(container_height));
  return Dimensions(width.resolveSize(available_width),
                    height.resolveSize(container_height));
}

void AppBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const int16_t edge = Scaled(internal::kAppBarEdgeInsetDp);
  const int16_t title_inset = Scaled(internal::kAppBarTitleInsetDp);
  const int16_t action_size = Scaled(internal::kActionTapTargetDp);
  const int16_t width = std::max<int16_t>(0, rect.width());
  const int16_t height = std::max<int16_t>(0, rect.height());
  const bool single_row = variant_ == AppBarVariant::kSmall;
  int16_t left = std::min<int16_t>(edge, width);
  int16_t right = std::max<int16_t>(left, width - edge);

  // Flexible bars reserve a 56dp control row at the top. Their title stack is
  // intentionally in a separate row below it, matching MediumTopAppBar and
  // LargeTopAppBar in Android's Material 3 implementation.
  const int16_t action_y = single_row
                               ? std::max<int16_t>(0, (height - action_size) / 2)
                               : edge;
  auto layout_action = [action_size, action_y](Widget* child, int16_t x) {
    if (child == nullptr) return;
    child->layout(
        Rect(x, action_y, x + action_size - 1, action_y + action_size - 1));
  };
  if (leading_ != nullptr) {
    const int16_t slot = std::min<int16_t>(action_size, right - left);
    layout_action(leading_, left);
    left += slot;
  }
  for (int index = 1; index >= 0; --index) {
    if (trailing_[index] == nullptr) continue;
    const int16_t slot = std::min<int16_t>(action_size, std::max<int16_t>(0, right - left));
    right -= slot;
    layout_action(trailing_[index], right);
  }

  // A single-row title starts at the 16dp title inset without navigation, or
  // immediately after its 48dp navigation slot. Flexible titles use the
  // second row and therefore do not reserve navigation/action width.
  if (single_row && leading_ == nullptr) {
    left = std::min<int16_t>(title_inset, right);
  }
  if (!single_row) {
    left = std::min<int16_t>(title_inset, width);
    right = std::max<int16_t>(left, width - title_inset);
  } else {
    right = std::max<int16_t>(left, right);
  }

  const int16_t lane_top = single_row ? 0 : action_size + 2 * edge;
  const int16_t lane_bottom = single_row
                                  ? height
                                  : std::max<int16_t>(lane_top,
                                                      height - Scaled(tokens().title_bottom_inset_dp));
  const int16_t lane_height = std::max<int16_t>(0, lane_bottom - lane_top);
  const int16_t lane_width = std::max<int16_t>(0, right - left);
  const bool show_subtitle = !subtitle_widget_.isGone();
  const int16_t title_height = std::min<int16_t>(
      title_widget_.measure(WidthSpec::AtMost(lane_width), HeightSpec::AtMost(lane_height)).height(),
      lane_height);
  const int16_t subtitle_height = show_subtitle
      ? std::min<int16_t>(subtitle_widget_.measure(WidthSpec::AtMost(lane_width), HeightSpec::AtMost(lane_height)).height(), lane_height - title_height)
      : 0;
  const int16_t stack_height = title_height + subtitle_height;
  const int16_t stack_top = single_row
                                ? (height - stack_height) / 2
                                : lane_bottom - stack_height;
  title_widget_.layout(Rect(left, stack_top, right - 1, stack_top + title_height - 1));
  if (show_subtitle) {
    subtitle_widget_.layout(Rect(left, stack_top + title_height, right - 1,
                                 stack_top + stack_height - 1));
  }
}

SearchBar::SearchBar(ApplicationContext& context)
    : Container(context),
      display_text_widget_(context),
      passive_search_icon_(context, ic_outlined_24_action_search()),
      leading_(nullptr),
      trailing_{nullptr, nullptr} {
  // These presentation children are stored by value and borrowed by the
  // container, avoiding heap ownership for the default search treatment.
  display_text_widget_.setUseOnSurfaceVariant(true);
  attachChild(display_text_widget_);
  attachChild(passive_search_icon_);
}

SearchBar::~SearchBar() {
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
  detachChild(&passive_search_icon_);
  detachChild(&display_text_widget_);
}

void SearchBar::setDisplayText(roo::string_view text) {
  if (display_text_ != text) {
    display_text_ = text;
    display_text_widget_.setText(text);
    invalidateInterior();
    requestLayout();
  }
}

Color SearchBar::background() const {
  return theme().material3Theme().color.surfaceContainerHigh;
}

::roo_windows::material3::ColorToken SearchBar::containerRole() const {
  return ::roo_windows::material3::ColorToken::kSurfaceContainerHigh;
}

const internal::SearchEntryTokens& SearchBar::entryTokens() const {
  return internal::kStandaloneSearchEntryTokens;
}

BorderStyle SearchBar::getBorderStyle() const {
  return BorderStyle(static_cast<uint8_t>(std::min<int16_t>(Scaled(28), 0xff)),
                     0);
}

bool SearchBar::fillTouchTargetPath(XDim x, YDim y,
                                    std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !bounds().contains(x, y)) return false;
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.parent_bounds().contains(x, y)) {
      continue;
    }
    if (AppendInteractiveChildTouchTarget(child, x, y, false, path)) {
      return true;
    }
  }
  return true;
}

bool SearchBar::fillSloppyTouchTargetPath(XDim x, YDim y,
                                          std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !getSloppyTouchBounds().contains(x, y)) {
    return false;
  }
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.getMaxSloppyTouchParentBounds().contains(x, y)) {
      continue;
    }
    const bool within_bounds = child.parent_bounds().contains(x, y);
    if (AppendInteractiveChildTouchTarget(child, x, y, !within_bounds, path)) {
      return true;
    }
  }
  return true;
}

void SearchBar::replaceSlot(Widget*& slot, WidgetRef widget) {
  Widget* incoming = widget.get();
  if (incoming == slot) return;
  if (slot) detachChild(slot);
  slot = incoming;
  if (slot) {
    CHECK(slot->parent() == nullptr);
    attachChild(std::move(widget));
  }
  invalidateInterior();
  requestLayout();
}

void SearchBar::setLeading(WidgetRef widget) {
  passive_search_icon_.setVisibility(
      widget.get() == nullptr ? Visibility::kVisible : Visibility::kGone);
  replaceSlot(leading_, std::move(widget));
}

void SearchBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int SearchBar::getChildrenCount() const {
  return 2 + (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& SearchBar::getChild(int idx) const {
  if (idx-- == 0) return display_text_widget_;
  if (idx-- == 0) return passive_search_icon_;
  if (leading_ && idx-- == 0) return *leading_;
  return ChildAt(trailing_, idx, "SearchBar");
}

Widget& SearchBar::getChild(int idx) {
  if (idx-- == 0) return display_text_widget_;
  if (idx-- == 0) return passive_search_icon_;
  if (leading_ && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "SearchBar child index out of bounds";
  return *leading_;
}

Dimensions SearchBar::onMeasure(WidthSpec width, HeightSpec height) {
  const internal::SearchEntryTokens& tokens = entryTokens();
  const int16_t edge = Scaled(tokens.edge_padding_dp);
  const int16_t gap = Scaled(tokens.slot_gap_dp);
  const int16_t row_height = Scaled(tokens.container_height_dp);
  Widget* leading = leading_ == nullptr ? &passive_search_icon_ : leading_;
  int16_t occupied = edge * 2;
  occupied += leading
                  ->measure(WidthSpec::AtMost(row_height),
                            HeightSpec::Exactly(row_height))
                  .width();
  if (!display_text_.empty()) occupied += gap;
  for (Widget* slot : trailing_) {
    if (slot == nullptr) continue;
    occupied += gap + slot->measure(WidthSpec::AtMost(row_height),
                                    HeightSpec::Exactly(row_height))
                          .width();
  }
  Dimensions text = display_text_widget_.measure(
      WidthSpec::Unspecified(0), HeightSpec::AtMost(row_height));
  const int16_t natural = occupied + text.width();
  const int16_t preferred =
      std::max<int16_t>(Scaled(240), std::min<int16_t>(Scaled(720), natural));
  return Dimensions(width.resolveSize(preferred),
                    height.resolveSize(row_height));
}

void SearchBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const internal::SearchEntryTokens& tokens = entryTokens();
  const int16_t edge = Scaled(tokens.edge_padding_dp);
  const int16_t gap = Scaled(tokens.slot_gap_dp);
  const int16_t row_height = rect.height();
  int16_t left = edge;
  int16_t right = rect.width() - edge;
  auto place = [row_height](Widget* child, int16_t x, int16_t max_width) {
    if (child == nullptr) return int16_t{0};
    Dimensions measured = child->measure(WidthSpec::AtMost(max_width),
                                         HeightSpec::AtMost(row_height));
    int16_t w = std::min<int16_t>(measured.width(), max_width);
    int16_t h = std::min<int16_t>(measured.height(), row_height);
    child->layout(
        Rect(x, (row_height - h) / 2, x + w - 1, (row_height - h) / 2 + h - 1));
    return w;
  };
  Widget* leading = leading_ == nullptr ? &passive_search_icon_ : leading_;
  left += place(leading, left, std::max<int16_t>(0, right - left)) + gap;
  for (int i = 1; i >= 0; --i) {
    Widget* slot = trailing_[i];
    if (slot == nullptr) continue;
    Dimensions measured =
        slot->measure(WidthSpec::AtMost(std::max<int16_t>(0, right - left)),
                      HeightSpec::AtMost(row_height));
    int16_t w =
        std::min<int16_t>(measured.width(), std::max<int16_t>(0, right - left));
    right -= w;
    place(slot, right, w);
    right -= gap;
  }
  int16_t text_width = std::max<int16_t>(0, right - left);
  display_text_widget_.layout(
      Rect(left, 0, left + text_width - 1, row_height - 1));
}

SearchAppBar::SearchAppBar(ApplicationContext& context)
    : Container(context),
      search_entry_(context),
      leading_(nullptr),
      trailing_{nullptr, nullptr},
      surface_state_(AppBarSurfaceState::kFlat) {
  attachChild(search_entry_);
  search_entry_.setOnInteractiveChange(
      [this]() { this->triggerInteractiveChange(); });
}

SearchAppBar::~SearchAppBar() {
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
  detachChild(&search_entry_);
}

void SearchAppBar::setSurfaceState(AppBarSurfaceState state) {
  if (surface_state_ != state) {
    surface_state_ = state;
    search_entry_.setSurfaceState(state);
    invalidateInterior();
  }
}

void SearchAppBar::setDisplayText(roo::string_view text) {
  search_entry_.setDisplayText(text);
}

void SearchAppBar::replaceSlot(Widget*& slot, WidgetRef widget) {
  Widget* incoming = widget.get();
  if (incoming == slot) return;
  if (slot) detachChild(slot);
  slot = incoming;
  if (slot) {
    CHECK(slot->parent() == nullptr);
    attachChild(std::move(widget));
  }
  invalidateInterior();
  requestLayout();
}

void SearchAppBar::setLeading(WidgetRef widget) {
  replaceSlot(leading_, std::move(widget));
}

void SearchAppBar::setInnerTrailing(uint8_t index, WidgetRef widget) {
  search_entry_.setTrailing(index, std::move(widget));
}

void SearchAppBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int SearchAppBar::getChildrenCount() const {
  return 1 + (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& SearchAppBar::getChild(int idx) const {
  if (idx-- == 0) return search_entry_;
  if (leading_ != nullptr && idx-- == 0) return *leading_;
  int n = ChildCount(trailing_);
  if (idx < n) return ChildAt(trailing_, idx, "SearchAppBar");
  LOG(FATAL) << "SearchAppBar child index out of bounds";
  return *leading_;
}

Widget& SearchAppBar::getChild(int idx) {
  if (idx-- == 0) return search_entry_;
  if (leading_ != nullptr && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "SearchAppBar child index out of bounds";
  return *leading_;
}

Color SearchAppBar::background() const {
  const auto& colors = theme().material3Theme().color;
  return surface_state_ == AppBarSurfaceState::kFlat ? colors.surface
                                                      : colors.surfaceContainer;
}

void SearchAppBar::EmbeddedSearchBar::setSurfaceState(
    AppBarSurfaceState state) {
  if (surface_state_ == state) return;
  surface_state_ = state;
  invalidateInterior();
}

Color SearchAppBar::EmbeddedSearchBar::background() const {
  const auto& colors = theme().material3Theme().color;
  return surface_state_ == AppBarSurfaceState::kFlat
             ? colors.surfaceContainer
             : colors.surfaceContainerHighest;
}

::roo_windows::material3::ColorToken
SearchAppBar::EmbeddedSearchBar::containerRole() const {
  return surface_state_ == AppBarSurfaceState::kFlat
             ? ::roo_windows::material3::ColorToken::kSurfaceContainer
             : ::roo_windows::material3::ColorToken::kSurfaceContainerHighest;
}

const internal::SearchEntryTokens&
SearchAppBar::EmbeddedSearchBar::entryTokens() const {
  return internal::kEmbeddedSearchEntryTokens;
}

Dimensions SearchAppBar::onMeasure(WidthSpec width, HeightSpec height) {
  const int16_t action_size = Scaled(internal::kActionTapTargetDp);
  const int16_t outer_height = Scaled(64);
  const int16_t entry_height =
      Scaled(internal::kEmbeddedSearchEntryTokens.container_height_dp);
  const int16_t available_width = std::max<int16_t>(0, width.value());
  if (leading_) leading_->measure(WidthSpec::AtMost(action_size),
                                  HeightSpec::Exactly(action_size));
  for (Widget* slot : trailing_) {
    if (slot) slot->measure(WidthSpec::AtMost(action_size),
                            HeightSpec::Exactly(action_size));
  }
  const int16_t outer_slots = (leading_ != nullptr) * action_size +
                              ChildCount(trailing_) * action_size;
  const int16_t entry_width = std::min<int16_t>(
      std::max<int16_t>(0, available_width - 2 * Scaled(internal::kAppBarEdgeInsetDp) -
                              outer_slots),
      Scaled(internal::kEmbeddedSearchEntryTokens.max_width_dp));
  search_entry_.measure(WidthSpec::Exactly(entry_width),
                        HeightSpec::Exactly(entry_height));
  return Dimensions(width.resolveSize(available_width),
                    height.resolveSize(outer_height));
}

void SearchAppBar::onLayout(bool changed, const Rect& rect) {
  (void)changed;
  const int16_t edge = Scaled(internal::kAppBarEdgeInsetDp);
  const int16_t action_size = Scaled(internal::kActionTapTargetDp);
  const int16_t entry_height =
      Scaled(internal::kEmbeddedSearchEntryTokens.container_height_dp);
  const int16_t width = std::max<int16_t>(0, rect.width());
  const int16_t height = std::max<int16_t>(0, rect.height());
  const int16_t action_y = std::max<int16_t>(0, (height - action_size) / 2);
  int16_t left = std::min<int16_t>(edge, width);
  int16_t right = std::max<int16_t>(left, width - edge);
  auto layout_outer = [action_size, action_y](Widget* child, int16_t x) {
    if (child) child->layout(Rect(x, action_y, x + action_size - 1,
                                  action_y + action_size - 1));
  };
  if (leading_) {
    layout_outer(leading_, left);
    left = std::min<int16_t>(right, left + action_size);
  }
  for (int i = 1; i >= 0; --i) {
    if (!trailing_[i]) continue;
    right = std::max<int16_t>(left, right - action_size);
    layout_outer(trailing_[i], right);
  }
  const int16_t available = std::max<int16_t>(0, right - left);
  const int16_t lane_width = std::min<int16_t>(
      available, Scaled(internal::kEmbeddedSearchEntryTokens.max_width_dp));
  // When the cap leaves slack, retain the central strip's reading edge rather
  // than centering the entry and making it drift away from page content.
  const int16_t lane_left = left;
  const int16_t lane_top = std::max<int16_t>(0, (height - entry_height) / 2);
  search_entry_.layout(Rect(lane_left, lane_top, lane_left + lane_width - 1,
                            lane_top + entry_height - 1));
}

bool SearchAppBar::fillTouchTargetPath(XDim x, YDim y,
                                       std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !bounds().contains(x, y)) return false;
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.parent_bounds().contains(x, y)) {
      continue;
    }
    if (AppendInteractiveChildTouchTarget(child, x, y, false, path)) {
      return true;
    }
  }
  path.pop_back();
  return false;
}

bool SearchAppBar::fillSloppyTouchTargetPath(XDim x, YDim y,
                                             std::vector<Widget*>& path) {
  if (!isVisible() || !isEnabled() || !getSloppyTouchBounds().contains(x, y)) {
    return false;
  }
  path.push_back(this);
  for (int i = getChildrenCount() - 1; i >= 0; --i) {
    Widget& child = getChild(i);
    if (!child.isVisible() || !child.isEnabled() ||
        !child.getMaxSloppyTouchParentBounds().contains(x, y)) {
      continue;
    }
    const bool within_bounds = child.parent_bounds().contains(x, y);
    if (AppendInteractiveChildTouchTarget(child, x, y, !within_bounds, path)) {
      return true;
    }
  }
  path.pop_back();
  return false;
}

}  // namespace roo_windows::material3
