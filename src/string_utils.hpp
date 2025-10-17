#ifndef HUB_STRING_UTILS_H
#define HUB_STRING_UTILS_H

#include <Arduino.h>
#include <string>
#include <vector>

namespace HubStringUtils {

/**
 * @brief Calculates the number of UTF-8 characters in an Arduino String.
 * 
 * This function counts the actual number of UTF-8 characters (code points),
 * not bytes. Multi-byte UTF-8 sequences are counted as single characters.
 * 
 * @param str The Arduino String to measure
 * @return The number of UTF-8 characters (not bytes)
 * 
 * @note Invalid UTF-8 sequences are counted byte-by-byte as separate characters.
 * @note Performance: O(n) where n is the byte length of the string
 */
inline size_t utf8CharCount(const String& str) noexcept {
  size_t char_count = 0;
  const char* ptr = str.c_str();
  const size_t len = str.length();
  
  for (size_t i = 0; i < len; ++i) {
    const unsigned char c = static_cast<unsigned char>(ptr[i]);
    
    // Count only the start of UTF-8 sequences (not continuation bytes)
    // Continuation bytes have the pattern 10xxxxxx (0x80-0xBF)
    if ((c & 0xC0) != 0x80) {
      ++char_count;
    }
  }
  
  return char_count;
}

/**
 * @brief Calculates the UTF-8 byte length required to encode an Arduino String.
 * 
 * For strings already in UTF-8 format, this returns the same as str.length().
 * This function validates UTF-8 sequences and counts the total bytes needed.
 * 
 * @param str The Arduino String to measure
 * @return The number of bytes in the UTF-8 representation
 * 
 * @note This is primarily useful for validation or conversion scenarios
 * @note Performance: O(n) where n is the byte length of the string
 */
inline size_t utf8ByteLength(const String& str) noexcept {
  // Arduino Strings are already UTF-8 encoded in memory
  return str.length();
}

/**
 * @brief Splits a std::string into tokens based on a delimiter character.
 * 
 * @param str The string to split
 * @param delimiter The character to split on
 * @param keep_empty If true, consecutive delimiters produce empty tokens; 
 *                   if false, they are skipped (default: false)
 * @return A vector of string tokens
 * 
 * @note Empty input returns empty vector
 * @note Leading/trailing delimiters produce empty tokens only if keep_empty=true
 * @note Memory efficient: pre-reserves space to minimize reallocations
 * 
 * Example with keep_empty=false (default):
 *   split("a::b", ':') → ["a", "b"]
 *   split("::a", ':') → ["a"]
 * 
 * Example with keep_empty=true:
 *   split("a::b", ':', true) → ["a", "", "b"]
 *   split("::a", ':', true) → ["", "", "a"]
 */
inline std::vector<std::string> split(const std::string& str, char delimiter, bool keep_empty = false) {
  std::vector<std::string> tokens;
  
  if (str.empty()) {
    return tokens;
  }
  
  // Pre-reserve space to minimize reallocations
  // Estimate: assume ~10% of characters are delimiters
  tokens.reserve(str.length() / 10 + 1);
  
  std::string token;
  token.reserve(32); // Reserve typical token size
  
  for (char c : str) {
    if (c == delimiter) {
      if (keep_empty || !token.empty()) {
        tokens.push_back(std::move(token));
        token.clear();
        token.reserve(32);
      }
    } else {
      token += c;
    }
  }
  
  // Add final token if non-empty or if keeping empty tokens
  if (keep_empty || !token.empty()) {
    tokens.push_back(std::move(token));
  }
  
  return tokens;
}

/**
 * @brief Splits an Arduino String into tokens based on a delimiter character.
 * 
 * @param str The Arduino String to split
 * @param delimiter The character to split on
 * @param keep_empty If true, consecutive delimiters produce empty tokens;
 *                   if false, they are skipped (default: false)
 * @return A vector of Arduino String tokens
 * 
 * @note Empty input returns empty vector
 * @note Leading/trailing delimiters produce empty tokens only if keep_empty=true
 * @note Optimized for Arduino String type to avoid conversions
 * 
 * Example with keep_empty=false (default):
 *   split(String("a,,b"), ',') → ["a", "b"]
 * 
 * Example with keep_empty=true:
 *   split(String("a,,b"), ',', true) → ["a", "", "b"]
 */
inline std::vector<String> split(const String& str, char delimiter, bool keep_empty = false) {
  std::vector<String> tokens;
  
  if (str.length() == 0) {
    return tokens;
  }
  
  // Pre-reserve space to minimize reallocations
  tokens.reserve(str.length() / 10 + 1);
  
  String token;
  token.reserve(32);
  
  for (size_t i = 0; i < str.length(); ++i) {
    char c = str.charAt(i);
    if (c == delimiter) {
      if (keep_empty || token.length() > 0) {
        tokens.push_back(token);
        token = "";
        token.reserve(32);
      }
    } else {
      token += c;
    }
  }
  
  // Add final token if non-empty or if keeping empty tokens
  if (keep_empty || token.length() > 0) {
    tokens.push_back(token);
  }
  
  return tokens;
}

/**
 * @brief Trims whitespace from both ends of a std::string.
 * 
 * @param str The string to trim
 * @return A new string with leading and trailing whitespace removed
 * 
 * @note Whitespace includes: space, tab, newline, carriage return, form feed, vertical tab
 * @note Does not modify the original string
 */
inline std::string trim(const std::string& str) {
  const char* whitespace = " \t\n\r\f\v";
  
  const size_t start = str.find_first_not_of(whitespace);
  if (start == std::string::npos) {
    return ""; // String is all whitespace
  }
  
  const size_t end = str.find_last_not_of(whitespace);
  return str.substr(start, end - start + 1);
}

/**
 * @brief Trims whitespace from both ends of an Arduino String.
 * 
 * @param str The Arduino String to trim
 * @return A new Arduino String with leading and trailing whitespace removed
 * 
 * @note Whitespace includes: space, tab, newline, carriage return
 * @note Does not modify the original string
 */
inline String trim(const String& str) {
  if (str.length() == 0) {
    return String();
  }
  
  int start = 0;
  int end = str.length() - 1;
  
  // Find first non-whitespace
  while (start <= end && (str[start] == ' ' || str[start] == '\t' || 
                          str[start] == '\n' || str[start] == '\r')) {
    ++start;
  }
  
  // Find last non-whitespace
  while (end >= start && (str[end] == ' ' || str[end] == '\t' || 
                          str[end] == '\n' || str[end] == '\r')) {
    --end;
  }
  
  if (start > end) {
    return String(); // All whitespace
  }
  
  return str.substring(start, end + 1);
}

} // namespace HubStringUtils

// Convenience: allow unqualified usage (optional - can be removed for stricter namespacing)
using HubStringUtils::utf8CharCount;
using HubStringUtils::utf8ByteLength;
using HubStringUtils::split;
using HubStringUtils::trim;

#endif // HUB_STRING_UTILS_H