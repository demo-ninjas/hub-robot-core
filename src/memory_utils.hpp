#ifndef HUB_MEMORY_UTILS_H
#define HUB_MEMORY_UTILS_H

#include <Arduino.h>

/**
 * @brief Returns the amount of free heap memory available on the device.
 * @returns The amount of free heap memory in bytes.
 */
#if defined(ARDUINO_ARCH_ESP32)
  int freeRam() {
    return esp_get_free_heap_size();
  }
#else
extern "C" char* sbrk(int incr);
int freeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
#endif

#endif // HUB_MEMORY_UTILS_H
