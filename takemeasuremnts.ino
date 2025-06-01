#include "SparkFun_AS7265X.h" 
AS7265X sensor;

#include <Wire.h>

// Define custom I2C pins
#define SDA_PIN 6
#define SCL_PIN 7

// Array to store highest values for each of the 18 channels
float maxValues[18] = {0.0};

void setup()
{
  Serial.begin(115200);
  Serial.println("AS7265x Spectral Triad - Max Value Tracker");

  Wire.begin(SDA_PIN, SCL_PIN);  // Initialize I2C with custom pins

  Serial.println("Point the Triad away and press a key to begin...");
  while (Serial.available() == false) { }
  Serial.read(); //Clear input

  if (sensor.begin() == false)
  {
    Serial.println("Sensor not detected. Check wiring. Freezing...");
    while (1);
  }

  sensor.disableIndicator(); //Turn off status LED

  Serial.println("Tracking max values for each channel...");
  Serial.println("A,B,C,D,E,F,G,H,R,I,S,J,T,U,V,W,K,L");
}

void loop()
{
  sensor.takeMeasurementsWithBulb(); // Measure all 18 channels

  // Store current readings
  float currentValues[18] = {
    sensor.getCalibratedA(), //410nm
    sensor.getCalibratedB(), //435nm
    sensor.getCalibratedC(), //460nm
    sensor.getCalibratedD(), //485nm
    sensor.getCalibratedE(), //510nm
    sensor.getCalibratedF(), //535nm
    sensor.getCalibratedG(), //560nm
    sensor.getCalibratedH(), //585nm
    sensor.getCalibratedR(), //610nm
    sensor.getCalibratedI(), //645nm
    sensor.getCalibratedS(), //680nm
    sensor.getCalibratedJ(), //705nm
    sensor.getCalibratedT(), //730nm
    sensor.getCalibratedU(), //760nm
    sensor.getCalibratedV(), //810nm
    sensor.getCalibratedW(), //860nm
    sensor.getCalibratedK(), //900nm
    sensor.getCalibratedL()  //940nm
  };

  bool newMaxDetected = false;

  // Compare and update max values
  for (int i = 0; i < 18; i++)
  {
    if (currentValues[i] > maxValues[i])
    {
      maxValues[i] = currentValues[i];
      newMaxDetected = true;
    }
  }

  // If any new max detected, print the full set
  if (newMaxDetected)
  {
    for (int i = 0; i < 18; i++)
    {
      Serial.print(maxValues[i], 2); // Print with 2 decimal places
      Serial.print(",");
    }
    Serial.println();
  }

  delay(100); // Small delay to prevent flooding (adjust as needed)
}
