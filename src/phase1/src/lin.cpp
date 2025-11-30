#include "lin.h"

#define MAX_BYTES 11 // Maximum number of bytes in a LIN frame
// Break is 14 sets of 52 us (728)
const unsigned long BREAK_THRESHOLD = 650; // microseconds, smaller than 728 to allow some margin
short dataIndex = 0;
unsigned long lastReceivedTime = 0;
enum { WAIT_SYNC, RECEIVING, FRAME_COMPLETE } frameState;
bool frameOverflow = false;


void lin::setupSerial() {
    Serial1.begin(19200); // LIN bus
    frameState = WAIT_SYNC; // Initialize the frame state
}

short lin::updateFrame(byte expectedPID) {
    // Check if new bytes are available from Serial1
    while (Serial1.available()) {
        byte inByte = Serial1.read();
        lastReceivedTime = micros(); // reset the break timer on new byte
        
        // Look for sync byte if we haven't started a frame
        if (frameState == WAIT_SYNC) {
            if (inByte == 0x55) {
                dataBuffer[0] = inByte;
                dataIndex = 1;
                frameState = RECEIVING;
                frameOverflow = false;
            }
        } else if (frameState == RECEIVING) {
            // Optionally, check for expected PID at second byte
            if (dataIndex == 1 && expectedPID > 0 && inByte != expectedPID) {
                // Not the frame we are expecting, reset
                dataIndex = 0;
                frameState = WAIT_SYNC;
                frameOverflow = false;
                if (inByte == 0x55) {
                    dataBuffer[0] = inByte;
                    dataIndex = 1;
                    frameState = RECEIVING;
                }
            } else {
                if (dataIndex < MAX_BYTES) {
                    dataBuffer[dataIndex++] = inByte;
                } else {
                    frameOverflow = true;
                }
            }
        }
    }
    
    // Check for a break condition (i.e., no new bytes in a while)
    if (frameState == RECEIVING && (micros() - lastReceivedTime) >= BREAK_THRESHOLD) {
        frameState = FRAME_COMPLETE;
    }
    
    // Process the frame if complete
    if (frameState == FRAME_COMPLETE) {
        // Reset for next frame
        short length = dataIndex;
        bool droppedFrame = frameOverflow;
        dataIndex = 0;
        frameState = WAIT_SYNC;
        frameOverflow = false;
        if (!droppedFrame && length >= 2) { // minimal frame length check
            return length; // return the number of bytes read
        }
        return 0; // return 0 if frame is not valid
    }

    // If we reach here, we are still waiting for a frame
    return 0; // return 0 if no complete frame is available
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