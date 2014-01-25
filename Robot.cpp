// Do not remove the include below
#include "Robot.h"

#define MOTOR_MAX_POWER 255
#define MOTOR_1 3
#define PIN_1_MOTOR_1 2
#define PIN_2_MOTOR_1 4
#define MOTOR_2 5
#define PIN_1_MOTOR_2 8
#define PIN_2_MOTOR_2 7
#define SERVO_TILTING_PIN 6
#define SERVO_PAN_PIN 9
#define AMOUNT_READINGS 53
#define MIDDLE_ANGLE AMOUNT_READINGS * 3 / 2

Servo servoTilting, servoPan;
DistanceGP2Y0A21YK distanceSensor;
Adafruit_L3GD20 gyro;
LSM303 accelCompass;

byte distances[AMOUNT_READINGS + 1];
int angleToSpin = 0;
byte motor1Power = MOTOR_MAX_POWER;
byte motor2Power = MOTOR_MAX_POWER;

ThreadController threadController = ThreadController();
Thread threadGoForward = Thread();
Thread threadGoBackward = Thread();
Thread threadDistanceReading = Thread();
Thread threadDistanceScan = Thread();
Thread threadLeftSpin = Thread();
Thread threadRightSpin = Thread();
Thread threadSpinControl = Thread();

ThreadController lockPreventingController = ThreadController();
Thread threadLockPreventing = Thread();
Thread threadCrazyBotMode = Thread();

Queue *accelerationsQueue = new Queue();

void setup() {
	Serial.begin(9600);
	pinMode(MOTOR_1, OUTPUT);
	pinMode(PIN_1_MOTOR_1, OUTPUT);
	pinMode(PIN_2_MOTOR_1, OUTPUT);
	pinMode(MOTOR_2, OUTPUT);
	pinMode(PIN_1_MOTOR_2, OUTPUT);
	pinMode(PIN_2_MOTOR_2, OUTPUT);

	servoTilting.attach(SERVO_TILTING_PIN);
	servoPan.attach(SERVO_PAN_PIN);
	servoTilting.write(80);
	servoPan.write(75);

	distanceSensor.begin(A0);

	Wire.begin();
	if (!gyro.begin(gyro.L3DS20_RANGE_2000DPS)) { // Error initializing the gyroscope
		while (1)
			;
	}

	accelCompass.init();
	accelCompass.enableDefault();
	accelCompass.writeAccReg(accelCompass.CTRL_REG4_A, 0x30); // Makes the accelerometer works between -8G e +8G

	threadGoForward.onRun(goForward);
	threadGoForward.setInterval(10);

	threadDistanceReading.onRun(distanceRead);
	threadDistanceReading.setInterval(10);

	threadGoBackward.onRun(goBackward);
	threadGoBackward.setInterval(10);
	threadGoBackward.enabled = false;

	threadDistanceScan.onRun(distanceScan);
	threadDistanceScan.setInterval(0);
	threadDistanceScan.enabled = false;

	threadLeftSpin.onRun(leftSpin);
	threadLeftSpin.setInterval(0);
	threadLeftSpin.enabled = false;

	threadRightSpin.onRun(rightSpin);
	threadRightSpin.setInterval(0);
	threadRightSpin.enabled = false;

	threadSpinControl.onRun(spinControl);
	threadSpinControl.setInterval(10);
	threadSpinControl.enabled = false;

	threadLockPreventing.onRun(lockPreventing);
	threadLockPreventing.setInterval(100);

	threadCrazyBotMode.onRun(crazyBotMode);
	threadCrazyBotMode.setInterval(10);
	threadCrazyBotMode.enabled = false;

	threadController.add(&threadGoForward);
	threadController.add(&threadGoBackward);
	threadController.add(&threadDistanceScan);
	threadController.add(&threadDistanceReading);
	threadController.add(&threadRightSpin);
	threadController.add(&threadLeftSpin);
	threadController.add(&threadSpinControl);

	lockPreventingController.add(&threadLockPreventing);
	lockPreventingController.add(&threadCrazyBotMode);
}

void loop() {
	threadController.run();
	lockPreventingController.run();
}

void lockPreventing() {
	AccelerometerReading accelerometerReading = readAccelerometer();
	accelerationsQueue->addValue(accelerometerReading.y);
	if (accelerationsQueue->size() < MAX_QUEUE_SIZE) {
		return;
	}

	int maximum = accelerationsQueue->maxVal();
	int minimum = accelerationsQueue->minVal();

	int diference = abs(maximum - minimum);
	if (diference < 80) { // If the diference between the max and min 15 accelerations is less than 80 micro Gs, than it is stuck somewhere
		threadController.enabled = false;
		threadCrazyBotMode.enabled = true;
	} else {
		threadController.enabled = true;
		threadCrazyBotMode.enabled = false;
	}

}

void crazyBotMode() {
	motorSpinRight();
	delay(1000);
	motorSpinLeft();
	delay(1000);
}

