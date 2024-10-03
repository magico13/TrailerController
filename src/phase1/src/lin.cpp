#include "lin.h"

// Break is 14 sets of 52 us, go lower for safety
const unsigned long BREAK_THRESHOLD = 728;
unsigned long lastReceivedTime = 0;

void lin::setupSerial() {
    Serial1.begin(19200); // LIN bus
}

short lin::readFrame(byte dataBuffer[], byte pid) {
    short dataIndex = 0;
    lastReceivedTime = micros();
    while (micros() - lastReceivedTime < BREAK_THRESHOLD) {
        while (Serial1.available() && dataIndex < MAX_BYTES) {
            // Read the incoming byte
            dataBuffer[dataIndex++] = Serial1.read();
            lastReceivedTime = micros(); // Update the last received time
            if (dataBuffer[0] != 0x55) {
                dataIndex = 0; // Reset the index if the first byte is not 0x55 (sync byte)
            }
            if (pid > 0 && dataIndex == 2 && dataBuffer[1] != pid) {
                dataIndex = 0; // Reset the index if the second byte is not the expected PID
            }
            if (dataIndex >= MAX_BYTES) {
                break;
            }
        }
    }
    if (dataIndex < 2) {
        return 0; // Return 0 if less than 2 bytes are read
    }

    // Return the count of bytes read
    return dataIndex;
}

byte lin::calculateChecksum(byte dataBuffer[], short length) {
    int checksum = 0;
    for (short i = 1; i < length; i++) { // skip the first byte, it's the sync byte
        checksum += dataBuffer[i];
        if (checksum > 0xFF) {
            checksum -= 0xFF;
        }
    }

    // invert the checksum bits
    return (byte)~checksum;
}