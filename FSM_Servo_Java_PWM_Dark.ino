#include "SparkFun_AS7265X.h"
#include <WiFiMulti.h>
#include <WiFi.h>
WiFiMulti wifiMulti;
WiFiServer server(8080); // TCP Server for Java communication

#define DEVICE "ESP32S3"
#include <Wire.h>
#include <ESP32Servo.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define WIFI_SSID "Spectral_Sensor_Lasha"
#define WIFI_PASSWORD "12345678"
#define INFLUXDB_URL "http://192.168.43.146:8086"
#define INFLUXDB_TOKEN "2BRAKtJRgMheO1anQa5HBys158Ih3b2tp0EEessDFn_aTVYnaVTeDE-8HDdeemVNuqHFkg00Wr9b-qmc1BGpIw=="
#define INFLUXDB_ORG "d8481950cdbfec67"
#define INFLUXDB_BUCKET "From_ESP32S3"

#define MOSFET_GATE_PIN_WHITE 13
#define MOSFET_GATE_PIN_IR 14
#define SDA_PIN 6
#define SCL_PIN 7
#define SERVO_PIN 11

// PWM settings for ESP32
#define LED_FREQUENCY 5000
#define LED_RESOLUTION 8
#define LED_BRIGHTNESS 20  // 50% brightness (0-255 scale)

AS7265X sensor;
Servo myServo;

enum State {
  READY,
  WAIT_FOR_LEAF,
  WAIT_FOR_EXTRACTION
};

State currentState = READY;
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);
Point sensorData("spectral_data");
IPAddress local_IP(192, 168, 43, 226);  // Set static IP
IPAddress gateway(192, 168, 43, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns(8, 8, 8, 8);  // Use Google's DNS 

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(MOSFET_GATE_PIN_WHITE, OUTPUT);
  pinMode(MOSFET_GATE_PIN_IR, OUTPUT);
  
  ledcAttach(MOSFET_GATE_PIN_WHITE, LED_FREQUENCY, LED_RESOLUTION);
  ledcAttach(MOSFET_GATE_PIN_IR, LED_FREQUENCY, LED_RESOLUTION);
  
  ledcWrite(MOSFET_GATE_PIN_WHITE, 0);
  ledcWrite(MOSFET_GATE_PIN_IR, 0);
  myServo.attach(SERVO_PIN);
  myServo.write(90);

  WiFi.config(local_IP, gateway, subnet, dns);  
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) delay(100);  // Wait for connection

  server.begin(); // Start TCP server

  timeSync("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.nis.gov");

  Serial.print("ESP32 IP Address: ");
Serial.println(WiFi.localIP());


  if (!sensor.begin()) {
    Serial.println("Sensor not found!");
    while (1);
  }
  sensor.disableIndicator();
  Serial.println("Ready. Press 's' to start.");
}


void loop() {
  handleSerialInput();
  handleWiFiInput();
}

void handleSerialInput() {
  if (Serial.available()) {
    processCommand(Serial.read());
  }
}

void handleWiFiInput() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("New TCP Client Connected");

  while (client.connected()) {
    if (client.available()) {
      String command = client.readStringUntil('\n');  // Read full command until newline
      command.trim();  // Remove any extra spaces or newline characters
      Serial.print("Command received: ");
      Serial.println(command);
      delay(50);  // Small delay to allow buffer processing

      processCommand(command[0]);  // Process only the first character (e.g., 's')
    }
  }

  client.stop();
  Serial.println("TCP Client Disconnected");
}


void processCommand(char command) {
  switch (currentState) {
    case READY:
      if (command == 's') {
        closeCase();  // Close the case only once
        for (int i = 0; i < 10; i++) {
          startMeasurementCycle();
        }
        openCase();  // Open case after measurements
      }
      break;

    case WAIT_FOR_LEAF:
      if (command == 'p') {
        closeCase();  // Close the case only once
        for (int i = 0; i < 10; i++) {
          measureReflectanceNominator();
        }
        openCase();  // Open case after measurements
      }
      break;

    case WAIT_FOR_EXTRACTION:
      if (command == 'f') resetToReady();
      break;
  }
}

void closeCase() {
  Serial.println("Closing case...");
  myServo.write(70);
  delay(1000);
  myServo.write(90);
  delay(1000);
}

