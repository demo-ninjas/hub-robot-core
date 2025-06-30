#ifndef HUB_I2C_UTILS_H
#define HUB_I2C_UTILS_H

#include <Wire.h>
#include <Arduino.h>
#include "definitions.h"

/**
 * @brief Scans the I2C bus for devices, printing the address of each found device to the provided printer device (or the default Serial if none is provided).
 * @returns The number of devices found on the I2C bus.
 */
int scan_i2c(Print* printer = nullptr) {
    if (printer == nullptr) {
        printer = &Serial;
    }

    printer->println("Scanning I2C Bus for Devices...");
    int nDevices = 0;
    for(int address = 1; address < 127; address++ ) {
        delayMicroseconds(50);
        Wire.beginTransmission(address);
        int error = Wire.endTransmission();

        if (error == 0) {
            printer->print("- ADDR: 0x");
            if (address < 16)
                printer->print(F("0"));
            printer->print(address, HEX);
            printer->print(" (");
            printer->print(address);
            printer->println(")");
            nDevices++;
        } else if (error==4) {
            printer->print("Unknown error at address 0x");
            if (address<16)
                printer->print(F("0"));
            printer->println(address,HEX);
        }
    }

    if (nDevices == 0)
        printer->println("No I2C devices found");

    return nDevices;
}
  
#endif // HUB_I2C_UTILS_H