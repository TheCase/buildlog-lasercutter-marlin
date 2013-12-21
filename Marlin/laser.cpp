/*
  laser.cpp - Laser control library for Arduino using 16 bit timers- Version 1
  Copyright (c) 2013 Timothy Schmidt.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "laser.h"
#include "Configuration.h"
#include "pins.h"
#include <avr/interrupt.h>
#include <Arduino.h>

uint8_t iLaserPower = 0;

void setupLaser()
{
  pinMode(LASER_FIRING_PIN, OUTPUT);
  pinMode(LASER_INTENSITY_PIN, OUTPUT);
  
  digitalWrite(LASER_ACC_PIN, HIGH);  // Laser accessories are active LOW, so preset the pin
  pinMode(LASER_ACC_PIN, OUTPUT);

  digitalWrite(LASER_AOK_PIN, HIGH);  // Setup to the AOK pin to pull-up.
  pinMode(LASER_AOK_PIN, INPUT);

  analogWrite(LASER_INTENSITY_PIN, 1);  // let Arduino setup do it's thing to the PWM pin
  
  TCCR4B = 0x00;  // stop Timer4 clock for register updates
  TCCR4A = 0x82; // Clear OC4A on match, fast PWM mode, lower WGM4x=14
  ICR4 = 560; // 800 clock cycles = 20,000hz
  OCR4A = 559; // ICR4 - 1 force immediate compare on next tick
  TCCR4B = 0x18 | 0x01; // upper WGM4x = 14, clock sel = prescaler, start running
  
  noInterrupts();
  TCCR4B &= 0xf8; // stop timer, OC4A may be active now
  TCNT4 = 560; // force immediate compare on next tick
  ICR4 = 560; // set new PWM period
  TCCR4B |= 0x01; // start the timer with proper prescaler value
  interrupts();
}

uint8_t laserPower() {
	return iLaserPower;
}
void fireLaser(float intensity)
{
  if (intensity < 0.0) intensity = 0.0;
  if (intensity > 100.0) intensity = 100.0;
  
  int laser_pwm = int(intensity / 100.0 * 560.0);
  iLaserPower = int(intensity);
  waitForLaserAok();
  analogWrite(LASER_INTENSITY_PIN, laser_pwm);
  digitalWrite(LASER_FIRING_PIN, HIGH);
  SERIAL_ECHO_START;
  SERIAL_ECHO("Laser firing intensity: ");
  SERIAL_ECHO(intensity);
  SERIAL_ECHO("Laser firing PWM: ");
  SERIAL_ECHOLN(laser_pwm);
}

void offLaser()
{
  digitalWrite(LASER_FIRING_PIN,LOW);
}

void prepareLaser() {
	digitalWrite(LASER_ACC_PIN, LOW);
}

static void waitForLaserAok() {
	uint32_t timeout = millis() + LASER_AOK_TIMEOUT;
	while(digitalRead(LASER_AOK_PIN)) {
		if (millis() > timeout) {
			SERIAL_ECHO_START;
			SERIAL_ECHOLN("KILL: Relay board failed to indicate AOK");
			kill();
		}
	}
}

void shutdownLaser() {
	digitalWrite(LASER_ACC_PIN, HIGH);
}


