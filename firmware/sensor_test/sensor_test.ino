/*
 * Sensor Sanity Test Sketch for ESP32
 * Tests:
 * 1. INA219 (Voltage & Current via I2C)
 * 2. BH1750 (Irradiance proxy via I2C)
 * 3. Analog / Thermistor / Temp sensor (ADC Pin 34)
 */

#include <Wire.h>
#include <Adafruit_INA219.h>
#include <BH1750.h>

Adafruit_INA219 ina219;
BH1750 lightMeter;

const int TEMP_PIN = 34; // ADC1 pin for thermistor/LM35

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000);

  Serial.println("[INFO] Initializing I2C bus (SDA=21, SCL=22)...");
  Wire.begin(21, 22);

  if (!ina219.begin()) {
    Serial.println("[ERROR] Failed to find INA219 chip. Check wiring and address (default 0x40).");
  } else {
    Serial.println("[OK] INA219 initialized successfully.");
  }

  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("[ERROR] Failed to find BH1750 sensor. Check wiring (ADDR to GND = 0x23).");
  } else {
    Serial.println("[OK] BH1750 initialized successfully.");
  }

  pinMode(TEMP_PIN, INPUT);
  analogReadResolution(12);
  Serial.println("[INFO] Sensor sanity test started. Streaming readings...\n");
}

void loop() {
  float busVoltage_V = ina219.getBusVoltage_V();
  float current_mA = ina219.getCurrent_mA();
  float current_A = current_mA / 1000.0f;
  float power_W = busVoltage_V * current_A;

  float lux = lightMeter.readLightLevel();

  int rawTemp = analogRead(TEMP_PIN);
  float tempVoltage = (rawTemp / 4095.0f) * 3.3f;
  // Approximate LM35 conversion (10mV per deg C) or calibrated thermistor curve
  float temp_C = tempVoltage * 100.0f;

  Serial.print("V_bus: ");
  Serial.print(busVoltage_V, 2);
  Serial.print(" V | I: ");
  Serial.print(current_A, 3);
  Serial.print(" A | P: ");
  Serial.print(power_W, 2);
  Serial.print(" W | Light: ");
  Serial.print(lux, 1);
  Serial.print(" lx | Temp: ");
  Serial.print(temp_C, 1);
  Serial.println(" C");

  delay(2000);
}
