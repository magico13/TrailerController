#include "lin.h"

#define MAX_BYTES 11 // Maximum number of bytes in a LIN frame
// Break is 14 sets of 52 us (728)
const unsigned long BREAK_THRESHOLD = 650; // microseconds, smaller than 728 to allow some margin
short dataIndex = 0;
unsigned long lastReceivedTime = 0;
enum { WAIT_SYNC, RECEIVING, FRAME_COMPLETE } frameState;
bool frameOverflow = false;
byte savedBuffer[MAX_BYTES]; // Temporary buffer for saving previous frame
short savedLength = 0;
bool hasSavedFrame = false;


void lin::setupSerial() {
    Serial1.begin(19200); // LIN bus
    frameState = WAIT_SYNC; // Initialize the frame state
}

short lin::updateFrame(byte expectedPID) {
    // If we have a pending new frame start (sync byte), restore it
    if (hasSavedFrame) {
        dataBuffer[0] = savedBuffer[0];
        dataIndex = 1;
        frameState = RECEIVING;
        frameOverflow = false;
        hasSavedFrame = false;
    }
    
    // Process all available bytes to clear the buffer quickly
    while (Serial1.available()) {
        unsigned long currentTime = micros();

        // If we were waiting for a break (idle time) to terminate the frame,
        // do it before we consume the next byte sitting in the FIFO. This
        // keeps us from treating the checksum as just another data byte when
        // frames arrive back-to-back.
        if (frameState == RECEIVING && (currentTime - lastReceivedTime) >= BREAK_THRESHOLD) {
            short length = dataIndex;
            bool droppedFrame = frameOverflow;
            dataIndex = 0;
            frameState = WAIT_SYNC;
            frameOverflow = false;
            if (!droppedFrame && length >= 2) {
                return length;
            }
            // Frame was invalid, restart loop without consuming the pending byte
            continue;
        }

        byte inByte = Serial1.read();
        currentTime = micros();
        
        // Check if this is a new frame start
        // If we see a sync byte (0x55) while already receiving, it's almost certainly
        // a new frame. Accept that we might occasionally split a frame that has 0x55
        // as data, but that's better than concatenating multiple frames together.
        if (frameState == RECEIVING && inByte == 0x55 && dataIndex >= 2 && !frameOverflow) {
            // We've hit a new frame - save this sync byte for next call
            savedBuffer[0] = inByte;
            savedLength = dataIndex;
            hasSavedFrame = true;
            lastReceivedTime = currentTime;
            
            // Return the current frame
            return dataIndex;
        }
        
        lastReceivedTime = currentTime;
        
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

                    // Hard stop at MAX_BYTES so a missing break or sync never
                    // lets the frame grow unbounded and corrupt the buffer.
                    if (dataIndex == MAX_BYTES) {
                        short length = dataIndex;
                        dataIndex = 0;
                        frameState = WAIT_SYNC;
                        frameOverflow = false;
                        return length;
                    }
                } else {
                    // Buffer overflow - frame is too long
                    // Save current byte if it's a sync for next call
                    if (inByte == 0x55) {
                        savedBuffer[0] = inByte;
                        hasSavedFrame = true;
                    }
                    
                    // Reset to look for next sync
                    frameOverflow = true;
                    dataIndex = 0;
                    frameState = WAIT_SYNC;
                    
                    // Return the overflowed frame (limited to MAX_BYTES to prevent buffer overrun)
                    return MAX_BYTES;
                }
            }
        }
    }

    // Check if we have a complete frame due to timeout (no more bytes available)
    if (frameState == RECEIVING && (micros() - lastReceivedTime) >= BREAK_THRESHOLD) {
        short length = dataIndex;
        bool droppedFrame = frameOverflow;
        dataIndex = 0;
        frameState = WAIT_SYNC;
        frameOverflow = false;
        if (!droppedFrame && length >= 2) {
            return length;
        }
    }

    return 0; // No complete frame available yet
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