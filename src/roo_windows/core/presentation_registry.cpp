#include "roo_windows/core/presentation_registry.h"

#include "roo_windows/core/application_context.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

PresentationRegistry::PresentationRegistry(ApplicationContext& context)
    : context_(context) {}

bool PresentationRegistry::observe(Widget& widget) {
  if (stopped_ || widget.tryContext() != &context_) return false;
  if (entries_.find(&widget) != entries_.end()) return true;
  entries_.insert(std::make_pair(&widget, Entry()));
  delivery_.reserve(entries_.size());
  requestReevaluation();
  return true;
}

void PresentationRegistry::unobserve(Widget& widget) {
  for (Widget*& pending : delivery_) {
    if (pending == &widget) pending = nullptr;
  }
  entries_.erase(&widget);
  if (entries_.empty() && notification_id_ >= 0) {
    context_.scheduler().cancel(notification_id_);
    notification_id_ = -1;
    pending_ = false;
  }
}

void PresentationRegistry::requestReevaluation() {
  if (stopped_ || entries_.empty() || pending_) return;
  pending_ = true;
  notification_id_ = context_.scheduler().scheduleOn(
      roo_time::Uptime::Now(), *this, roo_scheduler::PRIORITY_NORMAL);
}

void PresentationRegistry::noteSubtreeDetach(Widget& subtree) {
  for (auto& item : entries_) {
    Widget* candidate = item.first;
    for (Widget* current = candidate; current != nullptr;
         current = current->parent()) {
      if (current == &subtree) {
        item.second.detached_since_delivery = true;
        break;
      }
    }
  }
  requestReevaluation();
}

void PresentationRegistry::execute(roo_scheduler::ExecutionID id) {
  if (id != notification_id_) return;
  notification_id_ = -1;
  deliverPendingChanges();
}

void PresentationRegistry::deliverPendingChanges() {
  if (stopped_ || delivering_) return;
  if (notification_id_ >= 0) {
    context_.scheduler().cancel(notification_id_);
    notification_id_ = -1;
  }
  pending_ = false;
  delivering_ = true;
  delivery_.clear();
  delivery_.reserve(entries_.size());
  for (const auto& item : entries_) delivery_.push_back(item.first);
  for (size_t i = 0; i < delivery_.size(); ++i) {
    Widget* widget = delivery_[i];
    if (widget == nullptr) continue;
    auto it = entries_.find(widget);
    if (it == entries_.end()) continue;
    Entry& entry = it->second;
    PresentationState state = widget->presentationState();
    if (!entry.initial && entry.last_state == state &&
        !entry.detached_since_delivery) {
      continue;
    }
    PresentationChange change{state, entry.detached_since_delivery};
    entry.last_state = state;
    entry.initial = false;
    entry.detached_since_delivery = false;
    widget->onPresentationChanged(change);
  }
  delivery_.clear();
  delivering_ = false;
}

void PresentationRegistry::stop() {
  if (stopped_) return;
  stopped_ = true;
  if (notification_id_ >= 0) context_.scheduler().cancel(notification_id_);
  notification_id_ = -1;
  pending_ = false;
  delivery_.clear();
  entries_.clear();
}

}  // namespace roo_windows
