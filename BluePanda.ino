#include <SPI.h>
#include <Servo.h>
#include <Wire.h>
#include <math.h> 

#define trigPin 5
#define echoPin 6
#define servoPin 2

const float coffeeConcentration = 68.9f / 100.0f; //concentration of coffee in mg/ml

float mugRadius = 1.75f;

int refreshTime = 6000;
unsigned long dynamicTimeThreshhold = refreshTime;

int coffeeThreshhold = 100;

const int maxDist = 25;
const int minDist = 2;
const int arraySize = maxDist - minDist;

int prevModeDist = 0, currentModeDist = 0;
bool firstReading;

int distData[arraySize] = { 0 };

Servo my_servo;
long duration, distance_cm;

float caffeineLevel = 0; //user caffeine level in mg
float halfLifeSeconds = 20520; //half life of caffeine in seconds (5.7hrs)


void setup()
{
	Serial.begin(9600);

	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);

	my_servo.attach(servoPin);// attach your servo
	my_servo.writeMicroseconds(1500);

	firstReading = true;

}

void loop()
{
	int dist = getDistance();
	if (dist <= maxDist && dist >= minDist) {
		distData[dist - minDist]++;
	}

	if (millis() > dynamicTimeThreshhold) {
		updateLevels();
	}

	if (caffeineLevel > coffeeThreshhold)
		my_servo.write(180);
	else
		my_servo.write(0);

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
	Serial.println(prevModeDist);
	Serial.print("Current modal dist: ");
	Serial.println(currentModeDist);

	decrCaffeineLevel(60);
	incrCaffeineLevel(prevModeDist, currentModeDist);

	Serial.print("Final caffeine lvl: ");
	Serial.println(caffeineLevel);
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
	Serial.println(caffeineLevel - caffeineLevel * pow(0.5f, timeInSeconds / halfLifeSeconds));
	caffeineLevel *= pow(0.5f, timeInSeconds / halfLifeSeconds);
}

void incrCaffeineLevel(int prevDist, int newDist) {
	if (newDist > prevDist) {
		caffeineLevel += getVolume(newDist - prevDist) * coffeeConcentration;
		Serial.print("Incr conc by: ");
		Serial.println(getVolume(newDist - prevDist) * coffeeConcentration);
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


