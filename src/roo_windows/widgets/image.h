#pragma once

#include "roo_display.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/theme.h"

namespace roo_windows {

// Image is an arbitrary static content, represented as a roo_display::Drawable.
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

  void paint(const Canvas& canvas) const override;

  Dimensions getSuggestedMinimumDimensions() const;

 private:
  const roo_display::Drawable* image_;
  roo_display::Alignment alignment_;
};

}  // namespace roo_windows