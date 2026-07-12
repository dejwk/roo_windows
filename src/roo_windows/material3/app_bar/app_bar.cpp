#include "roo_windows/material3/app_bar/app_bar.h"

#include <utility>

#include "roo_logging.h"

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
    : Container(context), leading_(nullptr), trailing_{nullptr, nullptr} {}

SearchBar::~SearchBar() {
  for (Widget* slot : trailing_) {
    if (slot) detachChild(slot);
  }
  if (leading_) detachChild(leading_);
}

void SearchBar::setDisplayText(roo::string_view text) {
  if (display_text_ != text) {
    display_text_ = text;
    invalidateInterior();
    requestLayout();
  }
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
  replaceSlot(leading_, std::move(widget));
}

void SearchBar::setTrailing(uint8_t index, WidgetRef widget) {
  CheckTrailingIndex(index);
  replaceSlot(trailing_[index], std::move(widget));
}

int SearchBar::getChildrenCount() const {
  return (leading_ != nullptr) + ChildCount(trailing_);
}

const Widget& SearchBar::getChild(int idx) const {
  if (leading_ && idx-- == 0) return *leading_;
  return ChildAt(trailing_, idx, "SearchBar");
}

Widget& SearchBar::getChild(int idx) {
  if (leading_ && idx-- == 0) return *leading_;
  for (Widget* slot : trailing_) {
    if (slot && idx-- == 0) return *slot;
  }
  LOG(FATAL) << "SearchBar child index out of bounds";
  return *leading_;
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
