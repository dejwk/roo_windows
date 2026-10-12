#include "roo_windows/core/damage_region.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <vector>

#include "gtest/gtest.h"

namespace {
bool track_allocations = false;
size_t allocation_count = 0;

void* Allocate(size_t size) {
  if (track_allocations) ++allocation_count;
  return std::malloc(size == 0 ? 1 : size);
}

[[gnu::noinline]] void FreeAllocation(void* ptr) noexcept { std::free(ptr); }
}  // namespace

void* operator new(size_t size) {
  void* ptr = Allocate(size);
  if (ptr == nullptr) std::abort();
  return ptr;
}

void* operator new[](size_t size) { return ::operator new(size); }

void* operator new(size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}

void* operator new[](size_t size, const std::nothrow_t&) noexcept {
  return Allocate(size);
}

void operator delete(void* ptr) noexcept { FreeAllocation(ptr); }

void operator delete[](void* ptr) noexcept { FreeAllocation(ptr); }

void operator delete(void* ptr, size_t) noexcept { FreeAllocation(ptr); }

void operator delete[](void* ptr, size_t) noexcept { FreeAllocation(ptr); }

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
  FreeAllocation(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
  FreeAllocation(ptr);
}

namespace roo_windows::internal {
namespace {

using roo_display::Box;

uint32_t Next(uint32_t& state) {
  state = state * 1664525u + 1013904223u;
  return state;
}

Box RandomBox(uint32_t& state) {
  const int16_t x0 = static_cast<int16_t>(Next(state) % 12) - 3;
  const int16_t y0 = static_cast<int16_t>(Next(state) % 12) - 3;
  const int16_t x1 = static_cast<int16_t>(Next(state) % 12) - 3;
  const int16_t y1 = static_cast<int16_t>(Next(state) % 12) - 3;
  return Box(std::min(x0, x1), std::min(y0, y1), std::max(x0, x1),
             std::max(y0, y1));
}

// Verifies bounded accumulation conservatively retains every submitted pixel,
// while direct bounds and complement match the retained union exactly.
TEST(DamageRegionTest, RandomizedSmallGridOracle) {
  constexpr Box kClip(-3, -3, 8, 8);
  uint32_t state = 0xC0FFEEu;
  for (int trial = 0; trial < 300; ++trial) {
    DamageRegion region;
    std::array<bool, 12 * 12> submitted{};
    for (int addition = 0; addition < 9; ++addition) {
      const Box box = RandomBox(state);
      region.add(box);
      EXPECT_LE(region.size(), DamageRegion::kCapacity);
      for (int y = -3; y <= 8; ++y) {
        for (int x = -3; x <= 8; ++x) {
          if (box.contains(x, y)) submitted[(y + 3) * 12 + x + 3] = true;
        }
      }

      std::vector<Box> gaps;
      region.appendComplement(kClip, gaps);
      EXPECT_LE(gaps.size(), 45u);
      Box expected_bounds(0, 0, -1, -1);
      for (int y = -3; y <= 8; ++y) {
        for (int x = -3; x <= 8; ++x) {
          bool in_region = false;
          for (uint8_t i = 0; i < region.size(); ++i) {
            in_region |= region.at(i).contains(x, y);
          }
          const Box pixel(x, y, x, y);
          EXPECT_EQ(region.covers(pixel), in_region);
          EXPECT_FALSE(submitted[(y + 3) * 12 + x + 3] && !in_region);
          if (in_region) {
            expected_bounds = expected_bounds.empty()
                                  ? pixel
                                  : Box::Extent(expected_bounds, pixel);
          }
          int gap_count = 0;
          for (const Box& gap : gaps) {
            if (gap.contains(x, y)) ++gap_count;
          }
          EXPECT_EQ(gap_count, in_region ? 0 : 1);
        }
      }
      EXPECT_EQ(region.intersectionBounds(kClip), expected_bounds);
      EXPECT_EQ(region.bounds(), region.intersectionBounds(kClip));
      EXPECT_EQ(region.covers(kClip), gaps.empty());
    }
  }
}

// Verifies a clip in the damage envelope's hole is rejected without allocation.
TEST(DamageRegionTest, DirectIntersectionBoundsAndEmptyCases) {
  DamageRegion region;
  EXPECT_EQ(sizeof(DamageRegion), 34u);
  const Box empty(0, 0, -1, -1);
  EXPECT_TRUE(region.covers(empty));
  EXPECT_FALSE(region.covers(Box(0, 0, 0, 0)));
  region.add(Box(10, 10, 19, 19));
  region.add(Box(300, 220, 309, 229));
  EXPECT_EQ(region.intersectionBounds(Box(100, 100, 199, 139)), empty);
  EXPECT_EQ(region.intersectionBounds(Box(0, 0, 39, 39)), Box(10, 10, 19, 19));
  EXPECT_EQ(region.intersectionBounds(Box(0, 0, 319, 239)),
            Box(10, 10, 309, 229));
  EXPECT_EQ(region.intersectionBounds(empty), empty);

  allocation_count = 0;
  track_allocations = true;
  const Box result = region.intersectionBounds(Box(0, 0, 319, 239));
  track_allocations = false;
  EXPECT_EQ(result, Box(10, 10, 309, 229));
  EXPECT_EQ(allocation_count, 0u);
}

// Verifies zero-area merges and deterministic overflow ties preserve all
// pixels.
TEST(DamageRegionTest, MergesAdjacentAndChoosesFirstEqualCostPair) {
  DamageRegion region;
  region.add(Box(0, 0, 0, 0));
  region.add(Box(1, 0, 1, 0));
  EXPECT_EQ(region.size(), 1);
  EXPECT_EQ(region.at(0), Box(0, 0, 1, 0));

  region.clear();
  for (int x = 0; x <= 8; x += 2) {
    region.add(Box(x, 0, x, 0));
  }
  ASSERT_EQ(region.size(), 4);
  EXPECT_EQ(region.at(0), Box(4, 0, 4, 0));
  EXPECT_EQ(region.at(1), Box(6, 0, 6, 0));
  EXPECT_EQ(region.at(2), Box(8, 0, 8, 0));
  EXPECT_EQ(region.at(3), Box(0, 0, 2, 0));

  region.clear();
  region.add(Box(0, 2, 4, 2));
  region.add(Box(2, 0, 2, 4));
  EXPECT_EQ(region.size(), 2);
  region.add(Box(1, 1, 3, 3));
  EXPECT_EQ(region.size(), 3);
  region.add(Box(0, 0, 4, 4));
  EXPECT_EQ(region.size(), 1);
}

// Verifies inclusive maxima and the full int16 range use widened arithmetic.
TEST(DamageRegionTest, FullCoordinateRangeAndFragmentBound) {
  DamageRegion region;
  const Box full(-32768, -32768, 32767, 32767);
  region.add(Box(-32768, -32768, -32768, -32768));
  region.add(Box(32767, 32767, 32767, 32767));
  EXPECT_EQ(region.intersectionBounds(full), full);
  EXPECT_FALSE(region.covers(full));
  std::vector<Box> gaps;
  region.appendComplement(full, gaps);
  EXPECT_LE(gaps.size(), 45u);
  EXPECT_FALSE(gaps.empty());
  region.add(full);
  EXPECT_TRUE(region.covers(full));
  gaps.clear();
  region.appendComplement(full, gaps);
  EXPECT_TRUE(gaps.empty());
}

}  // namespace
}  // namespace roo_windows::internal
