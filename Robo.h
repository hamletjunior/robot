// Only modify this file to include
// - function definitions (prototypes)
// - include files
// - extern variable definitions
// In the appropriate section

#ifndef Robo_H_
#define Robo_H_
#include "Arduino.h"
//add your includes for the project Robo here
#include "Servo.h"
#include "DistanceGP2Y0A21YK.h"
#include "Thread.h"
#include "ThreadController.h"
#include "Wire.h"
#include "Adafruit_L3GD20.h"
#include "LSM303.h"
#include "Queue/Queue.h"
//end of add your includes here

struct AccelerometerReading {
	int x, y, z;
};

#ifdef __cplusplus
extern "C" {
#endif
void loop();
void goForward();
void distanceRead();
AccelerometerReading readAccelerometer();
//void desaceleraMotor();
//void aceleraMotor();
void goBackward();
void stop();
void distanceScan();
//void scanVertical();
void motorSpinRight();
void motorSpinLeft();
void leftSpin();
void rightSpin();
void spinControl();
void lockPreventing();
void crazyBotMode();
void setup(void);
#ifdef __cplusplus
} // extern "C"
#endif

//add your function definitions for the project Robo here



//Do not add code below this line
#endif /* Robo_H_ */
