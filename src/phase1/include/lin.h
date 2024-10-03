#ifndef LIN_H
#define LIN_H

#include <Arduino.h>

class lin {
    public:
        static const short MAX_BYTES = 11; // Store max of 11 bytes: sync, id, up to 8 data bytes, checksum
        void setupSerial();
        short readFrame(byte dataBuffer[], byte pid = 0);
        byte calculateChecksum(byte dataBuffer[], short length);
};

#endif // LIN_H