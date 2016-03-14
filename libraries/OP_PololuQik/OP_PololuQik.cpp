/* OP_PololuQik.cpp	Open Panzer PololuQik library - functions for communicating with Pololu's Qik dual-serial motor controllers, 
 *													such as the 2s12v10 and the 2s9v1 running the full Pololu Protocol in 7-bit mode.
 * Source: 			http://www.Pololu.com
 * Authors:    		Pololu
 *   
 * This library is a minor modification of the Pololu "qik-arduino" library.
 * For the original library, see their github page: https://github.com/pololu/qik-arduino
 *
 * All credit goes to Pololu. Their original copyright is listed below: 
 *
 * Copyright (c) 2012 Pololu Corporation.  For more information, see
 * http://www.pololu.com/
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 */ 

#include "OP_PololuQik.h"

OP_PololuQik::OP_PololuQik(byte deviceID, HardwareSerial *port) : _deviceID(deviceID), _port(port)
{
  
}

void OP_PololuQik::autobaud(boolean dontWait) const
{
  autobaud(port(), dontWait);
}

void OP_PololuQik::autobaud(HardwareSerial *thisport, boolean dontWait)
{
  if (!dontWait) { delay(1500); }
  thisport->write(QIK_INIT_COMMAND); // allow qik to autodetect baud rate
#if defined(ARDUINO) && ARDUINO >= 100
  thisport->flush();
#endif
  if (!dontWait) { delay(500); }
}

void OP_PololuQik::command(byte command, byte value) const
{
  _port->write(QIK_INIT_COMMAND);
  _port->write(deviceID());
  _port->write(command);
  _port->write(value);
}

void OP_PololuQik::throttleCommand(byte command, int speed) const
{
  speed = constrain(speed, -126, 126);
  this->command(command, (byte)abs(speed));
}

void OP_PololuQik::motor(byte motor, int speed) const
{
	if 		(motor == 1) 	throttleCommand((speed < 0 ? QIK_MOTOR_M0_REVERSE : QIK_MOTOR_M0_FORWARD), speed);
	else if (motor == 2)	throttleCommand((speed < 0 ? QIK_MOTOR_M1_REVERSE : QIK_MOTOR_M1_FORWARD), speed);
	else 	return;
}

void OP_PololuQik::allStop() const
{
  motor(1, 0);
  motor(2, 0);
}