void goForward() {
	analogWrite(MOTOR_1, motor1Power);
	analogWrite(MOTOR_2, motor2Power);

	digitalWrite(PIN_1_MOTOR_1, LOW);
	digitalWrite(PIN_2_MOTOR_1, HIGH);
	digitalWrite(PIN_1_MOTOR_2, LOW);
	digitalWrite(PIN_2_MOTOR_2, HIGH);
}

AccelerometerReading readAccelerometer() {
	AccelerometerReading reading;
	accelCompass.read();
	// According to the accelerometer datasheet, that's the mathematical Hocus Pocus to transform the raw reading in micro Gs.
	reading.x = (int) ((accelCompass.a.x >> 4) * 3.9);
	reading.y = (int) ((accelCompass.a.y >> 4) * 3.9);
	reading.z = (int) ((accelCompass.a.z >> 4) * 3.9);
	return reading;
}

void distanceRead() {
	int distance = distanceSensor.getDistanceCentimeter();
	if (distance > 10) {
		if (distance < 30) {
			byte balancedMotorPower = map(distance, 0, 30, 100,
			MOTOR_MAX_POWER);
			motor1Power = balancedMotorPower;
			motor2Power = balancedMotorPower;
		} else {
			motor1Power = MOTOR_MAX_POWER;
			motor2Power = MOTOR_MAX_POWER;
		}
	} else {
		threadGoForward.enabled = false;
		threadDistanceReading.enabled = false;
		threadDistanceScan.enabled = true;
		motor1Power = MOTOR_MAX_POWER;
		motor2Power = MOTOR_MAX_POWER;
	}
}

void stop() {
	analogWrite(MOTOR_MAX_POWER, motor1Power);
	analogWrite(MOTOR_MAX_POWER, motor2Power);

	digitalWrite(PIN_1_MOTOR_1, LOW);
	digitalWrite(PIN_2_MOTOR_1, LOW);
	digitalWrite(PIN_1_MOTOR_2, LOW);
	digitalWrite(PIN_2_MOTOR_2, LOW);
}

void distanceScan() {
	static int lastScanExecution = 0;
	static boolean goingBackwards = false;
	lockPreventingController.enabled = false;
	stop();
	delay(300);
	goBackward();
	delay(300);
	stop();

	int lastInstant = millis();
	float timeElapsedLastScan = lastInstant - lastScanExecution;
	if (timeElapsedLastScan < 2000 && !goingBackwards) { // stops the scan, go backwards and re-scan if the last scan was in less than 2 seconds
		goingBackwards = true;
		goBackward();
		delay(1000);
		stop();
		goingBackwards = false;
	}

	for (int i = 0; i <= AMOUNT_READINGS; i++) {
		servoPan.write(i * 3); // Since the max angle was divided by 3, we multiply i by 3
		distances[i] = distanceSensor.getDistanceCentimeter();
		delay(15);
	}
	servoPan.write(80);
	threadDistanceScan.enabled = false;
	int greaterDistanceAngle = 0;
	int greaterDistance = 0;

	// Two fors for equal measuring
	for (int i = AMOUNT_READINGS / 2; i <= AMOUNT_READINGS; i++) {
		if (distances[i] > greaterDistance) {
			greaterDistance = distances[i];
			greaterDistanceAngle = i * 3;
		}
	}
	for (int i = AMOUNT_READINGS / 2; i >= 0; i--) {
		if (distances[i] > greaterDistance) {
			greaterDistance = distances[i];
			greaterDistanceAngle = i * 3;
		}
	}

	if (greaterDistanceAngle > MIDDLE_ANGLE) { // LEFT
		angleToSpin = greaterDistanceAngle - MIDDLE_ANGLE;
		threadLeftSpin.enabled = true;
	} else { // RIGHT
		angleToSpin = MIDDLE_ANGLE - greaterDistanceAngle;
		threadRightSpin.enabled = true;
	}
	lockPreventingController.enabled = true;
	lastScanExecution = millis();
}

void goBackward() {
	analogWrite(MOTOR_1, motor1Power);
	analogWrite(MOTOR_2, motor2Power);

	digitalWrite(PIN_1_MOTOR_1, HIGH);
	digitalWrite(PIN_2_MOTOR_1, LOW);
	digitalWrite(PIN_1_MOTOR_2, HIGH);
	digitalWrite(PIN_2_MOTOR_2, LOW);
}

void motorSpinLeft() {
	analogWrite(MOTOR_1, motor1Power);
	analogWrite(MOTOR_2, motor2Power);
	digitalWrite(PIN_1_MOTOR_1, HIGH);
	digitalWrite(PIN_2_MOTOR_1, LOW);
	digitalWrite(PIN_1_MOTOR_2, LOW);
	digitalWrite(PIN_2_MOTOR_2, HIGH);
}

void motorSpinRight() {
	analogWrite(MOTOR_1, motor1Power);
	analogWrite(MOTOR_2, motor2Power);
	digitalWrite(PIN_1_MOTOR_1, LOW);
	digitalWrite(PIN_2_MOTOR_1, HIGH);
	digitalWrite(PIN_1_MOTOR_2, HIGH);
	digitalWrite(PIN_2_MOTOR_2, LOW);
}

