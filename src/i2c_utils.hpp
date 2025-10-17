#ifndef HUB_I2C_UTILS_H
#define HUB_I2C_UTILS_H

#include <Wire.h>
#include <Arduino.h>
#include "definitions.h"

/**
 * @file i2c_utils.hpp
 * @brief Utility helpers for working with I2C devices.
 *
 * This header currently provides a flexible bus scanning helper. It is kept header-only and
 * marked inline for zero linkage overhead when used in multiple translation units.
 */

/**
 * @brief Scan an I2C bus for device addresses.
 *
 * This function probes each address in the specified range and reports devices that ACK.
 * It is designed for efficiency and flexibility on ESP32 / Arduino platforms, allowing
 * customization of address range, inter-probe delay, error reporting, and target wire bus.
 *
 * Typical valid 7-bit I2C address range is 0x08 - 0x77 (below 0x08 are mostly reserved).
 *
 * @param printer        Optional Print target for human-readable output (defaults to Serial if available).
 * @param wire           Reference to the TwoWire bus to scan (defaults to global Wire).
 * @param startAddress   First address to test (inclusive). Defaults to 0x08 to skip reserved addresses.
 * @param endAddress     Last address to test (inclusive). Defaults to 0x77 (maximum 7-bit address).
 * @param delayMicros    Microseconds delay between probes. Kept short (default 20µs) to minimize total scan time.
 * @param showErrors     If true, prints addresses that return error code 4 (other error codes are silently ignored).
 * @param foundCallback  Optional callback invoked for each found address (receives the 7-bit address).
 *
 * @return int           Number of devices that acknowledged in the range.
 *
 * @note You should call wire.begin() before invoking this scan.
 * @note A yield() is performed periodically to keep the watchdog serviced on ESP platforms.
 */
inline int scan_i2c(
    Print* printer = nullptr,
    TwoWire& wire = Wire,
    uint8_t startAddress = 0x08,
    uint8_t endAddress   = 0x77,
    uint16_t delayMicros = 20,
    bool showErrors = false,
    void (*foundCallback)(uint8_t address) = nullptr
) {
    if (startAddress > endAddress) {
        // Invalid range; nothing to do
        return 0;
    }

    if (printer == nullptr) {
        // Use Serial only if it has been begun; if not, skip printing.
        if (&Serial) {
            printer = &Serial;
        }
    }

    if (printer) {
        printer->print(F("Scanning I2C bus (0x"));
        if (startAddress < 16) printer->print('0');
        printer->print(startAddress, HEX);
        printer->print(F("-0x"));
        if (endAddress < 16) printer->print('0');
        printer->print(endAddress, HEX);
        printer->println(F(")..."));
    }

    int devicesFound = 0;
    uint32_t lastYield = millis();

    for (uint16_t address = startAddress; address <= endAddress; ++address) {
        // Minimal spacing between transactions; tuneable.
        if (delayMicros) {
            delayMicroseconds(delayMicros);
        }

        wire.beginTransmission(static_cast<uint8_t>(address));
        // Use a repeated start where possible to avoid releasing the bus unnecessarily.
        uint8_t error = wire.endTransmission(true);

        if (error == 0) {
            ++devicesFound;
            if (printer) {
                printer->print(F("- ADDR: 0x"));
                if (address < 16) printer->print('0');
                printer->print(address, HEX);
                printer->print(F(" ("));
                printer->print(address);
                printer->println(F(")"));
            }
            if (foundCallback) {
                foundCallback(static_cast<uint8_t>(address));
            }
        } else if (showErrors && error == 4) { // Other error codes typically mean NACK
            if (printer) {
                printer->print(F("  ! Error 4 at 0x"));
                if (address < 16) printer->print('0');
                printer->println(address, HEX);
            }
        }

        // Service watchdog / allow background tasks every ~64 addresses or 50ms.
        if (millis() - lastYield > 50 || (address & 0x3F) == 0) {
            lastYield = millis();
            yield();
        }
    }

    if (printer) {
        if (devicesFound == 0) {
            printer->println(F("No I2C devices found."));
        } else {
            printer->print(F("Scan complete. Devices found: "));
            printer->println(devicesFound);
        }
    }

    return devicesFound;
}

#endif // HUB_I2C_UTILS_H