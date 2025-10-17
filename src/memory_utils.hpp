#ifndef HUB_MEMORY_UTILS_H
#define HUB_MEMORY_UTILS_H

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
  #include <esp_heap_caps.h>
#endif

/**
 * @namespace HubMemoryUtils
 * @brief Utilities for monitoring and managing memory on embedded systems.
 */
namespace HubMemoryUtils {

  /**
   * @brief Returns the amount of free heap memory available on the device.
   * 
   * Platform-specific implementation:
   * - ESP32: Uses esp_get_free_heap_size() for accurate heap measurement
   * - Other platforms: Estimates free RAM using stack pointer technique
   * 
   * @return The amount of free heap memory in bytes, or 0 if unable to determine
   * 
   * @note On ESP32, this returns the total free heap in all regions.
   *       Use esp_get_free_internal_heap_size() if you need internal-only memory.
   * @note On non-ESP32 platforms, the value is an approximation based on stack position.
   * @note This function is safe to call frequently but has a small overhead.
   * 
   * Example:
   * @code
   * size_t free = HubMemoryUtils::freeRam();
   * Serial.printf("Free memory: %u bytes\n", free);
   * @endcode
   */
  inline size_t freeRam() noexcept {
    #if defined(ARDUINO_ARCH_ESP32)
      return esp_get_free_heap_size();
    #else
      // For non-ESP32 platforms, use stack-based estimation
      // This is an approximation and may not be 100% accurate
      extern "C" char* sbrk(int incr);
      char top;
      char* heap_end = sbrk(0);
      
      // Safety check: ensure valid heap pointer
      if (heap_end == reinterpret_cast<char*>(-1)) {
        return 0; // sbrk failed
      }
      
      return &top - heap_end;
    #endif
  }

  /**
   * @brief Returns the minimum free heap size since boot (ESP32 only).
   * 
   * This is useful for detecting memory usage peaks and ensuring adequate headroom.
   * 
   * @return The minimum free heap size in bytes, or current free RAM on non-ESP32 platforms
   * 
   * @note This function is only meaningful on ESP32. On other platforms, it returns
   *       the same value as freeRam().
   * 
   * Example:
   * @code
   * size_t min_free = HubMemoryUtils::minFreeRam();
   * Serial.printf("Memory low-water mark: %u bytes\n", min_free);
   * @endcode
   */
  inline size_t minFreeRam() noexcept {
    #if defined(ARDUINO_ARCH_ESP32)
      return esp_get_minimum_free_heap_size();
    #else
      // Not available on non-ESP32 platforms, return current free RAM
      return freeRam();
    #endif
  }

  /**
   * @brief Returns the size of the largest free block in the heap (ESP32 only).
   * 
   * This is useful for determining if you can allocate large contiguous buffers.
   * Even with sufficient total free memory, fragmentation may prevent large allocations.
   * 
   * @return The size of the largest free heap block in bytes, or total free RAM on non-ESP32
   * 
   * @note On ESP32, heap fragmentation can cause allocation failures even when
   *       freeRam() shows sufficient memory. Check this value before large allocations.
   * @note On non-ESP32 platforms, this returns the same value as freeRam().
   * 
   * Example:
   * @code
   * size_t needed = 8192;
   * if (HubMemoryUtils::largestFreeBlock() >= needed) {
   *     uint8_t* buffer = new uint8_t[needed];
   *     // Use buffer...
   *     delete[] buffer;
   * }
   * @endcode
   */
  inline size_t largestFreeBlock() noexcept {
    #if defined(ARDUINO_ARCH_ESP32)
      return heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT);
    #else
      // Not available on non-ESP32 platforms, return current free RAM
      return freeRam();
    #endif
  }

  /**
   * @brief Returns the total heap size (ESP32 only).
   * 
   * @return The total heap size in bytes, or 0 on non-ESP32 platforms
   * 
   * @note This value is fixed at boot and represents the total heap available.
   * 
   * Example:
   * @code
   * size_t total = HubMemoryUtils::totalHeap();
   * size_t free = HubMemoryUtils::freeRam();
   * float usage = 100.0 * (1.0 - (float)free / (float)total);
   * Serial.printf("Heap usage: %.1f%%\n", usage);
   * @endcode
   */
  inline size_t totalHeap() noexcept {
    #if defined(ARDUINO_ARCH_ESP32)
      return heap_caps_get_total_size(MALLOC_CAP_DEFAULT);
    #else
      // Not available on non-ESP32 platforms
      return 0;
    #endif
  }

  /**
   * @brief Calculates the current heap usage percentage (ESP32 only).
   * 
   * @return Heap usage as a percentage (0.0 to 100.0), or 0.0 on non-ESP32 platforms
   * 
   * Example:
   * @code
   * float usage = HubMemoryUtils::heapUsagePercent();
   * if (usage > 90.0) {
   *     Serial.println("WARNING: Low memory!");
   * }
   * @endcode
   */
  inline float heapUsagePercent() noexcept {
    #if defined(ARDUINO_ARCH_ESP32)
      size_t total = totalHeap();
      if (total == 0) return 0.0f;
      size_t free = freeRam();
      return 100.0f * (1.0f - static_cast<float>(free) / static_cast<float>(total));
    #else
      return 0.0f;
    #endif
  }

} // namespace HubMemoryUtils

// Convenience using declarations for backward compatibility and ease of use
using HubMemoryUtils::freeRam;
using HubMemoryUtils::minFreeRam;
using HubMemoryUtils::largestFreeBlock;
using HubMemoryUtils::totalHeap;
using HubMemoryUtils::heapUsagePercent;

#endif // HUB_MEMORY_UTILS_H
