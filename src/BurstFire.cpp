/*
 * burstfire.c
 * 
 * Copyright 2025 Jon Senior <jon@M91P>
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

/* Copypasta

 */

/*
 * Some vocab:
 * The default implementation uses 100 frames (where a frame is a full mains
 * cycle). 50/60Hz doesn't matter as it just affects frame duration.
 * TODO: Insert well-written explanation of how burstfire works here!
 * Limitations: Currently only works with one instance. There's no practical reason
 * why multiple SSRs couldn't be driven by one arduino.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Arduino.h"
#include "BurstFire.h"
//#include "PinChangeInterrupt.h"

BurstFire::BurstFire(int ssrPin){
//BurstFire::BurstFire(int ssrPin, int zeroCrossPin){
//  instances[instanceCount++] = this; // Add ourself to the instance list for zero cross notification.
//  _zeroCrossPin = 12; //zeroCrossPin; // For the minute this defaults to pin 12, port B
  _ssrPin = ssrPin;
  _burstFirePatterns = (int*) malloc(MAX_RECURSIONS * sizeof(int));
  _currFrame = 1;
  pinMode(_ssrPin, OUTPUT); //Setup SSR output pin 
//  pinMode(_zeroCrossPin, INPUT_PULLUP); //Setup Zerocross input pin 
//  attachPinChangeInterrupt(digitalPinToPinChangeInterrupt(_zeroCrossPin), zeroCross, RISING);
//  PCICR  |= B00000001; // ISR Pinchange port B 
//  PCMSK0 |= B00010000; // ISR enable D12 pin for pinchange interupt Mask
  recalulateFrames(0);
}

/*
 * Needs to be called in every loop().
 */
void BurstFire::run(){
//  Serial.print(_currFrame);
//  Serial.println(".");
  if (_outputActive){
    _outputActive = false;
    digitalWrite(_ssrPin, _nextFrame);
//    Serial.print(_nextFrame);
//    Serial.print(",");
    if (isFrameOn(_currFrame + 1, _burstFirePatterns)){
      _nextFrame = 1;
    }else{
      _nextFrame = 0;
    }
    _currFrame++;
    if (_currFrame > FRAMES){
      Serial.println();
      _currFrame = 1;
    }
  }
}

/*
 * Recalculate the frames that should be on in order to reach the given
 * targetPercentage
 */
void BurstFire::recalulateFrames(int targetPercentage){
  _targetPercentage = targetPercentage;
  /* Setup the variables for running the recursion */
  int* moddedFrames = (int*) malloc((MAX_RECURSIONS + 1) * sizeof(int));
  moddedFrames[0] = FRAMES;
  int recursionDepth = 0;
  int curError = _targetPercentage;
  int calculatedPercentage = 0;
  // Repeat until we hit maximum depth or the target percentage
  while (curError != 0 && recursionDepth < MAX_RECURSIONS){
    calculateFramesRecursion(_targetPercentage, &curError, &calculatedPercentage, recursionDepth, _burstFirePatterns, moddedFrames);
    recursionDepth++;
  }
  while (recursionDepth < MAX_RECURSIONS){
    _burstFirePatterns[recursionDepth] = 0;
    recursionDepth++;
  }
  free(moddedFrames);
 }


/*
 * Recursion code to calculate the next set of frame mods.
 */
void BurstFire::calculateFramesRecursion(int targetPer, int* currError, int* calculatedPercentage, int recursionDepth, int* burstFirePatterns, int* moddedFrames){
  burstFirePatterns[recursionDepth] = moddedFrames[recursionDepth] / * currError;
  moddedFrames[recursionDepth + 1] = moddedFrames[recursionDepth] / burstFirePatterns[recursionDepth];
  *calculatedPercentage += moddedFrames[recursionDepth + 1];
  *currError = targetPer - *calculatedPercentage;
  
/*  Serial.print("calculateFrames: ");
  Serial.print(targetPer);
  Serial.print(", ");
  Serial.print(recursionDepth);
  Serial.print(", ");
  Serial.print(*currError);
  Serial.print(", ");
  Serial.print(*calculatedPercentage);
  Serial.print(", ");
  Serial.print(burstFirePatterns[recursionDepth]);
  Serial.print(", ");
  Serial.print(moddedFrames[recursionDepth+1]);
  Serial.print("\n");
  */
}

/*
 * Calculates if a given frame should be on. Frame number is indexed from 1 to FRAMES inclusive.
 */
bool BurstFire::isFrameOn(int frameNum, int * burstFirePatterns){
  /*
   * Method:
   */
/* DEBUG
  if (frameNum==29){
    Serial.println("isFrameOn 29: ");
    Serial.print("0: ");
    Serial.println(burstFirePatterns[0]);
    Serial.print("1: ");
    Serial.println(burstFirePatterns[1]);
    Serial.print("2: ");
    Serial.println(burstFirePatterns[2]);
    Serial.print("3: ");
    Serial.println(burstFirePatterns[3]);
    Serial.print("4: ");
    Serial.println(burstFirePatterns[4]);
  }
  */
  int i = 0;
  while (burstFirePatterns[i] != 0 && frameNum % product(burstFirePatterns, i) == 0 && i < MAX_RECURSIONS){
    //DEBUG
    if (frameNum==29){
      Serial.println(i);
    }
    i++;
  }
  if (i % 2 == 0){
    return false;
  }else{
    return true;
  }
}

/*
 * Calculate the product of the contents of an int array starting from 0 and continuing until maxIndex.
 * THIS IS NOT SAFE AND WILL OVERFLOW IF ALLOWED.
 */
int BurstFire::product(int * arr, int maxIndex){
  long result = 1;
  for (int i=0;i <= maxIndex; i++){
    result *= arr[i];
  }
  if ((int) result != result){
    printf("Product overflow: %d, %ld\n", (int) result, result);
  }
  return (int) result;
}

/* Dirty debug routines */

void BurstFire::zeroCross(){
//  Serial.println("activated");
  _outputActive = true;
}

/*
 * Interrupt routine... hopefully.
 *
void BurstFire::_zeroCrossInt(void){
  if (digitalRead(_zeroCrossPin) == false) {  // react only when pinchange was falling edge
                                          // otherwise do nothing and return
    for (int i = 0; i <= instanceCount; i++){ // Notify all the instances.
      instances[i]->_outputActive = true;  // enable the routine for writing the output pattern
                                          // to the SSR with any falling edge
    }
  }
}
/*
ISR(PCINT0_vect)  // ISR for pin change interrupt on port B pin 12 (Zerocrossing detected )
{
}
*/
