#include "roo_windows/containers/blit_cache_container.h"

#include <algorithm>

#include "roo_logging.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/clipper.h"

namespace roo_windows {

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
    auto old_safe = blit_safe_region_;
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

  // The blit_safe_region_ is a band (full-width or full-height).
  // If the dirty rect overlaps it, trim from the nearest edge, or keep the
  // larger remaining portion if it's in the middle.
  int16_t sy0 = blit_safe_region_.yMin();
  int16_t sy1 = blit_safe_region_.yMax();
  int16_t sx0 = blit_safe_region_.xMin();
  int16_t sx1 = blit_safe_region_.xMax();

  // Check if dirty rect overlaps the safe region at all.
  if (dirty.xMax() < sx0 || dirty.xMin() > sx1 || dirty.yMax() < sy0 ||
      dirty.yMin() > sy1) {
    return;  // No overlap.
  }

  // For a full-width band, trim in the Y dimension.
  if (sx0 == blit_safe_region_.xMin() && sx1 == blit_safe_region_.xMax()) {
    int32_t top_portion = dirty.yMin() - sy0;
    int32_t bottom_portion = sy1 - dirty.yMax();

    if (top_portion <= 0 && bottom_portion <= 0) {
      // Dirty rect covers the entire band.
      blit_safe_region_ = roo_display::Box(0, 0, -1, -1);
    } else if (top_portion >= bottom_portion) {
      // Keep the top portion.
      blit_safe_region_ = roo_display::Box(sx0, sy0, sx1, dirty.yMin() - 1);
    } else {
      // Keep the bottom portion.
      blit_safe_region_ = roo_display::Box(sx0, dirty.yMax() + 1, sx1, sy1);
    }
  }
}

roo_display::Box BlitCacheContainer::computeCleanBand(
    const roo_display::Box& panel_device,
    const std::vector<roo_display::Box>& excl, size_t excl_count,
    bool vertical) const {
  if (vertical) {
    // Find the largest contiguous full-width Y band not overlapping any
    // exclusion's Y projection (for exclusions that overlap the panel's X
    // range).
    int16_t band_y0 = panel_device.yMin();
    int16_t band_y1 = panel_device.yMax();

    // Collect Y-splits from exclusions that overlap our X range.
    // Use a small inline buffer to avoid allocation.
    int16_t splits[16];
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
      if (n_splits < 14) {
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

    int16_t splits[16];
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
      if (n_splits < 14) {
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
  int16_t saved_dx = 0;
  int32_t saved_dy = 0;
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
            unsigned long blit_t0 = micros();
            raw.blitCopy(usable_src.xMin(), usable_src.yMin(),
                         usable_src.xMax(), usable_src.yMax(),
                         usable_dest.xMin(), usable_dest.yMin());
            unsigned long blit_t1 = micros();
            LOG(INFO) << "BlitCache: blitCopy " << (blit_t1 - blit_t0)
                      << " us, area=" << usable_area;
            // Mark the blitted destination as an exclusion so the
            // subsequent child paint skips the already-correct pixels.
            clipper.addExclusion(usable_dest);
            blit_dest = usable_dest;
            blit_panel = panel_device;
            saved_dx = pending_dx_;
            saved_dy = pending_dy_;
            did_blit = true;
          }
        }
      }
    }

    has_pending_blit_ = false;
    pending_dx_ = 0;
    pending_dy_ = 0;
  }

  // Delegate to standard Container paint.  After a successful blit, clip the
  // canvas to only the revealed strip so that widgets fully within the blitted
  // area short-circuit immediately (empty clip box → markCleanDescending).
  Canvas paint_canvas(canvas);
  if (did_blit) {
    if (saved_dx == 0 && saved_dy < 0) {
      // Scrolled up: new content revealed at bottom.
      paint_canvas.clip(Box(blit_panel.xMin(), blit_dest.yMax() + 1,
                            blit_panel.xMax(), blit_panel.yMax()));
    } else if (saved_dx == 0 && saved_dy > 0) {
      // Scrolled down: new content revealed at top.
      paint_canvas.clip(Box(blit_panel.xMin(), blit_panel.yMin(),
                            blit_panel.xMax(), blit_dest.yMin() - 1));
    } else if (saved_dy == 0 && saved_dx < 0) {
      // Scrolled left: new content revealed at right.
      paint_canvas.clip(Box(blit_dest.xMax() + 1, blit_panel.yMin(),
                            blit_panel.xMax(), blit_panel.yMax()));
    } else if (saved_dy == 0 && saved_dx > 0) {
      // Scrolled right: new content revealed at left.
      paint_canvas.clip(Box(blit_panel.xMin(), blit_panel.yMin(),
                            blit_dest.xMin() - 1, blit_panel.yMax()));
    }
    // Diagonal scroll: no extra clipping (full paint fallback).
  }
  Container::paintWidgetContents(paint_canvas, clipper);

  // After painting, update blit_safe_region_ for next frame.
  if (blit_supported_ == 1) {
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
