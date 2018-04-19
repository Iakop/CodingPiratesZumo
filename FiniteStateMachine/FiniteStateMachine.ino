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
// The sensors are updated internally in the states, so irrelevant sensor input is ignored on a statewise basis.
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
  bool btnPress = button.getSingleDebouncedPress();
  Serial.print("Button: ");
  Serial.println(btnPress);
  return btnPress;
}

// Ultrasound sensor _______________________________________________________________________________________

// Let's define all the important pins:
#define TRIGGER_PIN 11
#define ECHO_PIN 2

// And for the left:
#define TRIGGER_PIN_LEFT A0
#define ECHO_PIN_LEFT A1
// And right:
#define TRIGGER_PIN_RIGHT A4
#define ECHO_PIN_RIGHT A5

// The maximum distance for the ultrasonic sensor
#define MAX_DISTANCE 100
// And the threshold for closeness:
#define SONAR_THRESHOLD 30

// Then we instantiate a new object of type NewPing:
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
// And two more for left and right sensing:
NewPing sonarLeft(TRIGGER_PIN_LEFT, ECHO_PIN_LEFT, MAX_DISTANCE);
NewPing sonarRight(TRIGGER_PIN_RIGHT, ECHO_PIN_RIGHT, MAX_DISTANCE);

// A variable to hold the read values:
int pingRead = 0;
int pingReadLeft = 0;
int pingReadRight = 0;

// And the update function:
int sonarUpdate(void){
  pingRead = sonar.ping_median(3) / US_ROUNDTRIP_CM; // The calculation done in floating point arithmetic.
  Serial.print("pingRead: ");
  Serial.println(pingRead);
  return pingRead;
}
// Plus left/right variants:
int sonarUpdateLeft(void){
  pingReadLeft = sonarLeft.ping_median(3) / US_ROUNDTRIP_CM; // The calculation done in floating point arithmetic.
  Serial.print("pingReadLeft: ");
  Serial.println(pingReadLeft);
  return pingReadLeft;
}
int sonarUpdateRight(void){
  pingReadRight = sonarRight.ping_median(3) / US_ROUNDTRIP_CM; // The calculation done in floating point arithmetic.
  Serial.print("pingReadRight: ");
  Serial.println(pingReadRight);
  return pingReadRight;
}

// QTR Sensors _____________________________________________________________________________________________

// Firstly, some important pins:
#define NUM_SENSORS 4 //6    // Number of sensors used - 2 in this case, as we don't want to waste time on the others.
#define TIMEOUT 500  // The maximum amount of time to wait, before the sensors stop waiting.
#define QTR1 4
#define QTR2 A3
//#define QTR3 11
//#define QTR4 A0
#define QTR5 A2
#define QTR6 5

// Then the threshhold light value.
// lightersurface = lower number. The threshhold for whit surface is around 200.
#define QTRTHRESHOLD 300

// Viewed from the back and right to left, the Sensors are attached to the pins:
QTRSensorsRC qtrrc((unsigned char[]) {QTR1, QTR2, /*QTR3, QTR4,*/ QTR5, QTR6}, NUM_SENSORS, TIMEOUT);

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

// Motors ____________________________________________________________________________________________

// Initializes The motors:
ZumoMotors motors;

// Constants to set the motor speeds:
#define REVERSE_SPEED     400 
#define TURN_SPEED        300
#define FORWARD_SPEED     400
#define SEARCH_SPEED      200
// Duration of backing, and turn:
#define REVERSE_DURATION  200 // ms
#define TURN_DURATION     300 // ms

// State Machine ____________________________________________________________________________________________

// First, the states and events are enumerated:
enum States { Start, Idle, Search, Chase, BackUp, TurnLeft, TurnRight };
enum Events { None, OnButtonPress, On5SecondsPassed, OnFind, OnEdge, OnSafe, OnLost, OnLeft, OnRight };

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
      if(zumoState == TurnLeft){
        zumoState = Chase;
      }
      if(zumoState == TurnRight){
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
      if(zumoState == TurnLeft){
        zumoState = BackUp;
      }
      if(zumoState == TurnRight){
        zumoState = BackUp;
      }
      break;
    case OnSafe:
      if(zumoState == BackUp){
        zumoState = Search;
      }
      break;
    case OnLost:
      if(zumoState == Chase){
        zumoState = Search;
      }
      break;
    case OnLeft:
      if(zumoState == Search){
        zumoState = TurnLeft;
      }
      break;
    case OnRight:
      if(zumoState == Search){
        zumoState = TurnRight;
      }
      break;
    default:
      break;
  }
}

// This loops, and checks all the sensors, to update the events, and thus change states:
void updateEvent(void){
  
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
    case TurnLeft:
      turnLeft();
      break;
    case TurnRight:
      turnRight();
      break;
    default:
      break;
  }
}

// Timers ____________________________________________________________________________________________________

// Some of the states require timers, and some also require similar mechanisms to function properly:

// The five seconds wait in the idle state needs a flag to keep track of start, and firing:
bool startIdleTimer = false;
bool stopIdleTimer = false;
unsigned long long int idleTimer;

// Handles the idleTimer.
bool updateIdle(void){
  if (startIdleTimer && millis() >= idleTimer + 5000 && !stopIdleTimer){
    stopIdleTimer = true;
    return true;
  }
  else{
    return false;
  }
}

// Both idle and search need a timer to track their actions properly.
unsigned long long int otteTalsTimer;

// init() function ___________________________________________________________________________________________

// Initializes the components of the robot in the beginning of the setup() function.
void initialize(){
  pinMode(ZUMO_BUTTON, INPUT);
  zumoState = Start;
  zumoEvent = None;
  idleTimer = millis();
  otteTalsTimer = millis();
}

