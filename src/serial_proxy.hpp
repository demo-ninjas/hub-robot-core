#ifndef SERIAL_PROXY_HPP
#define SERIAL_PROXY_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <Arduino.h>
#include <Wire.h>
#include <cstring>
#include <new>
#include "definitions.h"

// SerialProxy
// -------------
// A Print-compatible proxy that mirrors all writes to the global Serial object (if initialized)
// while storing a ring buffer of recent output for later retrieval (tail()).
//
// Key characteristics:
//  - Ring buffer with overwrite on wrap (always contains most recent data up to capacity).
//  - Memory allocation failure resilience (silent no-op buffering if alloc fails).
//  - Reduced duplicated write logic via helper functions.
//  - Additional convenience overloads for String and C strings.
//  - tail() scans backward for newline-delimited lines; returns what is available.
//  - Provides size(), capacity(), wrapped() introspection helpers.
//  - Non-copyable (avoids double free), movable.

class SerialProxy : public CachingPrinter {
    private:
        size_t buf_size_;          // Total capacity
        size_t head_;              // Next write index
        bool full_;                // Whether buffer has wrapped at least once
        byte* buf_;                // Ring buffer storage

        inline void appendByte(uint8_t c) {
            if (!buf_) return; // Allocation failed safeguard
            buf_[head_] = c;
            head_ = (head_ + 1) % buf_size_;
            if (head_ == 0) { full_ = true; }
        }

        inline void appendBuffer(const uint8_t* data, size_t len) {
            if (!buf_ || !data || len == 0) return;
            for (size_t i = 0; i < len; ++i) {
                appendByte(data[i]);
            }
        }

    public:
        explicit SerialProxy(size_t buf_size = 2048)
            : buf_size_(buf_size), head_(0), full_(false), buf_(nullptr) {
            if (buf_size_ == 0) buf_size_ = 1; // Prevent zero-sized allocation
            buf_ = new (std::nothrow) byte[buf_size_];
            if (buf_) {
                memset(buf_, 0, buf_size_);
            }
        }

        SerialProxy(const SerialProxy&) = delete;
        SerialProxy& operator=(const SerialProxy&) = delete;

        SerialProxy(SerialProxy&& other) noexcept
            : buf_size_(other.buf_size_), head_(other.head_), full_(other.full_), buf_(other.buf_) {
            other.buf_ = nullptr; other.buf_size_ = 0; other.head_ = 0; other.full_ = false;
        }
        SerialProxy& operator=(SerialProxy&& other) noexcept {
            if (this != &other) {
                delete[] buf_;
                buf_size_ = other.buf_size_; head_ = other.head_; full_ = other.full_; buf_ = other.buf_;
                other.buf_ = nullptr; other.buf_size_ = 0; other.head_ = 0; other.full_ = false;
            }
            return *this;
        }

        ~SerialProxy() override {
            delete[] buf_;
        }

        size_t write(uint8_t c) override {
            if (Serial) { Serial.write(c); }
            appendByte(c);
            return 1;
        }

        size_t write(const uint8_t *buffer, size_t size) override {
            if (!buffer || size == 0) return 0;
            if (Serial) { Serial.write(buffer, size); }
            appendBuffer(buffer, size);
            return size;
        }

        size_t write(const char *str) {
            if (!str) return 0;
            size_t len = std::strlen(str);
            if (Serial) { Serial.write(str, len); }
            appendBuffer(reinterpret_cast<const uint8_t*>(str), len);
            return len;
        }

        size_t write(const char *buffer, size_t size) {
            if (!buffer || size == 0) return 0;
            if (Serial) { Serial.write(buffer, size); }
            appendBuffer(reinterpret_cast<const uint8_t*>(buffer), size);
            return size;
        }

        size_t write(const char *buffer, size_t size, size_t offset) {
            if (!buffer || size == 0) return 0;
            size_t total_len = std::strlen(buffer);
            if (offset >= total_len) return 0;
            size_t clamped = std::min(size, total_len - offset);
            if (Serial) { Serial.write(buffer + offset, clamped); }
            appendBuffer(reinterpret_cast<const uint8_t*>(buffer + offset), clamped);
            return clamped;
        }

        size_t write(const String &str) {
            size_t len = str.length();
            if (len == 0) return 0;
            if (Serial) { Serial.write(str.c_str(), len); }
            appendBuffer(reinterpret_cast<const uint8_t*>(str.c_str()), len);
            return len;
        }

        void flush() override {
            if (Serial) { Serial.flush(); }
        }

        std::string tail(size_t lines = 20) const override {
            if (!buf_ || lines == 0) return std::string();
            size_t stored = full_ ? buf_size_ : head_;
            if (stored == 0) return std::string();

            size_t idx = (head_ == 0) ? (stored - 1) : (head_ - 1);
            size_t newline_count = 0;
            std::string out;
            out.reserve(stored);

            bool loop = true;
            while (loop) {
                char ch = static_cast<char>(buf_[idx]);
                out.push_back(ch);
                if (ch == '\n') {
                    ++newline_count;
                    if (newline_count == lines) break;
                }

                if (idx == 0) {
                    if (!full_) break; // reached start
                    idx = buf_size_ - 1; // wrap
                    if (idx == head_) loop = false; // buffer has only one char
                } else {
                    idx--;
                    if (!full_ && idx >= head_) { loop = false; } // shouldn't happen but defensive
                    if (full_ && idx == head_) { break; } // full wrap completed
                }
            }

            std::reverse(out.begin(), out.end());
            return out;
        }

        void clear() override {
            head_ = 0; full_ = false;
            if (buf_) { memset(buf_, 0, buf_size_); }
        }

        size_t size() const { return full_ ? buf_size_ : head_; }
        size_t capacity() const { return buf_size_; }
        bool wrapped() const { return full_; }
};

#endif // SERIAL_PROXY_HPP