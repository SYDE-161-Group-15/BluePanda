#include <SPI.h>
#include <Servo.h>
#include <Wire.h>

#define trigPin 5
#define echoPin 6
#define servoPin 2

Servo my_servo;
long duration, distance_cm;
bool lesscoffee = false;

void setup()
{
	Serial.begin(9600);

	pinMode(trigPin, OUTPUT);
	pinMode(echoPin, INPUT);

	my_servo.attach(servoPin);// attach your servo
	my_servo.writeMicroseconds(1500);

}

void loop()
{

	if (getDistance() < 15)
		my_servo.write(0);
	else
		my_servo.write(110);

	delay(100);

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

	Serial.print(distance_cm); //print distance unit cm
	Serial.println("cm");//distance

	return distance_cm;
}