// setup() and loop() ________________________________________________________________________________________

void setup(){
  initialize();
  Serial.begin(115200);
  /*Serial.print("Event: ");
  Serial.println(zumoEvent);
  Serial.print("State: ");
  Serial.println(zumoState);*/
}

void loop(){
  updateEvent();
  /*Serial.print("Event: ");
  Serial.println(zumoEvent);*/
  updateState();
  /*Serial.print("State: ");
  Serial.println(zumoState);*/
  doState();
}

// The state functions _______________________________________________________________________________________

void start(void){
  if(buttonUpdate()){
    zumoEvent = OnButtonPress;
  }
}

void idle(void){
  // Starts the idle timer once, to count down the time.
  if(!stopIdleTimer && !startIdleTimer){
    idleTimer = millis();
    startIdleTimer = true;
  }
  // When both stopIdleTimer and startIdletimer turn true, the time has gone by,
  // and the updateIdle() function fires the new event.
  if(updateIdle()){
    zumoEvent = On5SecondsPassed;
    return;
  }
}

void search(void){
  // Set the speeds counter each other for left and right, to spin:
  /*if(otteTal() == -1){
      zumoEvent = OnEdge;
      return; // Instantly return, to not waste any time.
  }*/
  
  
  // Update QTR for edge detection, to fire OnEdge as fast as possible.
  if(QTRUpdate() > -1){
    zumoEvent = OnEdge;
    return; // Instantly return, to not waste any time.
  }
  motors.setSpeeds(SEARCH_SPEED/2, SEARCH_SPEED);
  /*
  unsigned long long int millisRead = millis();  
  if (millisRead > otteTalsTimer && millisRead < otteTalsTimer + 1000){
    motors.setSpeeds(400 , 125);
  }
  else if (millisRead > otteTalsTimer + 1000 && millisRead < otteTalsTimer + 4000){
    motors.setSpeeds(125 , 400);
  }
  else if (millisRead > otteTalsTimer + 4000 && millisRead < otteTalsTimer + 6000){
    motors.setSpeeds(400 , 125);
  }
  else{
    otteTalsTimer = millis();
  }*/

  int sonarRead = sonarUpdate();
  int sonarReadLeft = sonarUpdateLeft();
  int sonarReadRight = sonarUpdateRight();
  // If no edge is seen, check for the opponent:
  if(sonarRead < SONAR_THRESHOLD && sonarRead > 0){
    motors.setSpeeds(0, 0);
    delay(50);
    zumoEvent = OnFind;
    return;
  }
  // Check for the opponent on left:
  else if(sonarReadLeft < SONAR_THRESHOLD && sonarReadLeft > 0){
    zumoEvent = OnLeft;
    return;
  }
  // Check for the opponent on Right:
  else if(sonarReadRight < SONAR_THRESHOLD && sonarReadRight > 0){
    zumoEvent = OnRight;
    return;
  }
}

void chase(void){
  motors.setSpeeds(FORWARD_SPEED, FORWARD_SPEED);
  // Update QTR for edge detection, to fire OnEdge as fast as possible.
  if(QTRUpdate() > -1){
    zumoEvent = OnEdge;
    return; // Instantly return, to not waste any time.
  }
}

void backUp(void){
  if(sensorValues[0] < sensorValues[NUM_SENSORS-1]){
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED/2);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
  }
  if(sensorValues[0] > sensorValues[NUM_SENSORS-1]){
    motors.setSpeeds(-REVERSE_SPEED/2, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
    delay(TURN_DURATION);
  }
  else{
    motors.setSpeeds(-REVERSE_SPEED, -REVERSE_SPEED);
    delay(REVERSE_DURATION);
    motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
    delay(TURN_DURATION);
  }
  zumoEvent = OnSafe;
}

void turnLeft(void){
  motors.setSpeeds(-TURN_SPEED, TURN_SPEED);
  // check for the opponent:
  
  // Update QTR for edge detection, to fire OnEdge as fast as possible.
  if(QTRUpdate() > -1){
    zumoEvent = OnEdge;
    return; // Instantly return, to not waste any time.
  }
  int sonarRead = sonarUpdate();
  // If no edge is seen, check for the opponent:
  if(sonarRead < SONAR_THRESHOLD && sonarRead > 0){
    motors.setSpeeds(0, 0);
    delay(50);
    zumoEvent = OnFind;
    return;
  }
}

void turnRight(void){
  motors.setSpeeds(TURN_SPEED, -TURN_SPEED);
  // check for the opponent:
  
  // Update QTR for edge detection, to fire OnEdge as fast as possible.
  if(QTRUpdate() > -1){
    zumoEvent = OnEdge;
    return; // Instantly return, to not waste any time.
  }

  int sonarRead = sonarUpdate();
  // If no edge is seen, check for the opponent:
  if(sonarRead < SONAR_THRESHOLD && sonarRead > 0){
    motors.setSpeeds(0, 0);
    delay(50);
    zumoEvent = OnFind;
    return;
  }
}

int otteTal(void){
  // Update QTR for edge detection, to fire OnEdge as fast as possible.
  if(QTRUpdate() > -1){
    return -1; // Instantly return, to not waste any time.
  }

  unsigned long long int millisRead = millis();  
  if (millisRead > otteTalsTimer && millisRead < otteTalsTimer + 1000){
    motors.setSpeeds(400 , 125);
  }
  else if (millisRead > otteTalsTimer + 1000 && millisRead < otteTalsTimer + 4000){
    motors.setSpeeds(125 , 400);
  }
  else if (millisRead > otteTalsTimer + 4000 && millisRead < otteTalsTimer + 6000){
    motors.setSpeeds(400 , 125);
  }
  else{
    otteTalsTimer = millis();
  }
  return 0;
}

