// Author: Jacob Bechmann Pedersen
// 2018-04-11
// GNU GPL v2

// Purpose:
// To give our Zumo Robots a model, by which to reaact to the opposing robots.
// The flowchart, that illustrates the FSM is as follows:
//                       _____
//                      |     |
//                      |Start|
//                      |_____|
//                         |
//                    OnButtonPress
//                         |
//                      ___v__
//                     |      |
//                     | Idle |
//                     |______|
//                         |
//                  On5SecondsPassed
//                         |
//                     ____v___
//                    |        |
//          --OnFind--| Search |<----
//         |          |________|     |
//      ___v___            |         |
//     |       |         OnEdge   OnSafe
//     | Chase |           |         |
//     |_______|       ____v___      |
//         |          |        |     |
//       OnEdge------>| BackUp |-----     
//                    |________|
//
// Method:
// In order to keep track of the states in the FSM, two enums are generated:
// One for states and one for events.
// 
// Then the updating of the state machine is all taken care of by the sensors in the loop function.
// Whenever a sensor meets certain set requirements, the event fires, and the state is updated accordingly.
//
// Then the code specific to run in that state runs, until the requirements are met for an event to set
// The machine out of the state.

// Let's first include all the relevant libraries:
#include <NewPing.h>
#include <ZumoShield.h>

// Zumo Pushtbutton ________________________________________________________________________________________

// The ZUMO_BUTTON is already defined, so let's use it:
Pushbutton button(ZUMO_BUTTON);

// And the update function wrapper:
bool buttonUpdate(void){
  return !button.isPressed();
}

// Ultrasound sensor _______________________________________________________________________________________

// Let's define all the important pins:
#define TRIGGER_PIN 11
#define ECHO_PIN 12

// The maximum distance for the ultrasonic sensor
#define MAX_DISTANCE 200
// And the threshold for closeness:
#define SONAR_THRESHOLD 50.0

// Then we instantiate a new object of type NewPing:
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// A variable to hold the read values:
// Float for higher precision
float pingRead = 0;

// And the update function:
float sonarUpdate(void){
  pingRead = (float)sonar.ping_median(5)/ US_ROUNDTRIP_CM; // The calculation done in floating point arithmetic.
  return pingRead;
}

// QTR Sensors _____________________________________________________________________________________________

// Firstly, some important pins:
#define NUM_SENSORS 6     // Number of sensors used
#define TIMEOUT 500  // The maximum amount of time to wait, before the sensors stop waiting.
#define EMITTER_PIN 2     // Emitter is unused, but mapped to pin 2.
#define QTR1 4
#define QTR2 A3
#define QTR3 11
#define QTR4 A0
#define QTR5 A2
#define QTR6 5

// Then the threshhold light value.
// lightersurface = lower number. The threshhold for whit surface is around 200.
#define QTRTHRESHOLD 200

// Viewed from the back and right to left, the Sensors are attached to the pins:
QTRSensorsRC qtrrc((unsigned char[]) {QTR1, QTR2, QTR3, QTR4, QTR5, QTR6},
  NUM_SENSORS, TIMEOUT, EMITTER_PIN);

unsigned int sensorValues[NUM_SENSORS]; // Array to hold the read values.

// And the update function:
int QTRUpdate(void){
  // Reads the sensor values, by waiting for the RC level to fall. 
  qtrrc.read(sensorValues);
  for(int i = 0; i < NUM_SENSORS; i++){
    if(sensorValues[i] < QTRTHRESHOLD){
      return i;
    }
  }
  return -1;
}

// State Machine ____________________________________________________________________________________________

// First, the states and events are enumerated:
enum States { Start, Idle, Search, Chase, BackUp };
enum Events { None, OnButtonPress, On5SecondsPassed, OnFind, OnEdge, OnSafe };

// Then the variables to hold the states themselves are defined:
static enum States zumoState = Start;
static enum Events zumoEvent = None;

// Prototypes of the state functiosn so they can be appended after setup() and loop()
void start(void);
void idle(void);
void search(void);
void chase(void);
void backUp(void);

// And finally the functions to handle the states:

// This function handles the event based state transitions:
void updateState(void){
  switch(zumoEvent){
    case None:
      break;
    case OnButtonPress:
      if(zumoState == Start){
        zumoState = Idle;
      }
      break;
    case On5SecondsPassed:
      if(zumoState == Idle){
        zumoState = Search;
      }
      break;
    case OnFind:
      if(zumoState == Search){
        zumoState = Chase;
      }
      break;
    case OnEdge:
      if(zumoState == Search){
        zumoState = BackUp;
      }
      if(zumoState == Chase){
        zumoState = BackUp;
      }
      break;
    case OnSafe:
      if(zumoState == BackUp){
        zumoState = Search;
      }
      break;
    default:
      break;
  }
}

// This loops, and checks all the sensors, to update the events, and thus change states:
void updateEvent(void){
  if(QTRUpdate() > -1){
    zumoEvent = OnEdge;
  }
  else if(sonarUpdate() < SONAR_THRESHOLD){
    zumoEvent = OnFind;
  }
  else if(buttonUpdate){
    zumoEvent = OnButtonPress;
  }
}

// This maps the states to the proper state-exclusives functionality:
void doState(void){
  switch (zumoState){
    case Start:
      start();
      break;
    case Idle:
      idle();
      break;
    case Search:
      search();
      break;
    case Chase:
      chase();
      break;
    case BackUp:
      backUp();
      break;
    default:
      break;
  }
}

// setup() and loop() ________________________________________________________________________________________

void setup(){
  Serial.begin(115200);
  pinMode(ZUMO_BUTTON, INPUT);
  zumoState = Start;
  zumoEvent = None;
  Serial.print("Event: ");
  Serial.println(zumoEvent);
  Serial.print("State: ");
  Serial.println(zumoState);
}

void loop(){
  updateEvent();
  Serial.print("Event: ");
  Serial.println(zumoEvent);
  updateState();
  Serial.print("State: ");
  Serial.println(zumoState);
  doState();
}

// The state functions _______________________________________________________________________________________

void start(void){
 
}
void idle(void){
  delay(5000);
  zumoEvent = On5SecondsPassed;
}
void search(void){
  
}
void chase(void){
  
}
void backUp(void){
  // Back up motors here.
}

