/*
 * burstfire.c
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

BurstFire::BurstFire(uint8_t ssrPin){
  _ssrPin = ssrPin;
  _burstFirePatterns = (uint8_t*) malloc(MAX_RECURSIONS * sizeof(uint8_t));
  _currFrame = 1;
  pinMode(_ssrPin, OUTPUT); //Setup SSR output pin 
  recalulateFrames(0);
}

BurstFire::BurstFire(uint8_t ssrPin, bool invertOutput) : BurstFire(ssrPin) {
  _invertOutput = invertOutput;
}

/*
 * Set the flag for inverting output
 */
void BurstFire::setInvertOutput(bool invertOutput){
  _invertOutput = invertOutput;
}

/*
 * Get the flag for inverting output
 */
bool BurstFire::getInvertOutput(){
  return _invertOutput;
}


/*
 * Recalculate the frames that should be on in order to reach the given
 * targetPercentage
 */
void BurstFire::recalulateFrames(uint8_t targetPercentage){
  _targetPercentage = targetPercentage;
  /* Setup the variables for running the recursion */
  uint16_t* moddedFrames = (uint16_t*) malloc((MAX_RECURSIONS + 1) * sizeof(uint16_t));
  moddedFrames[0] = FRAMES;
  uint8_t recursionDepth = 0;
  uint8_t curError = _targetPercentage;
  uint8_t calculatedPercentage = 0;
  // Repeat until we hit maximum depth or the target percentage
  while (curError != 0 && recursionDepth < MAX_RECURSIONS){
    calculateFramesRecursion(_targetPercentage, &curError, &calculatedPercentage, recursionDepth, _burstFirePatterns, moddedFrames);
    recursionDepth++;
  }
  // Populate the rest of the array with 0s to prevent overread.
  while (recursionDepth < MAX_RECURSIONS){
    _burstFirePatterns[recursionDepth] = 0;
    recursionDepth++;
  }
  free(moddedFrames);
 }


/*
 * Recursion code to calculate the next set of frame mods.
 */
void BurstFire::calculateFramesRecursion(uint8_t targetPer, uint8_t* currError, uint8_t* calculatedPercentage, uint8_t recursionDepth, uint8_t* burstFirePatterns, uint16_t* moddedFrames){
  burstFirePatterns[recursionDepth] = moddedFrames[recursionDepth] / * currError;
  moddedFrames[recursionDepth + 1] = moddedFrames[recursionDepth] / burstFirePatterns[recursionDepth];
  *calculatedPercentage += moddedFrames[recursionDepth + 1];
  *currError = targetPer - *calculatedPercentage;
}

/*
 * Calculates if a given frame should be on. Frame number is indexed from 1 to FRAMES inclusive.
 */
bool BurstFire::isFrameOn(uint16_t frameNum, uint8_t * burstFirePatterns){
  /*
   * Method:
   */
  uint8_t i = 0;
  while (burstFirePatterns[i] != 0 && frameNum % product(burstFirePatterns, i) == 0 && i < MAX_RECURSIONS){
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
int BurstFire::product(uint8_t * arr, uint8_t maxIndex){
  long result = 1;
  for (int i=0;i <= maxIndex; i++){
    result *= arr[i];
  }
  if ((int) result != result){
    //printf("Product overflow: %d, %ld\n", (int) result, result);
  }
  return (int) result;
}

/*
 * Zero cross event occurred so update the output pin
 */
void BurstFire::zeroCross(){
    digitalWrite(_ssrPin, _nextFrame);
    if (isFrameOn(_currFrame + 1, _burstFirePatterns)){
      if (!_invertOutput){
        _nextFrame = 1;
      }else{
        _nextFrame = 0;
      }
    }else{
      if (!_invertOutput){
        _nextFrame = 0;
      }else{
        _nextFrame = 1;
      }
    }
    _currFrame++;
    if (_currFrame > FRAMES){
      _currFrame = 1;
    }
}
