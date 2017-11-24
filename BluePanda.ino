#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <Wire.h>
#include <math.h> 

#define trigPin 5
#define echoPin 6
#define servoPin 2
#define backlightPin 13
#define btPinRX 11
#define btPinTX 10

LiquidCrystal_I2C lcd(0x20, 16, 2);
SoftwareSerial BTSerial(btPinRX, btPinTX);

const float coffeeConcentration = 68.9f / 100.0f; //concentration of coffee in mg/ml

float mugRadius = 1.75f; //radius of mug in cm

int refreshTime = 3000;
unsigned long dynamicTimeThreshhold = refreshTime;

int coffeeThreshhold = 100;

const int maxDist = 35;
const int minDist = 2;
const int arraySize = maxDist - minDist + 1;

int prevModeDist = 0, currentModeDist = 0;
bool firstReading, isLocked = false;

int distData[arraySize] = { 0 };

Servo my_servo;
long duration, distance_cm;

float caffeineLevel = 0; //user caffeine level in mg
float halfLifeSeconds = 205; //half life of caffeine in seconds (5.7hrs), 20520 seconds

String hello = "hello world";


void setup()
{
	Serial.begin(9600);
	BTSerial.begin(9600);

	pinMode(backlightPin, OUTPUT);
	digitalWrite(backlightPin, HIGH);

	lcd.init();
	lcd.backlight();

	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);

	my_servo.attach(servoPin);// attach your servo
	my_servo.writeMicroseconds(1500);

	my_servo.write(0);
	firstReading = true;

	lcd.print("Intake: ");
	lcd.print(caffeineLevel);
	lcd.print("mg");

	BTSerial.write("hello");

	lcd.setCursor(0, 1);
	if (isLocked)
		lcd.print("LOCKED");
	else lcd.print("UNLOCKED");
}

void loop()
{
	int dist = getDistance();
	if (dist <= maxDist && dist >= minDist) {
		distData[dist - minDist]++;
	}

	if (millis() > dynamicTimeThreshhold) {
		updateLevels();
		if (caffeineLevel > 100) {
			my_servo.write(90);
			isLocked = true;
		}
		else {
			my_servo.write(0);
			isLocked = false;
		}
		printLCD(caffeineLevel, isLocked);
	}
	delay(100);
}

void updateLevels() {
	Serial.print(refreshTime / 1000);
	Serial.println(" seconds has passed");
	dynamicTimeThreshhold = dynamicTimeThreshhold + refreshTime;

	if (firstReading) {
		currentModeDist = getMode();
		prevModeDist = currentModeDist;
		firstReading = false;
	}
	else {
		prevModeDist = currentModeDist;
		currentModeDist = getMode();
	}

	Serial.print("Previous modal dist: ");
	Serial.print(prevModeDist);
	Serial.println("cm");
	Serial.print("Current modal dist: ");
	Serial.print(currentModeDist);
	Serial.println("cm");

	decrCaffeineLevel(60);
	incrCaffeineLevel(prevModeDist, currentModeDist);

	Serial.print("Final caffeine lvl: ");
	Serial.print(caffeineLevel);
	Serial.println("mg");
	Serial.println();

	clearData();
}

long microsecondsToCentimeters(long ms) {
	return ms / 29 / 2;
}

long getDistance() {

	digitalWrite(trigPin, LOW);
	delayMicroseconds(2);
	digitalWrite(trigPin, HIGH);
	delayMicroseconds(15);
	digitalWrite(trigPin, LOW);

	duration = pulseIn(echoPin, HIGH);

	distance_cm = microsecondsToCentimeters(duration);

	return distance_cm;
}

void decrCaffeineLevel(float timeInSeconds) {
	Serial.print("Decr conc. by: ");
	Serial.print(caffeineLevel - caffeineLevel * pow(0.5f, timeInSeconds / halfLifeSeconds));
	Serial.println("mg");
	caffeineLevel *= pow(0.5f, timeInSeconds / halfLifeSeconds);
}

void incrCaffeineLevel(int prevDist, int newDist) {
	if (newDist > prevDist) {
		caffeineLevel += getVolume(newDist - prevDist) * coffeeConcentration;
		Serial.print("Incr conc by: ");
		Serial.print(getVolume(newDist - prevDist) * coffeeConcentration);
		Serial.println("mg");
	}
}

void clearData() {
	for (int i = 0; i < arraySize; i++)
		distData[i] = 0;
}

int getMode() {
	int mode, largestValue = 0;
	for (int i = 0; i < arraySize; i++) {
		if (distData[i] > largestValue) {
			largestValue = distData[i];
			mode = i + minDist;
		}
	}
	return mode;
}

float getVolume(int distance) {
	return PI * pow(mugRadius, 2.0f) * distance;
}

void printLCD(float intakeLevel, bool isLocked) {
	lcd.clear();
	lcd.print("Intake: ");
	lcd.print(intakeLevel);
	lcd.print("mg");

	lcd.setCursor(0, 1);
	if (isLocked)
		lcd.print("LOCKED");
	else lcd.print("UNLOCKED");
}