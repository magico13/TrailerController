#include "lin.h"

HardwareSerial linSerial(1);

// Break is 14 sets of 52 us, go lower for safety
const unsigned long BREAK_THRESHOLD = 675; 
const int MAX_BYTES = 11;
int dataIndex = 0;
unsigned long lastReceivedTime = 0;

void lin::setupSerial(int rxPin) {
    Serial.begin(115200); // For debugging output
    linSerial.begin(19200, SERIAL_8N1, rxPin); // LIN Serial
}

int lin::readFrame(byte dataBuffer[]) {
    dataIndex = 0;
    lastReceivedTime = micros();
    while (micros() - lastReceivedTime < BREAK_THRESHOLD) {
        while (linSerial.available() && dataIndex < MAX_BYTES) {
            // Read the incoming byte
            dataBuffer[dataIndex++] = linSerial.read();
            lastReceivedTime = micros(); // Update the last received time

            if (dataIndex >= MAX_BYTES) {
                break;
            }
        }
    }

    // Return the count of bytes read
    return dataIndex;
}