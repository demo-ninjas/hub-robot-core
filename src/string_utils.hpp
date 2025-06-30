#ifndef HUB_STRING_UTILS_H
#define HUB_STRING_UTILS_H

#include <Arduino.h>
#include <string>
#include <vector>

int utf8ByteLength(const String& str) {
  int utf_8_byte_length = 0;
  for (char c : str) {
    if ((c & 0x80) == 0)
      utf_8_byte_length += 1; // 1-byte character
    else if ((c & 0xE0) == 0xC0)
      utf_8_byte_length += 2; // 2-byte character
    else if ((c & 0xF0) == 0xE0)
      utf_8_byte_length += 3; // 3-byte character
    else if ((c & 0xF8) == 0xF0)
      utf_8_byte_length += 4; // 4-byte character
  }
  return utf_8_byte_length;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  for (char c : str) {
    if (c == delimiter) {
      if (!token.empty()) {
        tokens.push_back(token);
        token = "";
      }
    } else {
      token += c;
    }
  }

  if (!token.empty()) {
    tokens.push_back(token);
  }
  return tokens;
}

#endif // HUB_STRING_UTILS_H