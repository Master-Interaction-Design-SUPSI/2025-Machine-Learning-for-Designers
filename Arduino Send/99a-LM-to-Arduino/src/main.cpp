#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readString();
    Serial.println("Received: " + input);
    if (input == "on") {
      digitalWrite(LED_BUILTIN, HIGH);
    } else if (input == "off") {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
}