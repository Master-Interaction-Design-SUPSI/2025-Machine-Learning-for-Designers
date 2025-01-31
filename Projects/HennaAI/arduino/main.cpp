#include <Arduino.h>
#include <Adafruit_Thermal.h>
#include <Adafruit_NeoPixel.h>

/*------------------------------------------------------------------------
  This code prints incoming images on a thermal printer and toggles
  a NeoPixel ring color based on the button press. Each button press
  sends "pushed" over Serial to the host PC, which toggles audio
  recording in your existing JavaScript code.
  ------------------------------------------------------------------------*/

// ------------------- Thermal Printer Configuration -------------------
Adafruit_Thermal printer(&Serial0);      // Using hardware Serial0

// ------------------- NeoPixel Configuration --------------------------
#define LED_PIN     9      // Pin for NeoPixel data line
#define LED_COUNT   60     // Number of LEDs in the ring/strip
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// ------------------- Image Buffer -----------------------------------
uint8_t  buffer[100000];
uint32_t totalBytesToRead = 0;
uint32_t bytesRead        = 0;
bool     readingImage     = false;

// ------------------- Button Configuration ---------------------------
const int BUTTON_PIN    = 10;  // Define button pin
bool lastButtonState    = HIGH; // Track previous button state

// We'll track "recording" to decide LED color
bool isRecording        = false;

// -----------------------------------------------------------------------
// Setup
// -----------------------------------------------------------------------
void setup() {
  // NOTE: Some printers need 9600 baud instead of 19200. 
  //       Make sure it matches your printer's requirements.
  Serial.begin(9600);   // USB serial (to the PC)
  Serial0.begin(9600);  // Hardware serial for the thermal printer
  printer.begin();      // Init printer

  // Initialize button as INPUT_PULLUP (HIGH when not pressed)
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize NeoPixel ring
  strip.begin();
  strip.setBrightness(50);
  strip.clear();
  strip.show();

  // (Optional) tiny delay to stabilize input reading
  delay(50);
  lastButtonState = digitalRead(BUTTON_PIN);
}

// -----------------------------------------------------------------------
// Print the received image data on the thermal printer
// (Assumes width=384, thus rowBytes=48, and height derived from length)
// -----------------------------------------------------------------------
void printImageData(uint8_t *data, uint32_t length) {
  int width    = 384;
  int rowBytes = width / 8;
  int height   = length / rowBytes;

  Serial.print("Printing image: width=");
  Serial.print(width);
  Serial.print(", height=");
  Serial.println(height);

  printer.printBitmap(width, height, data);
  printer.feed(2);
}

// -----------------------------------------------------------------------
// Main Loop
// -----------------------------------------------------------------------
void loop() {
  // ---------------------------------------------------------------------
  // 1) BUTTON CHECK
  // ---------------------------------------------------------------------
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Detect a rising->falling transition => "press"
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    // Send "pushed" to the PC via Serial
    Serial.println("pushed");

    // Toggle our local isRecording state
    isRecording = !isRecording;

    // Simple debounce
    delay(200);
  }
  lastButtonState = currentButtonState;

  // ---------------------------------------------------------------------
  // 2) UPDATE NEOPIXELS
  // ---------------------------------------------------------------------
  // Red if recording, green if not (choose your own colors!)
  if (isRecording) {
    // Set all pixels to red
    for (uint16_t i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(255, 0, 0));
    }
  } else {
    // Set all pixels to green
    for (uint16_t i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, strip.Color(0, 255, 0));
    }
  }
  strip.show();

  // ---------------------------------------------------------------------
  // 3) READ INCOMING IMAGE DATA FROM PC => PRINT
  // ---------------------------------------------------------------------
  // If the browser is sending an image, read 4 bytes for length, then read
  // the full image into `buffer`, and call `printImageData()`.
  while (Serial.available()) {
    // If we're not reading an image yet, get the 4-byte length
    if (!readingImage && totalBytesToRead == 0) {
      static uint8_t lengthBytes[4];
      static uint8_t lengthByteCount = 0;

      lengthBytes[lengthByteCount++] = Serial.read();

      if (lengthByteCount == 4) {
        totalBytesToRead = (uint32_t)lengthBytes[0] |
                           ((uint32_t)lengthBytes[1] << 8) |
                           ((uint32_t)lengthBytes[2] << 16) |
                           ((uint32_t)lengthBytes[3] << 24);

        lengthByteCount = 0;
        readingImage    = true;
        bytesRead       = 0;

        Serial.println("Receiving image data...");
      }
    }
    // Otherwise, read the actual image bytes
    else if (readingImage) {
      if (bytesRead < totalBytesToRead) {
        buffer[bytesRead++] = Serial.read();
      } else {
        // If extra bytes come in, discard
        Serial.read();
      }

      // Once we've received the full length
      if (bytesRead == totalBytesToRead) {
        Serial.println("Done receiving image");
        readingImage     = false;
        // Print
        printImageData(buffer, totalBytesToRead);
        // Reset for next possible image
        totalBytesToRead = 0;
        bytesRead        = 0;
      }
    }
  }
}
