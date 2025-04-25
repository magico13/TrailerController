#ifndef LIN_H
#define LIN_H

#include <Arduino.h>

class lin {
    public:
        void setupSerial();
        short readFrame(byte dataBuffer[], byte pid = 0);
        short updateFrame(byte expectedPID = 0);
        byte calculateChecksum(byte dataBuffer[], short length);

        byte dataBuffer[11]; // Store max of 11 bytes: sync, id, up to 8 data bytes, checksum
};

#endif // LIN_H