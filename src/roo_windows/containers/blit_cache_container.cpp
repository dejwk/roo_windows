#include "roo_windows/containers/blit_cache_container.h"

#include <algorithm>

#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"

namespace roo_windows {

namespace {

// Computes a single-rectangle repaint region for (panel - copied).
//
// Arguments:
// - panel: The full panel area in device coordinates that may need repaint.
// - copied: The destination rectangle that has already been fixed by blitCopy
//   and should therefore be excluded from repaint. It is expected to be in the
//   same coordinate space as `panel`, and to be fully contained within
//   `panel`.
// - out_repaint: Output parameter. Set to the repaint rectangle only when this
//   function returns true.
//
// Return value:
// - true: (`panel` - `copied`) can be represented as one rectangle;
//   `out_repaint` is filled with that rectangle. This happens when copied is
//   empty, or when it is a full-width strip anchored to top/bottom, or a
//   full-height strip anchored to left/right.
// - false: The uncovered region is empty or non-rectangular (e.g. interior
//   hole/L-shape). In this case, a smaller single rectangular clip is not
//   meaningful; caller should clip to `panel` and add `copied` as an
//   exclusion.
bool computeSingleRectRemainder(const roo_display::Box& panel,
                                const roo_display::Box& copied,
                                roo_display::Box& out_repaint) {
  if (panel.empty()) return false;
  if (copied.empty()) {
    out_repaint = panel;
    return true;
  }

  // If `copied` spans full width, the repaint is rectangular only when the
  // `copied` rect is anchored to top or bottom (single horizontal strip).
  if (copied.xMin() == panel.xMin() && copied.xMax() == panel.xMax()) {
    if (copied.yMin() == panel.yMin()) {
      if (copied.yMax() == panel.yMax()) {
        out_repaint = roo_display::Box(0, 0, -1, -1);
        return true;
      }
      out_repaint = roo_display::Box(panel.xMin(), copied.yMax() + 1,
                                     panel.xMax(), panel.yMax());
      return !out_repaint.empty();
    }
    if (copied.yMax() == panel.yMax()) {
      out_repaint = roo_display::Box(panel.xMin(), panel.yMin(), panel.xMax(),
                                     copied.yMin() - 1);
      return !out_repaint.empty();
    }
    return false;
  }

  // If `copied` spans full height, the repaint is rectangular only when the
  // `copied` rect is anchored to left or right (single vertical strip).
  if (copied.yMin() == panel.yMin() && copied.yMax() == panel.yMax()) {
    if (copied.xMin() == panel.xMin()) {
      out_repaint = roo_display::Box(copied.xMax() + 1, panel.yMin(),
                                     panel.xMax(), panel.yMax());
      return !out_repaint.empty();
    }
    if (copied.xMax() == panel.xMax()) {
      out_repaint = roo_display::Box(panel.xMin(), panel.yMin(),
                                     copied.xMin() - 1, panel.yMax());
      return !out_repaint.empty();
    }
    return false;
  }

  return false;
}

}  // namespace

BlitCacheContainer::BlitCacheContainer(const Environment& env)
    : Container(env),
      child_(nullptr),
      blit_safe_region_(0, 0, -1, -1),
      pending_dx_(0),
      pending_dy_(0),
      has_pending_blit_(false),
      moving_(false),
      blit_supported_(-1) {}

void BlitCacheContainer::setChild(WidgetRef child) {
  if (child_ != nullptr) {
    detachChild(child_);
  }
  child_ = child.get();
  if (child_ != nullptr) {
    attachChild(std::move(child));
  }
  blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
  pending_dx_ = 0;
  pending_dy_ = 0;
  has_pending_blit_ = false;
  blit_supported_ = -1;
}

void BlitCacheContainer::clearChild() {
  if (child_ != nullptr) {
    detachChild(child_);
    child_ = nullptr;
  }
  blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
  pending_dx_ = 0;
  pending_dy_ = 0;
  has_pending_blit_ = false;
}

PreferredSize BlitCacheContainer::getPreferredSize() const {
  if (child_ == nullptr) {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }
  return child_->getPreferredSize();
}

Dimensions BlitCacheContainer::onMeasure(WidthSpec width, HeightSpec height) {
  if (child_ == nullptr) {
    return Dimensions(width.resolveSize(0), height.resolveSize(0));
  }
  return child_->measure(width, height);
}

void BlitCacheContainer::onLayout(bool changed, const Rect& rect) {
  if (child_ == nullptr) return;
  child_->layout(Rect(0, 0, rect.width() - 1, rect.height() - 1));
}

void BlitCacheContainer::moveTo(const Rect& new_bounds) {
  int16_t dx = new_bounds.xMin() - parent_bounds().xMin();
  int32_t dy = new_bounds.yMin() - parent_bounds().yMin();

  moving_ = true;
  Container::moveTo(new_bounds);
  moving_ = false;

  if (dx == 0 && dy == 0) return;
  if (blit_supported_ == 0) {
    return;
  }
  if (blit_safe_region_.empty()) {
    return;
  }

  pending_dx_ += dx;
  pending_dy_ += dy;
  has_pending_blit_ = true;
}

void BlitCacheContainer::invalidateDescending() {
  if (!moving_) {
    blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::invalidateDescending();
}

void BlitCacheContainer::invalidateDescending(const Rect& rect) {
  if (!moving_) {
    shrinkSafeRegion(rect);
  }
  Container::invalidateDescending(rect);
}

void BlitCacheContainer::propagateDirty(const Widget* child, const Rect& rect) {
  if (!moving_ && child == child_) {
    shrinkSafeRegion(rect);
  }
  Container::propagateDirty(child, rect);
}

void BlitCacheContainer::childHidden(const Widget* child) {
  if (!moving_) {
    blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::childHidden(child);
}

void BlitCacheContainer::childShown(const Widget* child) {
  if (!moving_) {
    blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }
  Container::childShown(child);
}

void BlitCacheContainer::shrinkSafeRegion(const Rect& dirty) {
  if (blit_safe_region_.empty()) return;

  // blit_safe_region_ is kept in device coordinates, while dirty is local.
  XDim abs_x = 0;
  YDim abs_y = 0;
  getAbsoluteOffset(abs_x, abs_y);
  Rect dirty_dev = dirty.translate(abs_x, abs_y);

  // During pending blit, the safe region still refers to the source frame
  // (pre-move) device coordinates. Map dirty rect back into that frame before
  // intersecting/trimming, otherwise we may falsely keep stale source pixels.
  if (has_pending_blit_) {
    dirty_dev = dirty_dev.translate(-pending_dx_, -pending_dy_);
  }

  // Keep the safe region as a single rectangle by subtracting dirty_dev and
  // retaining the largest remaining rectangle.
  int16_t sy0 = blit_safe_region_.yMin();
  int16_t sy1 = blit_safe_region_.yMax();
  int16_t sx0 = blit_safe_region_.xMin();
  int16_t sx1 = blit_safe_region_.xMax();

  roo_display::Box overlap =
      roo_display::Box::Intersect(blit_safe_region_, dirty_dev.asBox());
  if (overlap.empty()) {
    return;  // No overlap.
  }

  roo_display::Box candidates[4] = {
      roo_display::Box(sx0, sy0, sx1, overlap.yMin() - 1),
      roo_display::Box(sx0, overlap.yMax() + 1, sx1, sy1),
      roo_display::Box(sx0, sy0, overlap.xMin() - 1, sy1),
      roo_display::Box(overlap.xMax() + 1, sy0, sx1, sy1),
  };

  roo_display::Box best(0, 0, -1, -1);
  int32_t best_area = 0;
  for (const auto& c : candidates) {
    if (c.empty()) continue;
    int32_t area = (int32_t)c.width() * c.height();
    if (area > best_area) {
      best_area = area;
      best = c;
    }
  }

  blit_safe_region_ = best;
}

roo_display::Box BlitCacheContainer::computeCleanBand(
    const roo_display::Box& panel_device,
    const std::vector<roo_display::Box>& excl, size_t excl_count,
    bool vertical) const {
  static constexpr int kMaxSplits = 128;

  if (vertical) {
    // Find the largest contiguous full-width Y band not overlapping any
    // exclusion's Y projection (for exclusions that overlap the panel's X
    // range).
    int16_t band_y0 = panel_device.yMin();
    int16_t band_y1 = panel_device.yMax();

    // Collect Y-splits from exclusions that overlap our X range.
    // Use a small inline buffer to avoid allocation.
    int16_t splits[kMaxSplits];
    int n_splits = 0;
    splits[n_splits++] = band_y0;
    for (size_t ei = 0; ei < excl_count; ++ei) {
      const auto& e = excl[ei];
      if (e.xMax() < panel_device.xMin() || e.xMin() > panel_device.xMax()) {
        continue;  // Exclusion doesn't overlap our X range.
      }
      if (e.yMax() < band_y0 || e.yMin() > band_y1) {
        continue;  // Exclusion doesn't overlap our Y range.
      }
      if (n_splits + 2 < kMaxSplits) {
        splits[n_splits++] = e.yMin();
        splits[n_splits++] = e.yMax() + 1;
      } else {
        // Too many exclusions; bail out.
        return roo_display::Box(0, 0, -1, -1);
      }
    }
    splits[n_splits++] = band_y1 + 1;

    // Sort splits.
    std::sort(splits, splits + n_splits);

    // Find the largest gap between consecutive splits that doesn't contain
    // any exclusion.
    roo_display::Box best(0, 0, -1, -1);
    for (int i = 0; i < n_splits - 1; ++i) {
      int16_t seg_y0 = std::max(splits[i], band_y0);
      int16_t seg_y1 = std::min<int16_t>(splits[i + 1] - 1, band_y1);
      if (seg_y1 < seg_y0) continue;

      // Check if this segment is free of exclusions.
      bool blocked = false;
      for (size_t ei = 0; ei < excl_count; ++ei) {
        const auto& e = excl[ei];
        if (e.xMax() < panel_device.xMin() || e.xMin() > panel_device.xMax()) {
          continue;
        }
        if (e.yMin() <= seg_y1 && e.yMax() >= seg_y0) {
          blocked = true;
          break;
        }
      }
      if (blocked) continue;

      int32_t seg_height = seg_y1 - seg_y0 + 1;
      int32_t best_height = best.empty() ? 0 : (best.yMax() - best.yMin() + 1);
      if (seg_height > best_height) {
        best = roo_display::Box(panel_device.xMin(), seg_y0,
                                panel_device.xMax(), seg_y1);
      }
    }
    return best;
  } else {
    // Horizontal: find the largest full-height X band.
    int16_t band_x0 = panel_device.xMin();
    int16_t band_x1 = panel_device.xMax();

    int16_t splits[kMaxSplits];
    int n_splits = 0;
    splits[n_splits++] = band_x0;
    for (size_t ei = 0; ei < excl_count; ++ei) {
      const auto& e = excl[ei];
      if (e.yMax() < panel_device.yMin() || e.yMin() > panel_device.yMax()) {
        continue;
      }
      if (e.xMax() < band_x0 || e.xMin() > band_x1) {
        continue;
      }
      if (n_splits + 2 < kMaxSplits) {
        splits[n_splits++] = e.xMin();
        splits[n_splits++] = e.xMax() + 1;
      } else {
        return roo_display::Box(0, 0, -1, -1);
      }
    }
    splits[n_splits++] = band_x1 + 1;

    std::sort(splits, splits + n_splits);

    roo_display::Box best(0, 0, -1, -1);
    for (int i = 0; i < n_splits - 1; ++i) {
      int16_t seg_x0 = std::max(splits[i], band_x0);
      int16_t seg_x1 = std::min<int16_t>(splits[i + 1] - 1, band_x1);
      if (seg_x1 < seg_x0) continue;

      bool blocked = false;
      for (size_t ei = 0; ei < excl_count; ++ei) {
        const auto& e = excl[ei];
        if (e.yMax() < panel_device.yMin() || e.yMin() > panel_device.yMax()) {
          continue;
        }
        if (e.xMin() <= seg_x1 && e.xMax() >= seg_x0) {
          blocked = true;
          break;
        }
      }
      if (blocked) continue;

      int32_t seg_width = seg_x1 - seg_x0 + 1;
      int32_t best_width = best.empty() ? 0 : (best.xMax() - best.xMin() + 1);
      if (seg_width > best_width) {
        best = roo_display::Box(seg_x0, panel_device.yMin(), seg_x1,
                                panel_device.yMax());
      }
    }
    return best;
  }
}

void BlitCacheContainer::paintWidgetContents(const Canvas& canvas,
                                             Clipper& clipper) {
  using roo_display::Box;

  // On first paint, discover blit capability.
  if (blit_supported_ < 0) {
    blit_supported_ =
        clipper.rawOut().getCapabilities().supportsBlitCopy() ? 1 : 0;
  }

  // Capture the exclusion count before the blit and child paint, so that the
  // post-paint safe region computation ignores exclusions added by both the
  // blit and the child's finalizePaintWidget.
  size_t pre_paint_excl_count = clipper.exclusions().size();

  bool did_blit = false;
  Box blit_dest;
  Box blit_panel;
  if (!has_pending_blit_) {
    // No moveTo since last paint; nothing to blit.
  } else if (blit_supported_ != 1) {
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  } else if (blit_safe_region_.empty()) {
    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  } else {
    // Compute the panel's visible device-coordinate rect.  We clip to
    // canvas.clip_box() so that the off-screen portion (from scrolling) is
    // excluded.
    Box panel_device = Box::Intersect(
        Box(bounds().xMin() + canvas.dx(), bounds().yMin() + canvas.dy(),
            bounds().xMax() + canvas.dx(), bounds().yMax() + canvas.dy()),
        canvas.clip_box());

    bool is_vertical = (pending_dy_ != 0);

    // Compute the current clean band from clipper exclusions.
    Box curr_clean = computeCleanBand(panel_device, clipper.exclusions(),
                                      clipper.exclusions().size(), is_vertical);

    if (!curr_clean.empty()) {
      // The usable source: intersection of prev safe region and current clean
      // band.
      Box source = Box::Intersect(blit_safe_region_, curr_clean);

      if (!source.empty()) {
        // Compute destination: source shifted by the pending delta.
        Box dest(source.xMin() + pending_dx_, source.yMin() + pending_dy_,
                 source.xMax() + pending_dx_, source.yMax() + pending_dy_);

        // Clip destination to the panel (pixels outside are not useful).
        Box usable_dest = Box::Intersect(dest, panel_device);

        if (!usable_dest.empty()) {
          // Derive the usable source back from the clipped destination.
          Box usable_src(usable_dest.xMin() - pending_dx_,
                         usable_dest.yMin() - pending_dy_,
                         usable_dest.xMax() - pending_dx_,
                         usable_dest.yMax() - pending_dy_);

          int32_t usable_area =
              (int32_t)(usable_dest.xMax() - usable_dest.xMin() + 1) *
              (usable_dest.yMax() - usable_dest.yMin() + 1);
          int32_t panel_area =
              (int32_t)(panel_device.xMax() - panel_device.xMin() + 1) *
              (panel_device.yMax() - panel_device.yMin() + 1);

          if (usable_area > panel_area / 2) {
            // Execute the blit on the raw device.
            roo_display::DisplayOutput& raw = clipper.rawOut();
            raw.blitCopy(usable_src.xMin(), usable_src.yMin(),
                         usable_src.xMax(), usable_src.yMax(),
                         usable_dest.xMin(), usable_dest.yMin());
            // Mark the blitted destination as an exclusion so the
            // subsequent child paint can skip the already-correct pixels
            // when we cannot express repaint as a single rectangular clip.
            blit_dest = usable_dest;
            blit_panel = panel_device;
            did_blit = true;
          }
        }
      }
    }

    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }

  // Delegate to standard Container paint. After a successful blit, prefer a
  // rectangular repaint clip that omits the copied destination. If that is not
  // representable by a single rectangle, fall back to panel clip + exclusion.
  Canvas paint_canvas(canvas);
  if (did_blit) {
    Box repaint;
    if (computeSingleRectRemainder(blit_panel, blit_dest, repaint)) {
      paint_canvas.clip(repaint);
    } else {
      paint_canvas.clip(blit_panel);
      clipper.addExclusion(blit_dest);
    }
  }
  Container::paintWidgetContents(paint_canvas, clipper);

  if (clipper.isDeadlineExceeded()) {
    // Preserve incremental frame recovery but disable blit reuse after an
    // aborted frame; the exclusions/content set may be only partially updated.
    blit_safe_region_ = Box(0, 0, -1, -1);
    return;
  }

  // After painting, update blit_safe_region_ for next frame.
  // For non-blit paints with an existing safe region, shrinkSafeRegion()
  // already accounted for dirty areas. Recomputing from the current (possibly
  // narrow) clip can collapse safe region to the dirty stripe.
  if (blit_supported_ == 1 && (did_blit || blit_safe_region_.empty())) {
    Box panel_device = Box::Intersect(
        Box(bounds().xMin() + canvas.dx(), bounds().yMin() + canvas.dy(),
            bounds().xMax() + canvas.dx(), bounds().yMax() + canvas.dy()),
        canvas.clip_box());
    // Use vertical band by default; this covers the common scrolling case.
    blit_safe_region_ = computeCleanBand(panel_device, clipper.exclusions(),
                                         pre_paint_excl_count, true);
    // If horizontal band would be larger, use that instead.
    Box hband = computeCleanBand(panel_device, clipper.exclusions(),
                                 pre_paint_excl_count, false);
    int32_t varea =
        blit_safe_region_.empty()
            ? 0
            : (int32_t)blit_safe_region_.width() * blit_safe_region_.height();
    int32_t harea = hband.empty() ? 0 : (int32_t)hband.width() * hband.height();
    if (harea > varea) {
      blit_safe_region_ = hband;
    }
  }
}

}  // namespace roo_windows
