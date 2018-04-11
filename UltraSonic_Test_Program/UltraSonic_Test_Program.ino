// Author: Jacob Bechmann Pedersen
// 2018-04-11
// GNU GPL v2

// How does it work?
// The HC-SR04 module uses ultrasonic waves to localize objects.
// By sending a pulse signal over the trigger line, the SR04 produces a sound.
// This sound cannot be heard, other than by the SR04.
// The SR04 has an internal timer, that pulls a signal high from the time the trigger is heard,
// until the pulse has reflected unto the sensor again. This signal, has a pulse width corresponding to distance.
// Then the Arduino measures the signal length, and converts it to distance.

// To further help the accuracy of the measurements, a median filter can be applied.
#include <NewPing.h>

// Let's define the pins we use for the connection:
#define TRIGGER_PIN  11
#define ECHO_PIN     12

// Then the maximum distance:
#define MAX_DISTANCE 200

// Then we instantiate a new object of type NewPing:
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// A variable to hold the read values:
// Float for higher precision
float pingRead = 0;

void setup() {
  Serial.begin(9600);
}

void loop() {
  pingRead = (float)sonar.ping_median(5)/ US_ROUNDTRIP_CM; // The calculation done in floating point arithmetic.
  Serial.println(pingRead); // Print the results.
}
