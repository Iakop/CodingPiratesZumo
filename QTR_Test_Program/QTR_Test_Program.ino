// Author: Pololu
// Modified and Commented by: Jacob Bechmann Pedersen
// 2018-04-11
// GNU GPL v2

// The QTR Sensors on the Zumo Robot measure the reflectance of a surface by using an RC circuit.
// Then the decay time is measured.
// To put the operation simply:
//    -Turn on IR LEDs (optional).
//    -Make the I/O line connected to that sensor an output and drive it high.
//    -Wait several microseconds to give the 1 nF capacitor node time to reach 5 V.
//    -Make the I/O line an input (with internal pull-up disabled).
//    -Measure the time for the voltage to decay by waiting for the I/O line to go low.
//    -Turn off IR LEDs (optional).
// 
// Read more here: https://www.pololu.com/product/1419

#include <Wire.h>
#include <ZumoShield.h>

#define NUM_SENSORS   6     // Number of sensors used
#define TIMEOUT       500  // The maximum amount of time to wait, before the sensors stop waiting.
#define EMITTER_PIN   2     // Emitter is unused, but mapped to pin 2.

// Viewed from the back and right to left, the Sensors are attached to the pins:
QTRSensorsRC qtrrc((unsigned char[]) {4, A3, 11, A0, A2, 5},
  NUM_SENSORS, TIMEOUT, EMITTER_PIN);
unsigned int sensorValues[NUM_SENSORS]; // Array to hold the read values.

void setup()
{
  Serial.begin(9600); // set the data rate in bits per second for serial data transmission
}

void loop()
{
  // Reads the sensor values, by waiting for the RC level to fall. 
  qtrrc.read(sensorValues);

  // Print the values measured. If the Sensor is fully lit, the RC circuit will discharge very quickly,
  // And thus giv a low readout. LOW VALUE = LIGHT SURFACE.
  for (unsigned char i = 0; i < NUM_SENSORS; i++)
  {
    Serial.print(sensorValues[i]);
    Serial.print('\t'); // Inserts a tab after every data value.
  }
  Serial.println(); // Print a new line.

  delay(250); // Dealy for readability.
}
