#ifndef SERIAL_PROXY_HPP
#define SERIAL_PROXY_HPP

#include <string>
#include <vector>
#include <Arduino.h>
#include <Wire.h>
#include "definitions.h"

class SerialProxy : public CachingPrinter {
    private:
      size_t buf_size;
      size_t current_index;
      byte* buf;
    
    public:
        SerialProxy(size_t buf_size = 2048) : buf_size(buf_size), current_index(0) {
            buf = (byte*)malloc(buf_size);
        }

        ~SerialProxy() {
            free(buf);
        }
    
        size_t write(uint8_t c) override {
            if (Serial) {
                Serial.write(c);
            }

            if (current_index < buf_size) {
                buf[current_index] = c;
                current_index++;
            } else {
                // Buffer is full, so go back to the start of the buffer
                buf[0] = c;
                current_index = 1;
            }
            
            return 1;
        }

        void flush() {
            if (Serial) {
                Serial.flush();
            }
        }

        size_t write(const uint8_t *buffer, size_t size) override {
            if (Serial) {
                Serial.write(buffer, size);
            }

            for (size_t i = 0; i < size; ++i) {
                if (current_index < buf_size) {
                    buf[current_index] = buffer[i];
                    current_index++;
                } else {
                    // Buffer is full, so go back to the start of the buffer
                    buf[0] = buffer[i];
                    current_index = 1;
                }
            }
            return size;
        }

        size_t write(const char *str) {
            if (Serial) {
                Serial.write(str);
            }

            size_t len = strlen(str);
            for (size_t i = 0; i < len; ++i) {
                if (current_index < buf_size) {
                    buf[current_index] = str[i];
                    current_index++;
                } else {
                    // Buffer is full, so go back to the start of the buffer
                    buf[0] = str[i];
                    current_index = 1;
                }
            }
            return len;
        }

        size_t write(const char *buffer, size_t size) {
            if (Serial) {
                Serial.write(buffer, size);
            }

            for (size_t i = 0; i < size; ++i) {
                if (current_index < buf_size) {
                    buf[current_index] = buffer[i];
                    current_index++;
                } else {
                    // Buffer is full, so go back to the start of the buffer
                    buf[0] = buffer[i];
                    current_index = 1;
                }
            }
            return size;
        }

        size_t write(const char *buffer, size_t size, size_t offset) {
            if (Serial) {
                Serial.write(buffer + offset, size);
            }

            for (size_t i = 0; i < size; ++i) {
                if (current_index < buf_size) {
                    buf[current_index] = buffer[i + offset];
                    current_index++;
                } else {
                    // Buffer is full, so go back to the start of the buffer
                    buf[0] = buffer[i + offset];
                    current_index = 1;
                }
            }
            return size;
        }

        size_t write(const String &str) {
            if (Serial) {
                Serial.write(str.c_str(), str.length());
            }

            size_t len = str.length();
            for (size_t i = 0; i < len; ++i) {
                if (current_index < buf_size) {
                    buf[current_index] = str[i];
                    current_index++;
                } else {
                    // Buffer is full, so go back to the start of the buffer
                    buf[0] = str[i];
                    current_index = 1;
                }
            }
            return len;
        }

        std::string tail(size_t lines = 20) const {
            // Read back from the current position in the buffer, collecting lines upto the number of lines requested
            size_t line_count = 0;
            size_t i = current_index;
            while (i > 0) {
                if (buf[i] == '\n') {
                    line_count++;
                    if (line_count == lines) {
                        break;
                    }
                }
                i--;
            }

            std::string tail_string(buf + i, buf + current_index);
            if (line_count < lines) {
                // We didn't find enough lines in the buffer, so we need to wrap around to the end of the buffer
                i = buf_size - 1;
                while (i > current_index) {
                    if (buf[i] == '\0') {
                        break;
                    }
                    if (buf[i] == '\n') {
                        line_count++;
                        if (line_count == lines) {
                            break;
                        }
                    }
                    i--;
                }

                tail_string += std::string(buf + i, buf + buf_size);
            }
            return tail_string;
        }

        void clear() {
            current_index = 0;
            memset(buf, 0, buf_size);
        }
};

#endif // SERIAL_PROXY_HPP