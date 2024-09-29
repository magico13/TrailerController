#include "lin.h"

HardwareSerial linSerial(1);

// Break is 14 sets of 52 us, go lower for safety
const unsigned long BREAK_THRESHOLD = 728; 
const int MAX_BYTES = 11;
int dataIndex = 0;
unsigned long lastReceivedTime = 0;

void lin::setupSerial(int rxPin) {
    Serial.begin(115200); // For debugging output
    linSerial.begin(19200, SERIAL_8N1, rxPin); // LIN Serial
}

int lin::readFrame(byte dataBuffer[], byte pid) {
    dataIndex = 0;
    lastReceivedTime = micros();
    while (micros() - lastReceivedTime < BREAK_THRESHOLD) {
        while (linSerial.available() && dataIndex < MAX_BYTES) {
            // Read the incoming byte
            dataBuffer[dataIndex++] = linSerial.read();
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

    // Return the count of bytes read
    return dataIndex;
}