void leftSpin() {
	motorSpinLeft();
	threadLeftSpin.enabled = false;
	threadSpinControl.enabled = true;
}

void rightSpin() {
	motorSpinRight();
	threadRightSpin.enabled = false;
	threadSpinControl.enabled = true;
}

void spinControl() {
	static boolean dirtySpinExecution = true;
	static int lastGyroReadingInstant = 0;
	int lastInstant = millis();
	float elapsedTimeSinceLastReading = lastInstant - lastGyroReadingInstant;

	if (!dirtySpinExecution) {
		gyro.read();
		lastGyroReadingInstant = millis();
		float z = gyro.data.z;
		Serial.print("Lido: ");
		Serial.print(z);
		int grausGirados = (int) (abs(z * elapsedTimeSinceLastReading / 1000));
		Serial.print("Graus Girados: ");
		Serial.print(grausGirados);
		angleToSpin -= grausGirados;
		Serial.print("Angulo a girar: ");
		Serial.println(angleToSpin);
		if (angleToSpin <= 0) {
			dirtySpinExecution = true;
			threadSpinControl.enabled = false;
			stop();
			delay(500);
			threadGoForward.enabled = true;
			threadDistanceReading.enabled = true;
		}
	} else {
		dirtySpinExecution = false;
		lastGyroReadingInstant = millis();
	}
}

//void desaceleraMotor() {
//	for (int i = POTENCIA_MAXIMA_MOTOR; i >= 0; i--) {
//		analogWrite(controladorMotor1, i);
//		analogWrite(controladorMotor2, i);
//		delay(20);
//	}
//}

//void aceleraMotor() {
//	for (int i = 0; i <= POTENCIA_MAXIMA_MOTOR; i++) {
//		analogWrite(controladorMotor1, i);
//		analogWrite(controladorMotor2, i);
//		delay(20);
//	}
//}

//void scanVertical() {
//	for (int i = 76; i <= 110; i++) {
//		servoInclinacao.write(i);
//		delay(3);
//	}
//	for (int i = 110; i >= 15; i--) {
//		servoInclinacao.write(i);
//		delay(3);
//	}
//	for (int i = 15; i <= 75; i++) {
//		servoInclinacao.write(i);
//		delay(3);
//	}
//}

//void testeAcelerometroGiros() {
//	analogWrite(controladorMotor1, potenciaMotor1);
//	analogWrite(controladorMotor2, potenciaMotor2);
//	digitalWrite(pino1Motor1, HIGH);
//	digitalWrite(pino2Motor1, LOW);
//	digitalWrite(pino1Motor2, LOW);
//	digitalWrite(pino2Motor2, HIGH);
//	Serial.println("*********************");
//	Serial.println("Virando para esquerda");
//	Serial.println("*********************");
//	for (int i = 0; i < 50; i++) {
//		LeituraAcelerometro leituraAcelerometro = lerAcelerometro();
//		Serial.print("x;");
//		Serial.print(leituraAcelerometro.x);
//		Serial.print(";y;");
//		Serial.print(leituraAcelerometro.y);
//		Serial.print(";z;");
//		Serial.println(leituraAcelerometro.z);
//		delay(1);
//	}
//	freiar();
//	delay(500);
//	Serial.println("********************");
//	Serial.println("Virando para direita");
//	Serial.println("********************");
//	analogWrite(controladorMotor1, potenciaMotor1);
//	analogWrite(controladorMotor2, potenciaMotor2);
//	digitalWrite(pino1Motor1, LOW);
//	digitalWrite(pino2Motor1, HIGH);
//	digitalWrite(pino1Motor2, HIGH);
//	digitalWrite(pino2Motor2, LOW);
//	for (int i = 0; i < 50; i++) {
//		LeituraAcelerometro leituraAcelerometro = lerAcelerometro();
//		Serial.print("x;");
//		Serial.print(leituraAcelerometro.x);
//		Serial.print(";y;");
//		Serial.print(leituraAcelerometro.y);
//		Serial.print(";z;");
//		Serial.println(leituraAcelerometro.z);
//		delay(1);
//	}
//	freiar();
//	delay(500);
//}
//
//void testeAcelerometroBatidaFrente() {
//	Serial.println("**********************");
//	Serial.println("Teste batida de frente");
//	Serial.println("**********************");
//	analogWrite(controladorMotor1, potenciaMotor1);
//	analogWrite(controladorMotor2, potenciaMotor2);
//	digitalWrite(pino1Motor1, LOW);
//	digitalWrite(pino2Motor1, HIGH);
//	digitalWrite(pino1Motor2, LOW);
//	digitalWrite(pino2Motor2, HIGH);
//	for (int i = 0; i < 500; i++) {
//		LeituraAcelerometro leituraAcelerometro = lerAcelerometro();
//		Serial.print("x;");
//		Serial.print(leituraAcelerometro.x);
//		Serial.print(";y;");
//		Serial.print(leituraAcelerometro.y);
//		Serial.print(";z;");
//		Serial.println(leituraAcelerometro.z);
//		delay(1);
//	}
//	freiar();
//	delay(500);
//}
