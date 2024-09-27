#ifndef LIN_H
#define LIN_H

#include <Arduino.h>
#include <HardwareSerial.h>

class lin {
    public:
        const int MAX_BYTES = 11; // Store max of 11 bytes: sync, id, up to 8 data bytes, checksum
        void setupSerial(int rxPin);
        int readFrame(byte dataBuffer[]);
};

#endif // LIN_H