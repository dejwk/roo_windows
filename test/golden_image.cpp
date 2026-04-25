#include "test/golden_image.h"

#include <array>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <string>
#include <vector>

namespace roo_windows {
namespace test {
namespace {

bool IsTruthyEnv(const char* value) {
  if (value == nullptr) return false;
  std::string v(value);
  for (char& c : v) c = static_cast<char>(::tolower(c));
  return !(v.empty() || v == "0" || v == "false" || v == "no");
}

std::string SanitizeStem(const std::string& stem) {
  std::string out;
  out.reserve(stem.size());
  for (char c : stem) {
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') || c == '-' || c == '_') {
      out.push_back(c);
    } else {
      out.push_back('_');
    }
  }
  return out;
}

void EnsureParentDir(const std::string& path) {
  std::filesystem::path p(path);
  std::filesystem::create_directories(p.parent_path());
}

bool ReadToken(std::istream& in, std::string& out) {
  out.clear();
  while (true) {
    int c = in.peek();
    if (c == EOF) return false;
    if (std::isspace(static_cast<unsigned char>(c))) {
      in.get();
      continue;
    }
    if (c == '#') {
      std::string ignored;
      std::getline(in, ignored);
      continue;
    }
    break;
  }

  while (true) {
    int c = in.peek();
    if (c == EOF || std::isspace(static_cast<unsigned char>(c)) || c == '#') {
      break;
    }
    out.push_back(static_cast<char>(in.get()));
  }
  return !out.empty();
}

bool ReadPpm(const std::string& path,
             std::unique_ptr<roo_display::Offscreen<roo_display::Rgb888>>& out,
             std::string& error) {
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open()) {
    error = "cannot open file";
    return false;
  }

  std::string token;
  if (!ReadToken(in, token) || token != "P6") {
    error = "invalid PPM magic (expected P6)";
    return false;
  }
  if (!ReadToken(in, token)) {
    error = "missing width";
    return false;
  }
  int width = std::stoi(token);
  if (!ReadToken(in, token)) {
    error = "missing height";
    return false;
  }
  int height = std::stoi(token);
  if (!ReadToken(in, token)) {
    error = "missing max value";
    return false;
  }
  int max_value = std::stoi(token);
  if (max_value != 255) {
    error = "unsupported PPM max value";
    return false;
  }

  if (width < 0 || height < 0) {
    error = "invalid negative image dimensions";
    return false;
  }
  if (width > std::numeric_limits<int16_t>::max() ||
      height > std::numeric_limits<int16_t>::max()) {
    error = "image dimensions exceed supported range";
    return false;
  }

  in.get();  // Single whitespace separator after header.

  auto image = std::make_unique<roo_display::Offscreen<roo_display::Rgb888>>(
      static_cast<int16_t>(width), static_cast<int16_t>(height),
      roo_display::Rgb888());

  if (width > 0 && height > 0) {
    std::vector<int16_t> dst_x(width);
    std::vector<int16_t> dst_y(width);
    std::vector<roo_display::Color> colors(width);
    std::vector<uint8_t> row_rgb(static_cast<size_t>(width) * 3);

    for (int i = 0; i < width; ++i) {
      dst_x[i] = i;
    }

    for (int row = 0; row < height; ++row) {
      in.read(reinterpret_cast<char*>(row_rgb.data()), row_rgb.size());
      if (in.gcount() != static_cast<std::streamsize>(row_rgb.size())) {
        error = "unexpected EOF in pixel data";
        return false;
      }

      for (int col = 0; col < width; ++col) {
        size_t idx = static_cast<size_t>(col) * 3;
        colors[col] = roo_display::Color(row_rgb[idx + 0], row_rgb[idx + 1],
                                         row_rgb[idx + 2]);
      }

      std::fill(dst_y.begin(), dst_y.end(), row);
      image->output().writePixels(roo_display::BlendingMode::kSource,
                                  colors.data(), dst_x.data(), dst_y.data(),
                                  width);
    }
  }

