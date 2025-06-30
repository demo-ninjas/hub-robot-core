#ifndef HUB_DEFINITIONS_H
#define HUB_DEFINITIONS_H

#include <Arduino.h>

#ifndef DEC
#define DEC 10
#endif

#ifndef HEX
#define HEX 16
#endif

#ifndef OCT
#define OCT 8
#endif

#ifndef BIN
#define BIN 2
#endif

#ifndef LOW
#define LOW 0
#endif

#ifndef HIGH
#define HIGH 1
#endif

#ifndef INPUT
#define INPUT 0
#endif

#ifndef OUTPUT
#define OUTPUT 1
#endif

#ifndef constrain
#define constrain(val,min,max) ((val)<(min)?(min):((val)>(max)?(max):(val)))
#endif

class CachingPrinter : public Print {
    public: 
        virtual std::string tail(size_t lines = 20) const = 0;
        virtual void clear() = 0;
};

#endif // HUB_DEFINITIONS_H