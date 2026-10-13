#pragma once

// Shared synthetic output-pass scenes for host and Makerfabs ESP32-S3 probes.
#include <array>
#include <cstdint>

#include "roo_display/core/device.h"
#include "roo_windows/core/damage_region.h"

namespace roo_windows::benchmarks {

/// Source output operation used by one synthetic scene.
enum class DrawKind : uint8_t {
  kFill,
  kSparsePixels,
  kStreamedImage,
  kDirectImage,
};

/// Fixed damage geometry and output operation shared by host and target.
struct NativeDamageScene {
  const char* name;
  DrawKind draw;
  std::array<roo_display::Box, 5> damage;
  uint8_t damage_count;
  bool semantic_rect;
  bool rounded_semantic;
};

/// Returns fixed target-size scenes spanning controls and fragmented output.
inline const std::array<NativeDamageScene, 11>& NativeDamageScenes() {
  using roo_display::Box;
  const Box empty(0, 0, -1, -1);
  static const std::array<NativeDamageScene, 11> scenes = {{
      {"single_fill",
       DrawKind::kFill,
       {Box(200, 100, 299, 199), empty, empty, empty, empty},
       1,
       false,
       false},
      {"full_fill",
       DrawKind::kFill,
       {Box(0, 0, 799, 479), empty, empty, empty, empty},
       1,
       false,
       false},
      {"two_fill",
       DrawKind::kFill,
       {Box(10, 10, 19, 19), Box(780, 460, 789, 469), empty, empty, empty},
       2,
       false,
       false},
      {"four_fill",
       DrawKind::kFill,
       {Box(10, 10, 19, 19), Box(780, 10, 789, 19), Box(10, 460, 19, 469),
        Box(780, 460, 789, 469), empty},
       4,
       false,
       false},
      {"overlap_fill",
       DrawKind::kFill,
       {Box(180, 120, 279, 219), Box(230, 170, 329, 269), empty, empty, empty},
       2,
       false,
       false},
      {"crossing_fill",
       DrawKind::kFill,
       {Box(80, 225, 719, 254), Box(385, 40, 414, 439), Box(30, 30, 69, 69),
        Box(730, 410, 769, 449), empty},
       4,
       false,
       false},
      {"fragmented_fill",
       DrawKind::kFill,
       {Box(20, 20, 79, 79), Box(730, 20, 779, 89), Box(20, 390, 89, 459),
        Box(720, 390, 779, 459), empty},
       4,
       false,
       false},
      {"fragmented_sparse",
       DrawKind::kSparsePixels,
       {Box(20, 20, 79, 79), Box(730, 20, 779, 89), Box(20, 390, 89, 459),
        Box(720, 390, 779, 459), empty},
       4,
       true,
       false},
      {"fragmented_stream",
       DrawKind::kStreamedImage,
       {Box(20, 20, 79, 79), Box(730, 20, 779, 89), Box(20, 390, 89, 459),
        Box(720, 390, 779, 459), empty},
       4,
       true,
       false},
      {"fragmented_direct",
       DrawKind::kDirectImage,
       {Box(20, 20, 79, 79), Box(730, 20, 779, 89), Box(20, 390, 89, 459),
        Box(720, 390, 779, 459), empty},
       4,
       true,
       false},
      {"rounded_control",
       DrawKind::kFill,
       {Box(0, 0, 799, 479), empty, empty, empty, empty},
       1,
       false,
       true},
  }};
  return scenes;
}

/// Builds the bounded union used by either candidate for a scene.
inline internal::DamageRegion BuildNativeDamage(
    const NativeDamageScene& scene) {
  internal::DamageRegion damage;
  for (uint8_t i = 0; i < scene.damage_count; ++i) {
    damage.add(scene.damage[i]);
  }
  return damage;
}

/// Draws the same source operation through either output filter.
inline void DrawNativeDamageScene(const NativeDamageScene& scene,
                                  roo_display::DisplayOutput& output,
                                  const roo_display::Box& clip,
                                  const roo::byte* direct_pixels) {
  using roo_display::BlendingMode;
  using roo_display::Color;
  if (clip.empty()) return;
  if (scene.draw == DrawKind::kFill) {
    output.fillRect(BlendingMode::kSource, clip, Color(0xFF426281));
  } else if (scene.draw == DrawKind::kSparsePixels) {
    std::array<int16_t, 128> xs;
    std::array<int16_t, 128> ys;
    uint16_t count = 0;
    for (int y = clip.yMin(); y <= clip.yMax(); y += 11) {
      for (int x = clip.xMin(); x <= clip.xMax(); x += 13) {
        xs[count] = x;
        ys[count] = y;
        if (++count == xs.size()) {
          output.fillPixels(BlendingMode::kSource, Color(0xFF426281), xs.data(),
                            ys.data(), count);
          count = 0;
        }
      }
    }
    if (count != 0) {
      output.fillPixels(BlendingMode::kSource, Color(0xFF426281), xs.data(),
                        ys.data(), count);
    }
  } else if (scene.draw == DrawKind::kStreamedImage) {
    std::array<Color, 800> row;
    row.fill(Color(0xFF426281));
    output.setAddress(clip.xMin(), clip.yMin(), clip.xMax(), clip.yMax(),
                      BlendingMode::kSource);
    for (int y = clip.yMin(); y <= clip.yMax(); ++y) {
      output.write(row.data(), clip.width());
    }
  } else {
    const size_t pixel_bytes =
        output.getColorFormat().mode() ==
                roo_display::DisplayOutput::ColorFormat::kModeRgb565
            ? 2
            : 4;
    output.drawDirectRect(direct_pixels, clip.width() * pixel_bytes, 0, 0,
                          clip.width() - 1, clip.height() - 1, clip.xMin(),
                          clip.yMin());
  }
}

}  // namespace roo_windows::benchmarks