  out = std::move(image);
  return true;
}

bool WritePpm(const std::string& path, const roo_display::Rasterizable& image,
              std::string& error) {
  EnsureParentDir(path);
  std::ofstream out(path, std::ios::binary);
  if (!out.is_open()) {
    error = "cannot open file for write";
    return false;
  }

  roo_display::Box extents = image.extents();
  int16_t width = extents.width();
  int16_t height = extents.height();
  out << "P6\n" << width << " " << height << "\n255\n";

  if (width > 0 && height > 0) {
    std::vector<int16_t> src_x(width);
    std::vector<int16_t> src_y(width);
    std::vector<roo_display::Color> colors(width);
    std::vector<uint8_t> row_rgb(static_cast<size_t>(width) * 3);

    for (int i = 0; i < width; ++i) {
      src_x[i] = extents.xMin() + i;
    }

    for (int row = 0; row < height; ++row) {
      std::fill(src_y.begin(), src_y.end(), extents.yMin() + row);
      image.readColors(src_x.data(), src_y.data(), width, colors.data());

      for (int col = 0; col < width; ++col) {
        size_t idx = static_cast<size_t>(col) * 3;
        row_rgb[idx + 0] = colors[col].r();
        row_rgb[idx + 1] = colors[col].g();
        row_rgb[idx + 2] = colors[col].b();
      }

      out.write(reinterpret_cast<const char*>(row_rgb.data()), row_rgb.size());
      if (!out.good()) {
        error = "failed while writing file";
        return false;
      }
    }
  }

  if (!out.good()) {
    error = "failed while writing file";
    return false;
  }
  return true;
}

std::string ResolveGoldenReadPath(const std::string& rel) {
  std::vector<std::string> candidates;
  candidates.push_back(rel);

  const char* test_srcdir = std::getenv("TEST_SRCDIR");
  const char* test_workspace = std::getenv("TEST_WORKSPACE");
  if (test_srcdir != nullptr && test_workspace != nullptr) {
    candidates.push_back(std::string(test_srcdir) + "/" + test_workspace + "/" +
                         rel);
  }
  if (test_srcdir != nullptr) {
    candidates.push_back(std::string(test_srcdir) + "/_main/" + rel);
  }

  for (const auto& c : candidates) {
    if (std::filesystem::exists(c)) return c;
  }

  return rel;
}

std::string ResolveGoldenUpdatePath(const std::string& rel) {
  const char* ws_dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
  if (ws_dir != nullptr && ws_dir[0] != '\0') {
    return std::string(ws_dir) + "/" + rel;
  }

  const char* root = std::getenv("ROO_GOLDEN_UPDATE_ROOT");
  if (root != nullptr && root[0] != '\0') {
    return std::string(root) + "/" + rel;
  }

  return std::string();
}

std::string ArtifactDir() {
  const char* out = std::getenv("TEST_UNDECLARED_OUTPUTS_DIR");
  if (out != nullptr && out[0] != '\0') return std::string(out);
  return "/tmp";
}

void ReadColorRectExpanded(const roo_display::Rasterizable& image,
                           int16_t x_min, int16_t y_min, int16_t width,
                           int16_t height, roo_display::Color* out) {
  if (width <= 0 || height <= 0) return;
  bool uniform = image.readColorRect(x_min, y_min, x_min + width - 1,
                                     y_min + height - 1, out);
  if (!uniform) return;

  roo_display::Color fill = out[0];
  std::fill(out, out + static_cast<size_t>(width) * height, fill);
}

roo_display::Offscreen<roo_display::Rgb888> DiffImage(
    const roo_display::Rasterizable& actual,
    const roo_display::Rasterizable& expected, uint64_t& diff_pixel_count,
    int& first_diff_x, int& first_diff_y) {
  roo_display::Box actual_extents = actual.extents();
  roo_display::Box expected_extents = expected.extents();

  int16_t actual_w = actual_extents.width();
  int16_t actual_h = actual_extents.height();
  int16_t expected_w = expected_extents.width();
  int16_t expected_h = expected_extents.height();

  int16_t out_w = std::max(actual_w, expected_w);
  int16_t out_h = std::max(actual_h, expected_h);

  roo_display::Offscreen<roo_display::Rgb888> diff(out_w, out_h,
                                                   roo_display::Rgb888());

  diff_pixel_count = 0;
  first_diff_x = -1;
  first_diff_y = -1;

  if (out_w == 0 || out_h == 0) {
    return diff;
  }

  static constexpr int16_t kBlockSize = 8;
  std::array<roo_display::Color, kBlockSize * kBlockSize> actual_block;
  std::array<roo_display::Color, kBlockSize * kBlockSize> expected_block;
  roo_display::BufferedPixelWriter writer(diff.output(),
                                          roo_display::BlendingMode::kSource);

  for (int16_t y = 0; y < out_h; y += kBlockSize) {
    int16_t block_h = std::min<int16_t>(kBlockSize, out_h - y);
    for (int16_t x = 0; x < out_w; x += kBlockSize) {
      int16_t block_w = std::min<int16_t>(kBlockSize, out_w - x);

      int16_t actual_block_w = 0;
      int16_t actual_block_h = 0;
      int16_t expected_block_w = 0;
      int16_t expected_block_h = 0;

      std::fill(actual_block.begin(), actual_block.end(),
                roo_display::color::Transparent);
      std::fill(expected_block.begin(), expected_block.end(),
                roo_display::color::Transparent);

      if (x < actual_w && y < actual_h) {
        actual_block_w = std::min<int16_t>(block_w, actual_w - x);
        actual_block_h = std::min<int16_t>(block_h, actual_h - y);
        ReadColorRectExpanded(actual, actual_extents.xMin() + x,
                              actual_extents.yMin() + y, actual_block_w,
                              actual_block_h, actual_block.data());
      }

      if (x < expected_w && y < expected_h) {
        expected_block_w = std::min<int16_t>(block_w, expected_w - x);
        expected_block_h = std::min<int16_t>(block_h, expected_h - y);
        ReadColorRectExpanded(expected, expected_extents.xMin() + x,
                              expected_extents.yMin() + y, expected_block_w,
                              expected_block_h, expected_block.data());
      }

      for (int16_t dy = 0; dy < block_h; ++dy) {
        for (int16_t dx = 0; dx < block_w; ++dx) {
          bool in_actual = dx < actual_block_w && dy < actual_block_h;
          bool in_expected = dx < expected_block_w && dy < expected_block_h;

          roo_display::Color actual_color = roo_display::color::Transparent;
          roo_display::Color expected_color = roo_display::color::Transparent;
          if (in_actual) {
            actual_color =
                actual_block[static_cast<size_t>(dy) * actual_block_w + dx];
          }
          if (in_expected) {
            expected_color =
                expected_block[static_cast<size_t>(dy) * expected_block_w + dx];
          }

          uint8_t xor_red =
              static_cast<uint8_t>((actual_color.r() ^ expected_color.r()) |
                                   (actual_color.g() ^ expected_color.g()) |
                                   (actual_color.b() ^ expected_color.b()));
          writer.writePixel(x + dx, y + dy, roo_display::Color(xor_red, 0, 0));

          bool same =
              (!in_actual && !in_expected) ||
              (in_actual && in_expected && actual_color == expected_color);
          if (!same) {
            ++diff_pixel_count;
            if (first_diff_x < 0) {
              first_diff_x = x + dx;
              first_diff_y = y + dy;
            }
          }
        }
      }
    }
  }

  writer.flush();

  return diff;
}

}  // namespace

