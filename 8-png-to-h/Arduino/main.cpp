#include <Arduino.h>
/*------------------------------------------------------------------------
  Example sketch for Adafruit Thermal Printer library for Arduino.
  Demonstrates a few text styles & layouts, bitmap printing, etc.

  IMPORTANT: DECLARATIONS DIFFER FROM PRIOR VERSIONS OF THIS LIBRARY.
  This is to support newer & more board types, especially ones that don't
  support SoftwareSerial (e.g. Arduino Due).  You can pass any Stream
  (e.g. Serial1) to the printer constructor.  See notes below.
  ------------------------------------------------------------------------*/

#include "Adafruit_Thermal.h"

Adafruit_Thermal printer(&Serial0);      // Or Serial2, Serial3, etc.

uint8_t buffer[100000];
uint32_t totalBytesToRead = 0;
uint32_t bytesRead = 0;
bool readingImage = false;

// -----------------------------------------------------------------------

void setup() {

  // NOTE: SOME PRINTERS NEED 9600 BAUD instead of 19200, check test page.
  Serial.begin(9600);  // Initialize SoftwareSerial
  Serial0.begin(9600); // Use this instead if using hardware serial
  printer.begin();        // Init printer (same regardless of serial type)

  // The following calls are in setup(), but don't *need* to be.  Use them
  // anywhere!  They're just here so they run one time and are not printed
  // over and over (which would happen if they were in loop() instead).
  // Some functions will feed a line when called, this is normal.  
}

void printImageData(uint8_t *data, uint32_t length) {
  // If your image is always 384 wide, rowBytes = 384 / 8 = 48.
  int width = 384;
  int rowBytes = width / 8;
  int height = length / rowBytes; // derive height from total length

  Serial.print("Printing image: width=");
  Serial.print(width);
  Serial.print(", height=");
  Serial.println(height);

  printer.printBitmap(width, height, data);
  printer.feed(2);
}


void loop() {
  // 1. Wait for incoming data
  while (Serial.available()) {
    // If we haven't started reading image length, read that first
    if (!readingImage && totalBytesToRead == 0) {
      // For example, we read 4 bytes for length:
      static uint8_t lengthBytes[4];
      static uint8_t lengthByteCount = 0;

      lengthBytes[lengthByteCount++] = Serial.read();

      // Once we have 4 bytes, interpret them as length
      if (lengthByteCount == 4) {
        totalBytesToRead = (uint32_t)lengthBytes[0] |
                           ((uint32_t)lengthBytes[1] << 8) |
                           ((uint32_t)lengthBytes[2] << 16) |
                           ((uint32_t)lengthBytes[3] << 24);
        lengthByteCount = 0;
        readingImage = true;
        bytesRead = 0;

        Serial.println("Receiving image data...");
      }
    }
    // 2. Else read the image bytes
    else if (readingImage) {
      if (bytesRead < totalBytesToRead) {
        buffer[bytesRead++] = Serial.read();
      } else {
        // We either read everything, or ran out of space
        Serial.read(); // discard extra if any
      }

      // If we reached the total length
      if (bytesRead == totalBytesToRead) {
        // We have the full image data in 'buffer'
        Serial.println("Done receiving image");
        readingImage = false;
        // Now print it
        printImageData(buffer, totalBytesToRead);
        totalBytesToRead = 0;
        bytesRead = 0;
      }
    }
  }
}