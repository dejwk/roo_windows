#include "roo_windows/material3/list/list.h"

#include "roo_logging.h"

namespace roo_windows {
namespace material3 {

StandardListItemInit StandardListItemInit::OneLine(
    roo_display::StringView headline, Widget* leading, Widget* trailing) {
  StandardListItemInit init;
  init.headline = headline;
  init.leading = leading;
  init.trailing = trailing;
  return init;
}

StandardListItemInit StandardListItemInit::TwoLine(
    roo_display::StringView headline, roo_display::StringView supporting,
    Widget* leading, Widget* trailing) {
  StandardListItemInit init = OneLine(headline, leading, trailing);
  init.supporting = supporting;
  return init;
}

StandardListItemInit StandardListItemInit::ThreeLine(
    roo_display::StringView headline, roo_display::StringView supporting,
    roo_display::StringView overline, Widget* leading, Widget* trailing,
    Widget* body) {
  StandardListItemInit init = TwoLine(headline, supporting, leading, trailing);
  init.overline = overline;
  init.body = body;
  init.supporting_policy.max_lines = overline.empty() ? 2 : 1;
  init.leading_alignment = VerticalVisualAlignment::kTop;
  init.trailing_alignment = VerticalVisualAlignment::kTop;
  init.prefer_top_text_alignment = true;
  return init;
}

ExpandablePanel::ExpandablePanel(const Environment& env)
    : Container(env),
      content_(),
      animation_duration_millis_(0),
      animation_progress_millis_(0),
      expanded_(false) {}

void ExpandablePanel::setContent(WidgetRef content) {
  (void)content;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::setContent";
}

void ExpandablePanel::clearContent() {
  if (content_.get() == nullptr) return;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::clearContent";
}

void ExpandablePanel::setExpanded(bool expanded, bool animate) {
  (void)animate;
  if (!expanded && !expanded_) return;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::setExpanded";
}

bool ExpandablePanel::isExpanded() const { return expanded_; }

bool ExpandablePanel::isAnimating() const {
  (void)animation_progress_millis_;
  return false;
}

void ExpandablePanel::setAnimationDuration(uint16_t millis) {
  animation_duration_millis_ = millis;
}

int ExpandablePanel::getChildrenCount() const { return 0; }

const Widget& ExpandablePanel::getChild(int idx) const {
  (void)idx;
  LOG(FATAL) << "Unimplemented: material3::ExpandablePanel::getChild";
  return *static_cast<const Widget*>(nullptr);
}

Widget& ExpandablePanel::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const ExpandablePanel&>(*this).getChild(idx));
}

ListEntry::ListEntry(const Environment& env)
    : Container(env),
      item_(nullptr),
      leading_child_(nullptr),
      trailing_child_(nullptr),
      body_child_(nullptr),
      visual_context_() {}

bool ListEntry::hasItem() const { return item_ != nullptr; }

ListItem* ListEntry::item() { return item_; }

const ListItem* ListEntry::item() const { return item_; }

void ListEntry::setItem(ListItem& item) {
  (void)item;
  LOG(FATAL) << "Unimplemented: material3::ListEntry::setItem";
}

void ListEntry::clearItem() {
  if (item_ == nullptr) return;
  LOG(FATAL) << "Unimplemented: material3::ListEntry::clearItem";
}

void ListEntry::refreshFromItem() {
  if (item_ == nullptr) return;
  LOG(FATAL) << "Unimplemented: material3::ListEntry::refreshFromItem";
}

void ListEntry::setVisualContext(const ListEntryVisualContext& context) {
  visual_context_ = context;
  invalidateInterior();
}

const ListEntryVisualContext& ListEntry::visualContext() const {
  return visual_context_;
}

int ListEntry::getChildrenCount() const { return 0; }

const Widget& ListEntry::getChild(int idx) const {
  (void)idx;
  LOG(FATAL) << "Unimplemented: material3::ListEntry::getChild";
  return *static_cast<const Widget*>(nullptr);
}

