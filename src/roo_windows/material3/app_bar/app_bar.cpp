#include "roo_windows/material3/app_bar/app_bar.h"

#include <algorithm>
#include <utility>

#include "roo_display/ui/alignment.h"
#include "roo_display/ui/text_label.h"
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

}  // namespace

Dimensions internal::AppBarText::getSuggestedMinimumDimensions() const {
  if (text_.empty()) return Dimensions(0, 0);
  const roo_display::Font& font = font_body1();
  return Dimensions(font.getHorizontalStringMetrics(text_).width(),
                    font.metrics().maxHeight());
}

void internal::AppBarText::paint(PaintContext& ctx) const {
  if (text_.empty()) return;
  const roo_display::Font& font = font_body1();
  ctx.canvas().drawTiled(
      roo_display::StringViewLabel(
          text_, font, theme().material3Theme().color.onSurfaceVariant),
      bounds(), roo_display::kLeft | roo_display::kMiddle);
}

AppBar::AppBar(ApplicationContext& context, AppBarVariant variant)
    : Container(context),
      leading_(nullptr),
      trailing_{nullptr, nullptr},
      variant_(variant),
      title_alignment_(AppBarTitleAlignment::kLeading),
      surface_state_(AppBarSurfaceState::kFlat) {}

AppBar::~AppBar() {
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
}

void AppBar::setVariant(AppBarVariant variant) {
  if (variant_ == variant) return;
  variant_ = variant;
  invalidateInterior();
  requestLayout();
}

void AppBar::setTitleAlignment(AppBarTitleAlignment alignment) {
  if (title_alignment_ == alignment) return;
  title_alignment_ = alignment;
  invalidateInterior();
  requestLayout();
}

void AppBar::setSurfaceState(AppBarSurfaceState state) {
  if (surface_state_ == state) return;
  surface_state_ = state;
  invalidateInterior();
}

void AppBar::setTitle(roo::string_view title) {
  if (title_ == title) return;
  title_ = title;
  invalidateInterior();
  requestLayout();
}

void AppBar::setSubtitle(roo::string_view subtitle) {
  if (subtitle_ == subtitle) return;
  subtitle_ = subtitle;
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
  return (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& AppBar::getChild(int idx) const {
  if (leading_ && idx-- == 0) return *leading_;
  return ChildAt(trailing_, idx, "AppBar");
}

Widget& AppBar::getChild(int idx) {
  if (leading_ && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "AppBar child index out of bounds";
  return *leading_;
}

SearchBar::SearchBar(ApplicationContext& context)
    : Container(context),
      display_text_widget_(context),
      passive_search_icon_(context, ic_outlined_24_action_search()),
      leading_(nullptr),
      trailing_{nullptr, nullptr} {
  // These presentation children are stored by value and borrowed by the
  // container, avoiding heap ownership for the default search treatment.
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

BorderStyle SearchBar::getBorderStyle() const {
  return BorderStyle(static_cast<uint8_t>(std::min<int16_t>(Scaled(28), 0xff)),
                     0);
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
  const int16_t edge =
      Scaled(internal::kStandaloneSearchEntryTokens.edge_padding_dp);
  const int16_t gap =
      Scaled(internal::kStandaloneSearchEntryTokens.slot_gap_dp);
  const int16_t row_height =
      Scaled(internal::kStandaloneSearchEntryTokens.container_height_dp);
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
  const int16_t edge =
      Scaled(internal::kStandaloneSearchEntryTokens.edge_padding_dp);
  const int16_t gap =
      Scaled(internal::kStandaloneSearchEntryTokens.slot_gap_dp);
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
      leading_(nullptr),
      inner_trailing_{nullptr, nullptr},
      trailing_{nullptr, nullptr},
      surface_state_(AppBarSurfaceState::kFlat) {}

SearchAppBar::~SearchAppBar() {
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  for (Widget* slot : inner_trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
}

void SearchAppBar::setSurfaceState(AppBarSurfaceState state) {
  if (surface_state_ != state) {
    surface_state_ = state;
    invalidateInterior();
  }
}

void SearchAppBar::setDisplayText(roo::string_view text) {
  if (display_text_ != text) {
    display_text_ = text;
    invalidateInterior();
    requestLayout();
  }
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
  CheckTrailingIndex(index);
  replaceSlot(inner_trailing_[index], std::move(widget));
}

void SearchAppBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int SearchAppBar::getChildrenCount() const {
  return (leading_ != nullptr) + ChildCount(inner_trailing_) +
         ChildCount(trailing_);
}

const Widget& SearchAppBar::getChild(int idx) const {
  if (leading_ != nullptr && idx-- == 0) return *leading_;
  int n = ChildCount(inner_trailing_);
  if (idx < n) return ChildAt(inner_trailing_, idx, "SearchAppBar");
  return ChildAt(trailing_, idx - n, "SearchAppBar");
}

Widget& SearchAppBar::getChild(int idx) {
  if (leading_ != nullptr && idx-- == 0) return *leading_;
  for (Widget* slot : inner_trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "SearchAppBar child index out of bounds";
  return *leading_;
}

}  // namespace roo_windows::material3
