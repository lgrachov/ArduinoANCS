/*
 * CallResponder - demonstrate performAction() by auto-declining incoming calls.
 *
 * When an incoming-call notification arrives that exposes a negative action,
 * this sketch triggers it (declining the call). Use responsibly :)
 *
 * Board: Arduino Uno R4 WiFi. Requires the ArduinoANCS + ArduinoBLE libraries.
 */

#include <ArduinoANCS.h>

void onNotification(const ANCSNotification& n) {
  Serial.print(n.categoryName());
  Serial.print(F(" from "));
  Serial.println(n.title);

  if (n.category == ANCS_CATEGORY_INCOMING_CALL && n.hasNegativeAction()) {
    Serial.println(F("  -> declining call"));
    ANCS.performAction(n.uid, ANCS_ACTION_NEGATIVE);
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  ANCS.onNotification(onNotification);
  ANCS.begin("Arduino-ANCS");

  Serial.println(F("Ready. Pair your iPhone."));
}

void loop() {
  ANCS.poll();
}