void openCase() {
  Serial.println("Opening case...");
  myServo.write(120);
  delay(1000);
  myServo.write(90);
}


void startMeasurementCycle() {
  measureReflectanceTranscendenceDenominatorDark();
}

void measureReflectanceTranscendenceDenominatorDark() {
  Serial.println("Measuring Reflectance & Transcendence Denominator Dark...");
  takeMeasurement("Reflectance_Denominator_Dark");
  takeMeasurement("Transcendence_Denominator_Dark");
  measureTranscendenceDenominator();
}

void measureTranscendenceDenominator() {
  Serial.println("Measuring Transcendence Denominator...");
  ledcWrite(MOSFET_GATE_PIN_WHITE, LED_BRIGHTNESS);
  ledcWrite(MOSFET_GATE_PIN_IR, LED_BRIGHTNESS);
  delay(50);
  takeMeasurement("Transcendence_Denominator");
  delay(50);
  ledcWrite(MOSFET_GATE_PIN_WHITE, 0);
  ledcWrite(MOSFET_GATE_PIN_IR, 0);
  currentState = WAIT_FOR_LEAF;
}

void measureReflectanceNominator() {
  Serial.println("Measuring Reflectance Nominator...");
  takeMeasurementWithBulb("Reflectance_Nominator");
  measureReflectanceTranscendenceNominatorDark();
}

void measureReflectanceTranscendenceNominatorDark() {
  Serial.println("Measuring Reflectance & Transcendence Nominator Dark...");
  takeMeasurement("Reflectance_Nominator_Dark");
  takeMeasurement("Transcendence_Nominator_Dark");
  measureTranscendenceNominator();
}

void measureTranscendenceNominator() {
  Serial.println("Measuring Transcendence Nominator...");
  ledcWrite(MOSFET_GATE_PIN_WHITE, LED_BRIGHTNESS);
  ledcWrite(MOSFET_GATE_PIN_IR, LED_BRIGHTNESS);
  delay(50);
  takeMeasurement("Transcendence_Nominator");
  delay(50);
  ledcWrite(MOSFET_GATE_PIN_WHITE, 0);
  ledcWrite(MOSFET_GATE_PIN_IR, 0);
  currentState = WAIT_FOR_EXTRACTION;
}

void resetToReady() {
  Serial.println("Returning to Ready state. Press 's' to start again.");
  currentState = READY;
}

void takeMeasurementWithBulb(const char* measurementType) {
  sensorData.clearFields();
  sensorData.clearTags();
  sensorData.addTag("measurement_type", measurementType);
  sensor.takeMeasurementsWithBulb();
  storeMeasurementData();
}

void takeMeasurement(const char* measurementType) {
  sensorData.clearFields();
  sensorData.clearTags();
  sensorData.addTag("measurement_type", measurementType);
  sensor.takeMeasurements();
  storeMeasurementData();
}

void storeMeasurementData() {
  sensorData.addField("410nm", sensor.getCalibratedA());
  sensorData.addField("435nm", sensor.getCalibratedB());
  sensorData.addField("460nm", sensor.getCalibratedC());
  sensorData.addField("485nm", sensor.getCalibratedD());
  sensorData.addField("510nm", sensor.getCalibratedE());
  sensorData.addField("535nm", sensor.getCalibratedF());
  sensorData.addField("560nm", sensor.getCalibratedG());
  sensorData.addField("585nm", sensor.getCalibratedH());
  sensorData.addField("610nm", sensor.getCalibratedR());
  sensorData.addField("645nm", sensor.getCalibratedI());
  sensorData.addField("680nm", sensor.getCalibratedS());
  sensorData.addField("705nm", sensor.getCalibratedJ());
  sensorData.addField("730nm", sensor.getCalibratedT());
  sensorData.addField("760nm", sensor.getCalibratedU());
  sensorData.addField("810nm", sensor.getCalibratedV());
  sensorData.addField("860nm", sensor.getCalibratedW());
  sensorData.addField("900nm", sensor.getCalibratedK());
  sensorData.addField("940nm", sensor.getCalibratedL());
  
  if (!client.writePoint(sensorData)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
        delay(100);  // Allow network buffer to recover
  } else {
    Serial.println("Data written to InfluxDB");
        delay(200);  // Allow network buffer to recover
  }
}
