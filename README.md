# Hub Robot Core

This is the base library for the Hub's low-powered Robot projects.

Primarily designed for supporting Arduino or ESP based robotic projects.

See also: 

* [Hub Robot HTTP](https://github.com/demo-ninjas/hub-robot-http-utils) - Very basic HTTP Server for low powered robots
* [Hub Robot OLED](https://github.com/demo-ninjas/hub-robot-oled) - Simple OLED wrapper library to support some of the small OLED screens used in the HUB

## Dev Machine Setup

Make sure you have the following installed on your machine: 

* A C/C++ compiler + standard dev tools installed (gcc, git, cmake etc...)
* PlatformIO Tools (https://platformio.org/)
* VSCode (https://code.visualstudio.com/)

Install the following VSCode Extensions: 

* C/C++ Extension pack (https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) [This includes the C/C++ Extension]
* CMake Tools (https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* PlatformIO IDE (https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)

Install the `Arduino Espressif32` Framework in PlatformIO and make sure an ESP32 toolchain is installed (We typically use `ESP32S3`, eg. `toolchain-xtensa-esp32s3`).

## Publish

This library is intended to be used as a dependency for a Platform.io app.

The `library.json` describes the library and dependencies to Platform.io.

increment the version number when making changes you want to be used by dependent libraries.

Currently, not published to PlatformIO, load the library dependency directly via the GitHub URL.