::testing::AssertionResult CompareOrUpdateGolden(
    const roo_display::Rasterizable& actual,
    const std::string& golden_relative_path, const std::string& artifact_stem) {
  const bool update = IsTruthyEnv(std::getenv("ROO_UPDATE_GOLDENS"));

  if (update) {
    std::string update_path = ResolveGoldenUpdatePath(golden_relative_path);
    if (update_path.empty()) {
      return ::testing::AssertionFailure()
             << "ROO_UPDATE_GOLDENS is set, but BUILD_WORKSPACE_DIRECTORY and "
                "ROO_GOLDEN_UPDATE_ROOT are unset";
    }

    std::string error;
    if (!WritePpm(update_path, actual, error)) {
      return ::testing::AssertionFailure()
             << "Failed to update golden '" << update_path << "': " << error;
    }

    return ::testing::AssertionSuccess() << "Updated golden: " << update_path;
  }

  std::string expected_path = ResolveGoldenReadPath(golden_relative_path);
  std::unique_ptr<roo_display::Offscreen<roo_display::Rgb888>> expected;
  std::string read_error;
  if (!ReadPpm(expected_path, expected, read_error)) {
    std::string artifact_base =
        ArtifactDir() + "/" + SanitizeStem(artifact_stem);
    std::string actual_path = artifact_base + "_actual.ppm";
    std::string write_error;
    WritePpm(actual_path, actual, write_error);

    return ::testing::AssertionFailure()
           << "Missing/unreadable golden: " << expected_path << " ("
           << read_error << ")\n"
           << "Wrote actual image to: " << actual_path << "\n"
           << "To regenerate goldens: ROO_UPDATE_GOLDENS=1 bazel test ...";
  }

  uint64_t diff_pixel_count = 0;
  int diff_x = -1;
  int diff_y = -1;
  roo_display::Offscreen<roo_display::Rgb888> diff =
      DiffImage(actual, *expected, diff_pixel_count, diff_x, diff_y);

  if (diff_pixel_count == 0) {
    return ::testing::AssertionSuccess();
  }

  std::string artifact_base = ArtifactDir() + "/" + SanitizeStem(artifact_stem);
  std::string xor_path = artifact_base + "_xor.ppm";

  std::string write_error;
  WritePpm(xor_path, diff, write_error);

  return ::testing::AssertionFailure()
         << "Golden mismatch for " << golden_relative_path << "\n"
         << "First differing pixel: (" << diff_x << ", " << diff_y << ")\n"
         << "Differing pixel count: " << diff_pixel_count << "\n"
         << "XOR diff: " << xor_path;
}

}  // namespace test
}  // namespace roo_windows
