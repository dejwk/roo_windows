#pragma once

// See
// https://stackoverflow.com/questions/31117497/fastest-integer-square-root-in-the-least-amount-of-instructions

namespace roo_windows {

// static uint8_t sqrt_arr[] = {
//     0,  1,  1,  1,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4, 4,
//     4,  4,  4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  6, 6,
//     6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  7,  7,  7,  7,  7,  7,  7, 7,
//     7,  7,  7,  7,  7,  7,  7,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8,  8, 8,
//     8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9,  9, 9,
//     9,  9,  9,  9,  9,  10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
//     10, 10, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
//     11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12,
//     12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
//     12, 12, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
//     13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14,
//     14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
//     14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
//     15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

// static uint16_t pow_arr[] = {
//     0,   1,   4,   9,   16,  25,  36,  49,  64,  81,  100,
//     121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441,
//     484, 529, 576, 625, 676, 729, 784, 841, 900, 961,
// };

// inline uint8_t isqrt(uint16_t n) {
//   if (n <= 255) {
//     return sqrt_arr[n];
//   }
//   int r = 0x80;
//   int c = 0x80;
//   for (;;) {
//     if (r * r > n) {
//       r ^= c;
//     }
//     c >>= 1;
//     if (c == 0) {
//       return r;
//     }
//     r |= c;
//   }
// }

// inline uint16_t isqrt(uint32_t n) {
//   // if (n <= 255) {
//   //   return sqrt_arr[n];
//   // }
//   uint32_t r = 0x8000;
//   uint32_t c = 0x8000;
//   for (;;) {
//     if (r * r > n) {
//       r ^= c;
//     }
//     c >>= 1;
//     if (c == 0) {
//       return r;
//     }
//     r |= c;
//   }
// }

inline uint16_t isqrt32(uint32_t x) {
  uint32_t r = 0, r2 = 0;
  for (int p = 15; p >= 0; --p) {
    uint32_t tr2 = r2 + (r << (p + 1)) + ((uint32_t)1u << (p + p));
    if (tr2 <= x) {
      r2 = tr2;
      r |= ((uint32_t)1u << p);
    }
  }
  return r;
}

inline uint32_t isqrt64(uint64_t x) {
  uint64_t r = 0, r2 = 0;
  for (int p = 31; p >= 0; --p) {
    uint64_t tr2 = r2 + (r << (p + 1)) + ((uint64_t)1u << (p + p));
    if (tr2 <= x) {
      r2 = tr2;
      r |= ((uint64_t)1u << p);
    }
  }
  return r;
}

// Works for x up to (2^12)^2-1 = 4096^2-1.
inline uint16_t isqrt24(uint32_t x) {
  uint32_t r = 0, r2 = 0;
  uint32_t tr2;
  tr2 = r2 + (r << 12) + ((uint32_t)1u << 22);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 11);
  }
  tr2 = r2 + (r << 11) + ((uint32_t)1u << 20);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 10);
  }
  tr2 = r2 + (r << 10) + ((uint32_t)1u << 18);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 9);
  }
  tr2 = r2 + (r << 9) + ((uint32_t)1u << 16);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 8);
  }
  tr2 = r2 + (r << 8) + ((uint32_t)1u << 14);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 7);
  }
  tr2 = r2 + (r << 7) + ((uint32_t)1u << 12);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 6);
  }
  tr2 = r2 + (r << 6) + ((uint32_t)1u << 10);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 5);
  }
  tr2 = r2 + (r << 5) + ((uint32_t)1u << 8);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 4);
  }
  tr2 = r2 + (r << 4) + ((uint32_t)1u << 6);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 3);
  }
  tr2 = r2 + (r << 3) + ((uint32_t)1u << 4);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 2);
  }
  tr2 = r2 + (r << 2) + ((uint32_t)1u << 2);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 1);
  }
  tr2 = r2 + (r << 1) + ((uint32_t)1u << 0);
  if (tr2 <= x) {
    r2 = tr2;
    r |= ((uint32_t)1u << 0);
  }
  return r;
}

}  // namespace roo_windows
