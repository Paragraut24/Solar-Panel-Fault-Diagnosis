/*
 * Solar PV Fault Diagnosis - Main Edge Node Firmware
 * Target: ESP32
 *
 * Telemetry Fields:
 * - timestamp (epoch ms or ISO string)
 * - voltage_v
 * - current_a
 * - power_w
 * - panel_temp_c
 * - g_proxy (lux)
 * - load_state
 * - label (normal, partial_shading, open_circuit, soiling, line_fault)
 * - experiment_id (e.g. EXP-001)
 */

#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <BH1750.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

// WiFi Configuration
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// Firebase RTDB Configuration
#define API_KEY "YOUR_FIREBASE_API_KEY"
#define DATABASE_URL "https://YOUR_PROJECT_ID-default-rtdb.firebaseio.com/"

// Experiment Tracking Metadata
const char* EXPERIMENT_ID = "EXP-001";
const char* CURRENT_LABEL = "normal";     // normal, partial_shading, open_circuit, soiling, line_fault
const char* LOAD_STATE = "resistive";      // resistive, open, dynamic

// Sensor Instances and Pin Assignment
Adafruit_INA219 ina219;
BH1750 lightMeter;
const int TEMP_PIN = 34; // ADC1 pin for panel temperature sensor
const unsigned long SEND_INTERVAL_MS = 2000;

// Firebase State
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool firebaseReady = false;
unsigned long lastSendTime = 0;

// NTP Time Configuration
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 19800; // IST: UTC+5:30
const int DAYLIGHT_OFFSET_SEC = 0;

unsigned long getEpochTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return millis();
  }
  time(&now);
  return (unsigned long)now;
}

void initWiFi() {
  Serial.print("[WIFI] Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WIFI] Connected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n[WIFI] Connection failed. Running in offline/serial mode.");
  }
}

void initFirebase() {
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.signer.test_mode = true; // anonymous / unauthenticated rule or testing

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(1024);
  firebaseReady = true;
  Serial.println("[FIREBASE] Initialized client.");
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("==================================================");
  Serial.println("Solar PV Fault Diagnosis - ESP32 Telemetry Node");
  Serial.println("==================================================");

  Wire.begin(21, 22);

  if (!ina219.begin()) {
    Serial.println("[WARN] INA219 not detected at 0x40. Check wiring.");
  } else {
    Serial.println("[OK] INA219 ready.");
  }

  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("[WARN] BH1750 not detected at 0x23. Check wiring.");
  } else {
    Serial.println("[OK] BH1750 ready.");
  }

  pinMode(TEMP_PIN, INPUT);
  analogReadResolution(12);

  initWiFi();
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  initFirebase();
}

void loop() {
  if (millis() - lastSendTime >= SEND_INTERVAL_MS) {
    lastSendTime = millis();

    float busVoltage = ina219.getBusVoltage_V();
    float current_mA = ina219.getCurrent_mA();
    float current_A = current_mA / 1000.0f;
    float power_W = busVoltage * current_A;

    float lux = lightMeter.readLightLevel();

    int rawTemp = analogRead(TEMP_PIN);
    float tempVoltage = (rawTemp / 4095.0f) * 3.3f;
    float temp_C = tempVoltage * 100.0f; // Calibrate for sensor type (LM35/NTC)

    unsigned long timestamp = getEpochTime();

    // Local Serial Output (CSV format for backup logging)
    Serial.printf("%lu,%s,%s,%.2f,%s,%.2f,%.3f,%.2f,%.1f,%.1f\n",
                  timestamp, EXPERIMENT_ID, CURRENT_LABEL, 0.0f, LOAD_STATE,
                  busVoltage, current_A, power_W, temp_C, lux);

    // Push to Firebase RTDB under telemetry path
    if (WiFi.status() == WL_CONNECTED && Firebase.ready()) {
      FirebaseJson json;
      json.set("timestamp", (int64_t)timestamp);
      json.set("voltage_v", busVoltage);
      json.set("current_a", current_A);
      json.set("power_w", power_W);
      json.set("panel_temp_c", temp_C);
      json.set("g_proxy", lux);
      json.set("load_state", LOAD_STATE);
      json.set("label", CURRENT_LABEL);
      json.set("experiment_id", EXPERIMENT_ID);

      if (!Firebase.RTDB.pushJSON(&fbdo, "/telemetry", &json)) {
        Serial.printf("[FIREBASE ERROR] Reason: %s\n", fbdo.errorReason().c_str());
      }
    }
  }
}
