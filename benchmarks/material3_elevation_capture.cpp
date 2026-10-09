// Exports the production decoration rasterizer's alpha for all shadow settings.
// Usage: bazel run //:material3_elevation_capture -- /absolute/output/directory
// Build again with --copt=-DROO_WINDOWS_ZOOM=200 for the second zoom sample.
#include <cstdio>
#include <filesystem>
#include <string>
#include <system_error>
#include <vector>

#include "roo_windows/config.h"
#include "roo_windows/decoration/decoration.h"

int main(int argc, char** argv) {
  if (argc != 2) return 1;
  std::error_code error;
  std::filesystem::create_directories(argv[1], error);
  if (error) return 2;
  constexpr int kMarginDp = 64;
  const int zoom = ROO_WINDOWS_ZOOM;
  for (int size : {56, 96}) {
    const int margin = roo_windows::Scaled(kMarginDp);
    const int side = roo_windows::Scaled(size);
    const int radius = roo_windows::Scaled(size == 56 ? 16 : 28);
    const int dim = side + 2 * margin;
    std::vector<int16_t> x(dim), y(dim);
    std::vector<roo_display::Color> colors(dim);
    std::vector<unsigned char> alpha(dim);
    for (int col = 0; col < dim; ++col) x[col] = col;
    for (int elevation = 0; elevation < 32; ++elevation) {
      const roo_windows::Decoration decoration(
          roo_display::Box(margin, margin, margin + side - 1,
                           margin + side - 1),
          elevation, roo_windows::OverlaySpec(), nullptr,
          roo_display::Color(0xFFFFFFFF),
          roo_windows::BorderStyle(radius, 0).corner_radii(),
          roo_windows::SmallNumber(0), roo_display::Color(0x00000000));
      const std::string filename =
          std::string(argv[1]) + "/roo_z" + std::to_string(zoom) + "_s" +
          std::to_string(size) + "_e" + std::to_string(elevation) + ".pgm";
      FILE* out = std::fopen(filename.c_str(), "wb");
      if (out == nullptr) return 2;
      std::fprintf(out, "P5\n%d %d\n255\n", dim, dim);
      for (int row = 0; row < dim; ++row) {
        for (int col = 0; col < dim; ++col) y[col] = row;
        decoration.readColors(x.data(), y.data(), dim, colors.data());
        for (int col = 0; col < dim; ++col) alpha[col] = colors[col].a();
        if (std::fwrite(alpha.data(), 1, dim, out) !=
            static_cast<size_t>(dim)) {
          std::fclose(out);
          return 3;
        }
      }
      std::fclose(out);
    }
  }
  std::printf("Exported 64 production-rasterizer samples at zoom %d\n", zoom);
}
