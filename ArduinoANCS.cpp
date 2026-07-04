#include "ArduinoANCS.h"

// ---- ANCS UUIDs ------------------------------------------------------------
#define ANCS_SERVICE_UUID       "7905f431-b5ce-4e99-a40f-4b1e122d00d0"
#define ANCS_NOTIF_SOURCE_UUID  "9fbf120d-6301-42d9-8c58-25e699a21dbd"
#define ANCS_CONTROL_POINT_UUID "69d1d8f3-45e1-49a8-9821-9bbdfdaad9d9"
#define ANCS_DATA_SOURCE_UUID   "22eac6e9-24d6-4bb5-be44-b36ace7c7bfb"

// ---- Control Point CommandIDs ----------------------------------------------
#define CMD_GET_NOTIFICATION_ATTRIBUTES 0
#define CMD_PERFORM_NOTIFICATION_ACTION 2

// ---- Notification attribute IDs --------------------------------------------
#define ATTR_APP_IDENTIFIER 0
#define ATTR_TITLE          1
#define ATTR_SUBTITLE       2
#define ATTR_MESSAGE        3
#define ATTR_DATE           5

// Number of attributes we request per notification (app,title,subtitle,msg,date).
#define ANCS_EXPECTED_ATTRS 5

// Retry cadence for discovery/subscription (ms).
#define ANCS_RETRY_INTERVAL_MS 500

// Global singleton.
ANCSClass ANCS;

