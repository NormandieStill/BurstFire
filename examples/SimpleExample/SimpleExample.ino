/*
 * SimpleExample.ino
 * 
 * Copyright 2025 NormandieStill <normandiestill@gmail.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
#include <BurstFire.h>

// Pin Settings
const byte _potPin = A1;
const byte _zeroCrossPin = 2;
const byte _ssrPin = 3;

// Constants 

const uint16_t _potReadInterval = 500; // update the pot twice per second

// Globals

volatile bool zeroCrossTrigger = false;
BurstFire bf(_ssrPin, true);
uint8_t targetPer;
long zeroCrossTime;
long lastZeroCrossTime;
long potReadTime;

void setup() {

  Serial.begin(115200);
  // Setup zero cross interrupt and ssrPin

  pinMode(_potPin, INPUT);
  pinMode(_zeroCrossPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(_zeroCrossPin), _zeroCrossISR, FALLING);

  readTargetPercentage();
  bf.recalulateFrames(targetPer);
  zeroCrossTime = millis();
}

/*
 * Main loop
 */
void loop() {
// First handle the zeroCrossing
  if (zeroCrossTrigger){
    bf.zeroCross();
    long newTime = millis();
    zeroCrossTime = newTime - lastZeroCrossTime;
    lastZeroCrossTime = newTime;
    zeroCrossTrigger = false;
  }
  // Reread the value of the potentiometer.
  readTargetPercentage();
}

/*
 * Read a new percentage from the pot on _potPin
 */
void readTargetPercentage(){
  if (abs(millis() - potReadTime) > _potReadInterval){
    long potVal = analogRead(_potPin);
    uint8_t newTarget = (uint8_t) map(potVal, 0, 1023, 0, 100);
    if (newTarget != targetPer){
      targetPer = newTarget;
      bf.recalulateFrames(targetPer);
    }
    potReadTime = millis();
  }
}

/*
 * Interrupt service routine for zero cross detection.
 */
void _zeroCrossISR(){
  zeroCrossTrigger = true;
}
