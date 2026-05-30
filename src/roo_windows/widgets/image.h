#pragma once

#include "roo_display.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

// Image is an arbitrary static content, represented as a roo_display::Drawable.
//
// Note: once set, the anchor extents of the underlying drawable should remain
// constant. If they do change, however, call requestLayout() to make the image
// aware of the changed dimensions.
class Image : public BasicWidget {
 public:
  // Creates an empty image. You can set the content after construction by
  // calling setImage().
  Image(const Environment& env,
        roo_display::Alignment alignment = roo_display::kCenter |
                                           roo_display::kMiddle)
      : BasicWidget(env), image_(nullptr), alignment_(alignment) {}

  // Creates an image with the specified content, and optionally the alignment.
  // If the alignment is not specified, it defaults to centered (kCenter |
  // kMiddle).
  Image(const Environment& env, const roo_display::Drawable& image,
        roo_display::Alignment alignment = roo_display::kCenter |
                                           roo_display::kMiddle)
      : BasicWidget(env), image_(&image), alignment_(alignment) {}

  // Sets the new content and invalidates the interior if needed so that it gets
  // redrawn. If nullptr, the image is drawn as an empty canvas.
  void setImage(const roo_display::Drawable* image);

  // Sets the new alignment and invalidates the interior if needed so that it
  // gets redrawn.
  void setAlignment(roo_display::Alignment alignment);

  /// Paints the underlying drawable at the configured alignment, clearing to
  /// the background for any uncovered area.
  void paint(PaintContext& ctx) const override;

  /// Computes ink insets so the widget's visual extents follow the drawable's
  /// anchor extents inside the laid-out bounds.
  Insets getInkInsets() const override;

  /// Reports the drawable's natural anchor extents as the minimum size.
  Dimensions getSuggestedMinimumDimensions() const override;

 private:
  const roo_display::Drawable* image_;
  roo_display::Alignment alignment_;
};

}  // namespace roo_windows