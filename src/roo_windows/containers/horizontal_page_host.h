#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "roo_scheduler.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/widget_ref.h"

namespace roo_windows {

/// Viewport-sized host that keeps one selected content page active.
///
/// Phase 1 behavior supports programmatic page selection and current-page-only
/// layout. Swipe and settle animation hooks are reserved for later phases.
class HorizontalPageHost : public Container,
                           private roo_scheduler::Executable {
 public:
  /// Creates an empty page host.
  explicit HorizontalPageHost(ApplicationContext& context);

  ~HorizontalPageHost() override;

  /// Appends a page to the host's backing page list.
  ///
  /// The first page added becomes the current page.
  void addPage(WidgetRef page);

  /// Removes all pages and detaches any active children.
  void clearPages();

  /// Returns the number of stored pages.
  int pageCount() const;

  /// Returns the currently settled page index, or -1 when empty.
  int currentIndex() const;

  /// Selects a new current page.
  ///
  /// Returns false if the index is out of range or already selected.
  /// In phase 1, `animate = true` logs an unimplemented warning and snaps.
  bool setCurrentIndex(int index, bool animate = true);

  /// Page host contributes no local surface paint in phase 1.
  void paint(PaintContext& ctx) const override;

 protected:
  /// Hook called after the settled page index changes.
  virtual void onSettledIndexChanged(int old_index, int new_index);

  /// Measures as one viewport, not a strip width.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Lays out the active current page to fill the viewport.
  void onLayout(bool changed, const Rect& rect) override;

  int getChildrenCount() const override;
  Widget& getChild(int idx) override;
  const Widget& getChild(int idx) const override;

 private:
  enum SlotId : uint8_t {
    kPreviousSlot = 0,
    kCurrentSlot = 1,
    kNextSlot = 2,
    kSlotCount = 3,
  };

  struct ActiveSlot {
    Widget* widget = nullptr;
    int page_index = -1;
  };

  void execute(roo_scheduler::ExecutionID id) override;

  /// Binds slots to pages according to the current settled index.
  void syncActiveSlots();

  /// Assigns one slot to one page using a borrowed child attachment.
  void bindSlotToPage(SlotId slot_id, int page_index);

  /// Detaches the current page from the specified slot.
  void clearSlot(SlotId slot_id);

  /// Returns the idx-th active child in fixed slot order.
  Widget* activeChildAt(int idx);
  const Widget* activeChildAt(int idx) const;

  std::vector<WidgetRef> pages_;
  std::vector<int8_t> page_to_slot_;
  std::array<ActiveSlot, kSlotCount> active_slots_;

  int settled_index_;
  float page_position_;
};

}  // namespace roo_windows