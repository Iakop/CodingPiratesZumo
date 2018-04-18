/*
 * This example uses the Pushbutton library to demonstrate three different
 * methods for detecting a button press and release. It blinks the yellow
 * user LED each time the Zumo user button is pressed and released. You can
 * still use this demo without a Zumo Shield if you connect a normally-open
 * momentary pushbutton between digital pin 12 and ground on your Arduino.
 */

#include <Wire.h>
#include <ZumoShield.h>

#define LED_PIN 13

/*
 * Create a Pushbutton object for pin 12 (the Zumo user pushbutton pin)
 * with default settings, which enables the internal pull-up on the pin
 * and interprets a HIGH pin value as the default (unpressed) state of the
 * button. (Optional arguments can be passed to the constructor to specify
 * other button types and connection methods; see the documentation for
 * details.)
 */
Pushbutton button(12);

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop()
{
{
    if (button.getSingleDebouncedRelease())
    {
      // blink LED
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Button Pressed!");
  
  delay(200);
  digitalWrite(LED_PIN, LOW);
  Serial.println("Button Ready!");

    }
}
  
}
