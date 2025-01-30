/*
   Motor controller for medium duty motorized platform. 
	 Communicates with the Sabertooth2x12 motor controller from Dimensional 
	 Engineering 

   Based on simpleMotorControlWebPage from Telepresence robot

	 Created by Michael Shiloh

	26 Nov 2023 - ms - created, based on SabertoothMotorController
	                   hard coded for Serial3 on an Arduino Mega

  TODO
  - Handle millis overflowing (long = ? seconds)
  - All those hard coded numbers in motor control should be consts
	- Add an example to show how to initialize and control!

Instructions

   Set Motor Controller DIP switch:
	(non-lithium battery, simplified serial, single controller, 9600 baud)
   1 - on
   2 - off
   3 - on
   4 - off
   5 - on
   6 - on

Initialize the object with 

	object(timeout);

and then in setup()

	object.init();

Methods provided by this library:

	object.forward(speed); 
	object.backward(speed);
	object.left(speed);
	object.right(speed);
	object.tick();

If tick() is called, it will check the last time an instruction was
received and will stop the motors if no new commands were received within the
time set by the timeout value in milliseconds.

Alternately, motors can be stopped by setting the speed to zero.

*/

#ifndef SabertoothMotorControllerSerial3_h
#define SabertoothMotorControllerSerial3_h

#include "Arduino.h"

class SabertoothMotorControllerSerial3 
{
  public:

    SabertoothMotorControllerSerial3 (int _motorTimeOut);
		void init();
		void forward(int _speed);
		void backward(int _speed);
		void left(int _speed);
		void right(int _speed);
		void tick();

	private:

		long motorTimeOut = 90; // stop after this many milliseconds
		long motor1OnAt = -1; // -1 means not running
		long motor2OnAt = -1; //
		// for controlling the motors
		const int MYFORWARD = 0;
		const int MYREVERSE = 1;
		int forwardSpeed = 80;

		void controlMotor1(int speed, bool direction);
		void controlMotor2(int speed, bool direction);
		void stopBothMotors();
		void stopMotor1();
		void stopMotor2();
};

#endif
