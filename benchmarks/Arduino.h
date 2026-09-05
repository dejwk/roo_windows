#pragma once

// Parsing-only Arduino clock surface for standalone target-ABI probes. This
// directory is added ahead of platform headers only by the documented probe
// command; production builds never select this file.
#include <stdint.h>

using boolean = bool;

uint32_t micros();
uint32_t millis();
void delay(uint32_t milliseconds);
