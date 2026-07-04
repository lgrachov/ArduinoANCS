/*
 * ArduinoANCS - Apple Notification Center Service (ANCS) client for Arduino.
 *
 * Lets an ArduinoBLE-capable board (e.g. the Arduino Uno R4 WiFi) receive iOS
 * notification metadata (app id, title, subtitle, message, date) over BLE and
 * deliver it to your sketch through a simple callback.
 *
 * Usage:
 *   #include <ArduinoANCS.h>
 *
 *   void onNotification(const ANCSNotification& n) {
 *     Serial.print(n.appIdentifier); Serial.print(": ");
 *     Serial.print(n.title);         Serial.print(" - ");
 *     Serial.println(n.message);
 *   }
 *
 *   void setup() {
 *     Serial.begin(9600);
 *     ANCS.onNotification(onNotification);
 *     ANCS.begin();
 *   }
 *
 *   void loop() { ANCS.poll(); }
 *
 * The library owns the BLE stack (BLE.begin/advertise) by default so a sketch
 * can be a few lines. See begin() for advanced/manual BLE setups.
 */

#ifndef ARDUINO_ANCS_H
#define ARDUINO_ANCS_H

#include <Arduino.h>
#include <ArduinoBLE.h>

// ---- Tunable buffer sizes (override with -D or #define before include) -----
#ifndef ANCS_APP_ID_LEN
#define ANCS_APP_ID_LEN   48
#endif
#ifndef ANCS_TITLE_LEN
#define ANCS_TITLE_LEN    48
#endif
#ifndef ANCS_SUBTITLE_LEN
#define ANCS_SUBTITLE_LEN 48
#endif
#ifndef ANCS_MESSAGE_LEN
#define ANCS_MESSAGE_LEN  256
#endif
#ifndef ANCS_DATE_LEN
#define ANCS_DATE_LEN     16   // "YYYYMMDDTHHMMSS" + NUL
#endif
#ifndef ANCS_MAX_PENDING
#define ANCS_MAX_PENDING  4    // notifications awaiting metadata at once
#endif

// ---- ANCS enums ------------------------------------------------------------
// Notification Source EventID
enum ANCSEvent {
  ANCS_EVENT_ADDED    = 0,
  ANCS_EVENT_MODIFIED = 1,
  ANCS_EVENT_REMOVED  = 2,
};

// Notification Source EventFlags (bitmask)
enum ANCSFlags {
  ANCS_FLAG_SILENT          = (1 << 0),
  ANCS_FLAG_IMPORTANT       = (1 << 1),
  ANCS_FLAG_PRE_EXISTING    = (1 << 2),
  ANCS_FLAG_POSITIVE_ACTION = (1 << 3),
  ANCS_FLAG_NEGATIVE_ACTION = (1 << 4),
};

// Notification Source CategoryID
enum ANCSCategory {
  ANCS_CATEGORY_OTHER              = 0,
  ANCS_CATEGORY_INCOMING_CALL      = 1,
  ANCS_CATEGORY_MISSED_CALL        = 2,
  ANCS_CATEGORY_VOICEMAIL          = 3,
  ANCS_CATEGORY_SOCIAL             = 4,
  ANCS_CATEGORY_SCHEDULE           = 5,
  ANCS_CATEGORY_EMAIL              = 6,
  ANCS_CATEGORY_NEWS               = 7,
  ANCS_CATEGORY_HEALTH_AND_FITNESS = 8,
  ANCS_CATEGORY_BUSINESS_FINANCE   = 9,
  ANCS_CATEGORY_LOCATION           = 10,
  ANCS_CATEGORY_ENTERTAINMENT      = 11,
};

// Action IDs for performAction()
enum ANCSActionID {
  ANCS_ACTION_POSITIVE = 0,
  ANCS_ACTION_NEGATIVE = 1,
};

