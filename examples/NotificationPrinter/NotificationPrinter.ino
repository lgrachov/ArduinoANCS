/*
 * NotificationPrinter - print iOS notification metadata over Serial.
 *
 * Board:   Arduino Uno R4 WiFi (or any ArduinoBLE-capable board)
 * Library: ArduinoANCS (depends on ArduinoBLE)
 *
 * First run:
 *   1. Upload, open Serial Monitor at 9600 baud.
 *   2. On the iPhone: Settings > Bluetooth > connect to "Arduino-ANCS".
 *   3. Accept the pairing prompt AND the "allow notifications" prompt.
 *   4. Trigger a notification (send yourself a text, etc.).
 *
 * If re-pairing fails, "Forget This Device" in iOS Bluetooth settings first.
 */

#include <ArduinoANCS.h>

void onNotification(const ANCSNotification& n) {
  Serial.println(F("--- Notification ---"));
  Serial.print(F("  App:      ")); Serial.println(n.appIdentifier);
  Serial.print(F("  Category: ")); Serial.println(n.categoryName());
  Serial.print(F("  Title:    ")); Serial.println(n.title);
  if (n.subtitle[0]) { Serial.print(F("  Subtitle: ")); Serial.println(n.subtitle); }
  Serial.print(F("  Message:  ")); Serial.println(n.message);
  Serial.print(F("  Date:     ")); Serial.println(n.date);
  Serial.print(F("  UID:      ")); Serial.println(n.uid);
  Serial.println(F("--------------------"));
}

void onRemoved(const ANCSNotification& n) {
  Serial.print(F("[removed] uid="));
  Serial.println(n.uid);
}

void onConnect()    { Serial.println(F("iPhone connected - pairing...")); }
void onDisconnect() { Serial.println(F("iPhone disconnected")); }

void setup() {
  Serial.begin(9600);
  while (!Serial);

  ANCS.onNotification(onNotification);
  ANCS.onRemoved(onRemoved);
  ANCS.onConnect(onConnect);
  ANCS.onDisconnect(onDisconnect);

  if (!ANCS.begin("Arduino-ANCS")) {
    Serial.println(F("Failed to start BLE!"));
    while (1);
  }

  Serial.println(F("ANCS ready. Waiting for iPhone..."));
}

void loop() {
  ANCS.poll();
}