// ---- Small helpers ---------------------------------------------------------
static uint32_t readUID(const uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void copyText(char* dst, size_t cap, const uint8_t* src, uint16_t len) {
  if (cap == 0) return;
  size_t n = (len < cap - 1) ? len : cap - 1;
  memcpy(dst, src, n);
  dst[n] = '\0';
}

static const char* const CATEGORY_NAMES[] = {
  "Other", "IncomingCall", "MissedCall", "Voicemail", "Social",
  "Schedule", "Email", "News", "HealthAndFitness", "BusinessAndFinance",
  "Location", "Entertainment"
};

const char* ANCSNotification::categoryName() const {
  if (category < (sizeof(CATEGORY_NAMES) / sizeof(CATEGORY_NAMES[0]))) {
    return CATEGORY_NAMES[category];
  }
  return "Unknown";
}

// ---- Lifecycle -------------------------------------------------------------
ANCSClass::ANCSClass()
  : _dummyService("180A"),
    _dummyChar("2A3D", BLERead | BLEWrite | BLEEncryption),
    _manageBLE(true),
    _connected(false),
    _discovered(false),
    _subscribed(false),
    _lastAttempt(0),
    _onNotification(nullptr),
    _onRemoved(nullptr),
    _onConnect(nullptr),
    _onDisconnect(nullptr),
    _asmLen(0) {
  for (int i = 0; i < ANCS_MAX_PENDING; i++) _pendingUsed[i] = false;
}

bool ANCSClass::begin(const char* deviceName, bool manageBLE) {
  _manageBLE = manageBLE;

  if (_manageBLE) {
    if (!BLE.begin()) {
      return false;
    }

    BLE.setLocalName(deviceName);
    BLE.setDeviceName(deviceName);

    // Advertising an encrypted characteristic gives iOS a reason to bond,
    // which is what unlocks the (encrypted-only) ANCS service.
    _dummyService.addCharacteristic(_dummyChar);
    BLE.addService(_dummyService);
    _dummyChar.writeValue(0);
    BLE.setAdvertisedService(_dummyService);

    BLE.setPairable(true);
    BLE.advertise();
  }

  return true;
}

void ANCSClass::end() {
  if (_manageBLE) {
    BLE.stopAdvertise();
    BLE.end();
  }
  handleDisconnect();
}

// ---- Main pump -------------------------------------------------------------
void ANCSClass::poll() {
  BLE.poll();  // keep the stack + pairing state machine turning

  BLEDevice central = BLE.central();
  bool linkUp = central && central.connected();

  if (!linkUp) {
    if (_connected) handleDisconnect();
    return;
  }

  if (!_connected) handleConnect(central);

  // Step 1: discover ANCS once, throttled so we don't spin.
  if (!_discovered) {
    if (millis() - _lastAttempt < ANCS_RETRY_INTERVAL_MS) return;
    _lastAttempt = millis();
    if (!discover(central)) return;
    _discovered = true;
  }

  // Step 2: keep trying to subscribe until iOS bonds and accepts the CCCD
  // write (the rejected attempts are what trigger the pairing prompt).
  if (!_subscribed) {
    if (millis() - _lastAttempt < ANCS_RETRY_INTERVAL_MS) return;
    _lastAttempt = millis();
    if (!subscribe()) return;
    _subscribed = true;
  }

  // Ready: service incoming notifications and metadata replies.
  readNotificationSource();
  readDataSource();
}

void ANCSClass::handleConnect(BLEDevice& central) {
  (void)central;
  _connected = true;
  _discovered = false;
  _subscribed = false;
  _asmLen = 0;
  _lastAttempt = 0;
  if (_onConnect) _onConnect();
}

void ANCSClass::handleDisconnect() {
  bool was = _connected;
  _connected = false;
  _discovered = false;
  _subscribed = false;
  _asmLen = 0;
  for (int i = 0; i < ANCS_MAX_PENDING; i++) _pendingUsed[i] = false;
  if (was && _onDisconnect) _onDisconnect();
}

// ---- Discovery + subscription ---------------------------------------------
bool ANCSClass::discover(BLEDevice& central) {
  if (!central.discoverAttributes()) return false;

  BLEService ancs = central.service(ANCS_SERVICE_UUID);
  if (!ancs) return false;

  _notificationSource = ancs.characteristic(ANCS_NOTIF_SOURCE_UUID);
  _controlPoint       = ancs.characteristic(ANCS_CONTROL_POINT_UUID);
  _dataSource         = ancs.characteristic(ANCS_DATA_SOURCE_UUID);

  return _notificationSource && _controlPoint && _dataSource;
}

bool ANCSClass::subscribe() {
  // Data Source first so we never miss a reply, then the Notification Source.
  if (!_dataSource.subscribe()) return false;
  if (!_notificationSource.subscribe()) return false;
  return true;
}

// ---- Notification Source ---------------------------------------------------
void ANCSClass::readNotificationSource() {
  if (!_notificationSource || !_notificationSource.valueUpdated()) return;

  uint8_t buf[8];
  int n = _notificationSource.readValue(buf, sizeof(buf));
  if (n < 8) return;

  uint32_t uid = readUID(&buf[4]);
  uint8_t event = buf[0];

  if (event == ANCS_EVENT_REMOVED) {
    // No metadata fetch for removals; deliver the header immediately.
    ANCSNotification n2;
    n2.uid = uid;
    n2.event = event;
    n2.flags = buf[1];
    n2.category = buf[2];
    n2.categoryCount = buf[3];
    n2.appIdentifier[0] = n2.title[0] = n2.subtitle[0] = '\0';
    n2.message[0] = n2.date[0] = '\0';
    if (_onRemoved) _onRemoved(n2);
    return;
  }

  // Added / Modified: record it and ask iOS for the details.
  ANCSNotification* p = addPending(uid);
  if (!p) return;  // pending table full; drop
  p->uid = uid;
  p->event = event;
  p->flags = buf[1];
  p->category = buf[2];
  p->categoryCount = buf[3];
  p->appIdentifier[0] = p->title[0] = p->subtitle[0] = '\0';
  p->message[0] = p->date[0] = '\0';

  requestAttributes(uid);
}

void ANCSClass::requestAttributes(uint32_t uid) {
  if (!_controlPoint) return;

  uint8_t cmd[] = {
    CMD_GET_NOTIFICATION_ATTRIBUTES,
    (uint8_t)(uid), (uint8_t)(uid >> 8), (uint8_t)(uid >> 16), (uint8_t)(uid >> 24),
    ATTR_APP_IDENTIFIER,
    ATTR_TITLE,    (uint8_t)(ANCS_TITLE_LEN - 1),    0x00,
    ATTR_SUBTITLE, (uint8_t)(ANCS_SUBTITLE_LEN - 1), 0x00,
    ATTR_MESSAGE,  (uint8_t)((ANCS_MESSAGE_LEN - 1) & 0xFF),
                   (uint8_t)(((ANCS_MESSAGE_LEN - 1) >> 8) & 0xFF),
    ATTR_DATE
  };
  _controlPoint.writeValue(cmd, sizeof(cmd));
}

// ---- Data Source (metadata replies, possibly fragmented) -------------------
void ANCSClass::readDataSource() {
  if (!_dataSource || !_dataSource.valueUpdated()) return;

  uint8_t buf[256];
  int n = _dataSource.readValue(buf, sizeof(buf));
  if (n <= 0) return;

  // Append to the reassembly buffer, guarding against overflow.
  if (_asmLen + (size_t)n > sizeof(_asm)) {
    _asmLen = 0;  // give up on this oversized response and resync
    return;
  }
  memcpy(_asm + _asmLen, buf, n);
  _asmLen += n;

  if (tryParseAssembly()) {
    _asmLen = 0;
  }
}

// Returns true once a full response has been parsed and delivered.
bool ANCSClass::tryParseAssembly() {
  if (_asmLen < 5) return false;

  uint32_t uid = readUID(&_asm[1]);
  ANCSNotification* p = findPending(uid);
  if (!p) {
    // Reply for an unknown UID - discard to resync.
    _asmLen = 0;
    return false;
  }

  size_t i = 5;
  int parsed = 0;
  while (i + 3 <= _asmLen) {
    uint8_t  attrId = _asm[i];
    uint16_t alen   = (uint16_t)_asm[i + 1] | ((uint16_t)_asm[i + 2] << 8);
    if (i + 3 + alen > _asmLen) return false;  // need more fragments

    const uint8_t* val = &_asm[i + 3];
    switch (attrId) {
      case ATTR_APP_IDENTIFIER: copyText(p->appIdentifier, ANCS_APP_ID_LEN, val, alen); break;
      case ATTR_TITLE:          copyText(p->title,         ANCS_TITLE_LEN, val, alen);  break;
      case ATTR_SUBTITLE:       copyText(p->subtitle,      ANCS_SUBTITLE_LEN, val, alen); break;
      case ATTR_MESSAGE:        copyText(p->message,       ANCS_MESSAGE_LEN, val, alen); break;
      case ATTR_DATE:           copyText(p->date,          ANCS_DATE_LEN, val, alen);   break;
      default: break;
    }

    i += 3 + alen;
    if (++parsed >= ANCS_EXPECTED_ATTRS) {
      if (_onNotification) _onNotification(*p);
      removePending(p);
      return true;
    }
  }
  return false;  // incomplete - wait for more data
}

// ---- Pending-notification table -------------------------------------------
ANCSNotification* ANCSClass::addPending(uint32_t uid) {
  // Reuse an existing slot for the same UID (a Modified event updates it).
  ANCSNotification* existing = findPending(uid);
  if (existing) return existing;

  for (int i = 0; i < ANCS_MAX_PENDING; i++) {
    if (!_pendingUsed[i]) {
      _pendingUsed[i] = true;
      return &_pending[i];
    }
  }
  return nullptr;  // table full
}

ANCSNotification* ANCSClass::findPending(uint32_t uid) {
  for (int i = 0; i < ANCS_MAX_PENDING; i++) {
    if (_pendingUsed[i] && _pending[i].uid == uid) return &_pending[i];
  }
  return nullptr;
}

void ANCSClass::removePending(ANCSNotification* n) {
  for (int i = 0; i < ANCS_MAX_PENDING; i++) {
    if (&_pending[i] == n) { _pendingUsed[i] = false; return; }
  }
}

// ---- Actions ---------------------------------------------------------------
bool ANCSClass::performAction(uint32_t uid, uint8_t actionId) {
  if (!_subscribed || !_controlPoint) return false;

  uint8_t cmd[] = {
    CMD_PERFORM_NOTIFICATION_ACTION,
    (uint8_t)(uid), (uint8_t)(uid >> 8), (uint8_t)(uid >> 16), (uint8_t)(uid >> 24),
    actionId
  };
  return _controlPoint.writeValue(cmd, sizeof(cmd));
}