// ---- Parsed notification delivered to your callback ------------------------
struct ANCSNotification {
  uint32_t uid;            // unique id (use with performAction)
  uint8_t  event;          // ANCSEvent
  uint8_t  flags;          // ANCSFlags bitmask
  uint8_t  category;       // ANCSCategory
  uint8_t  categoryCount;  // # of notifications in this category

  char appIdentifier[ANCS_APP_ID_LEN];
  char title[ANCS_TITLE_LEN];
  char subtitle[ANCS_SUBTITLE_LEN];
  char message[ANCS_MESSAGE_LEN];
  char date[ANCS_DATE_LEN];

  // Human-readable category name (never null).
  const char* categoryName() const;

  bool hasPositiveAction() const { return flags & ANCS_FLAG_POSITIVE_ACTION; }
  bool hasNegativeAction() const { return flags & ANCS_FLAG_NEGATIVE_ACTION; }
};

typedef void (*ANCSNotificationCallback)(const ANCSNotification&);
typedef void (*ANCSEventCallback)();

// ---- The library ------------------------------------------------------------
class ANCSClass {
public:
  ANCSClass();

  // Start advertising and prepare to receive notifications.
  //   deviceName : name shown to iOS.
  //   manageBLE  : when true (default) the library calls BLE.begin() and
  //                advertises for you. Set false if your sketch already ran
  //                BLE.begin()/advertise() and manages its own services.
  // Returns false only if it tried and failed to start the BLE stack.
  bool begin(const char* deviceName = "Arduino-ANCS", bool manageBLE = true);

  // Stop and release resources.
  void end();

  // Pump the state machine. Call this every loop() iteration.
  void poll();

  // True once an iPhone is connected AND ANCS is subscribed (ready).
  bool ready() const { return _subscribed; }
  // True whenever an iPhone is connected at the link layer.
  bool connected() const { return _connected; }

  // Callback registration.
  void onNotification(ANCSNotificationCallback cb) { _onNotification = cb; }
  void onRemoved(ANCSNotificationCallback cb)      { _onRemoved = cb; }
  void onConnect(ANCSEventCallback cb)             { _onConnect = cb; }
  void onDisconnect(ANCSEventCallback cb)          { _onDisconnect = cb; }

  // Trigger a notification action (e.g. accept/decline a call). Requires the
  // notification to expose the matching action flag. Returns false if not
  // connected or the write fails.
  bool performAction(uint32_t uid, uint8_t actionId);

private:
  // --- connection lifecycle ---
  void handleConnect(BLEDevice& central);
  void handleDisconnect();
  bool discover(BLEDevice& central);
  bool subscribe();

  // --- data handling ---
  void readNotificationSource();
  void readDataSource();
  void requestAttributes(uint32_t uid);
  bool tryParseAssembly();

  // --- pending-notification bookkeeping ---
  ANCSNotification* addPending(uint32_t uid);
  ANCSNotification* findPending(uint32_t uid);
  void removePending(ANCSNotification* n);

  // Owned BLE objects (only used when manageBLE == true).
  BLEService        _dummyService;
  BLEByteCharacteristic _dummyChar;
  bool _manageBLE;

  // Discovered ANCS characteristics on the iPhone.
  BLECharacteristic _notificationSource;
  BLECharacteristic _controlPoint;
  BLECharacteristic _dataSource;

  // State machine flags.
  bool _connected;
  bool _discovered;
  bool _subscribed;
  unsigned long _lastAttempt;

  // Callbacks.
  ANCSNotificationCallback _onNotification;
  ANCSNotificationCallback _onRemoved;
  ANCSEventCallback        _onConnect;
  ANCSEventCallback        _onDisconnect;

  // Reassembly buffer for Data Source responses (may span several packets).
  uint8_t _asm[ANCS_MESSAGE_LEN + 128];
  size_t  _asmLen;

  // Pending notifications awaiting their metadata reply.
  ANCSNotification _pending[ANCS_MAX_PENDING];
  bool _pendingUsed[ANCS_MAX_PENDING];
};

extern ANCSClass ANCS;

#endif // ARDUINO_ANCS_H
