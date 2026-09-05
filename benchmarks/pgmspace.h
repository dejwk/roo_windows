#pragma once

// Parsing-only program-memory compatibility for standalone target-ABI probes.
#define PROGMEM
#ifndef pgm_read_byte
#define pgm_read_byte(address) \
  (*reinterpret_cast<const unsigned char*>(address))
#endif