Widget& ListEntry::getChild(int idx) {
  return const_cast<Widget&>(
      static_cast<const ListEntry&>(*this).getChild(idx));
}

StandardListItem::StandardListItem(const StandardListItemInit& init)
    : overline_(init.overline),
      headline_(init.headline),
      supporting_(init.supporting),
      overline_policy_(init.overline_policy),
      headline_policy_(init.headline_policy),
      supporting_policy_(init.supporting_policy),
      leading_(init.leading),
      trailing_(init.trailing),
      body_(init.body),
      divider_inset_hint_(init.divider_inset_hint),
      leading_alignment_(static_cast<uint8_t>(init.leading_alignment)),
      trailing_alignment_(static_cast<uint8_t>(init.trailing_alignment)),
      prefer_top_text_alignment_(init.prefer_top_text_alignment) {}

roo_display::StringView StandardListItem::overlineText() const {
  return overline_;
}

roo_display::StringView StandardListItem::headlineText() const {
  return headline_;
}

roo_display::StringView StandardListItem::supportingText() const {
  return supporting_;
}

ListTextPolicy StandardListItem::overlinePolicy() const {
  return overline_policy_;
}

ListTextPolicy StandardListItem::headlinePolicy() const {
  return headline_policy_;
}

ListTextPolicy StandardListItem::supportingPolicy() const {
  return supporting_policy_;
}

Widget* StandardListItem::leading() const { return leading_; }

Widget* StandardListItem::trailing() const { return trailing_; }

Widget* StandardListItem::body() const { return body_; }

VerticalVisualAlignment StandardListItem::leadingAlignment() const {
  return static_cast<VerticalVisualAlignment>(leading_alignment_);
}

VerticalVisualAlignment StandardListItem::trailingAlignment() const {
  return static_cast<VerticalVisualAlignment>(trailing_alignment_);
}

DividerInsetHint StandardListItem::dividerInsetHint() const {
  return divider_inset_hint_;
}

bool StandardListItem::preferTopTextAlignment() const {
  return prefer_top_text_alignment_;
}

Widget* StandardListItem::leadingWidget() { return leading_; }

Widget* StandardListItem::trailingWidget() { return trailing_; }

Widget* StandardListItem::bodyWidget() { return body_; }

List::List(const Environment& env)
    : Container(env),
      entries_(),
      variant_(ListVariant::kExpressive),
      style_(ListStyle::kStandard),
      selection_policy_(),
      divider_policy_() {}

List::~List() {
  for (Widget* entry : entries_) {
    if (entry->isOwnedByParent()) delete entry;
  }
}

void List::setVariant(ListVariant variant) {
  if (variant_ == variant) return;
  variant_ = variant;
  invalidateInterior();
}

void List::setStyle(ListStyle style) {
  if (style_ == style) return;
  style_ = style;
  invalidateInterior();
}

void List::setSelectionPolicy(const ListSelectionPolicy& policy) {
  selection_policy_ = policy;
  invalidateInterior();
}

void List::setDividerPolicy(const ListDividerPolicy& policy) {
  divider_policy_ = policy;
  invalidateInterior();
}

void List::add(ListEntry& entry) {
  (void)entry;
  LOG(FATAL) << "Unimplemented: material3::List::add(ListEntry&)";
}

void List::add(std::unique_ptr<ListEntry> entry) {
  (void)entry;
  LOG(FATAL) << "Unimplemented: material3::List::add(unique_ptr<ListEntry>)";
}

void List::clear() {
  if (entries_.empty()) return;
  LOG(FATAL) << "Unimplemented: material3::List::clear";
}

int List::getChildrenCount() const { return entries_.size(); }

const Widget& List::getChild(int idx) const {
  CHECK(idx >= 0);
  CHECK_LT(idx, (int)entries_.size());
  return *entries_[idx];
}

Widget& List::getChild(int idx) {
  return const_cast<Widget&>(static_cast<const List&>(*this).getChild(idx));
}

}  // namespace material3
}  // namespace roo_windows