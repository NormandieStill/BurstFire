/*
  BurstFire.h - Library for flashing Morse code.
  Released into the public domain.
*/

//#define BF_DEBUG

#ifndef BurstFire_h
#define BurstFire_h

#ifdef BF_DEBUG
#include <stdint.h>
#include <iostream>
#else
#include "Arduino.h"
#endif

class BurstFire
{
  public:
    BurstFire(uint8_t ssrPin);
    BurstFire(uint8_t ssrPin, bool invertOutput);
    void recalulateFrames(uint8_t targetPercentage);
    void zeroCross();
    void setInvertOutput(bool invertOutput);
    bool getInvertOutput();
  private:
    void calculateFramesRecursion(uint8_t targetPer, int16_t* currError, int16_t* calculatedPercentage, uint8_t recursionDepth, int16_t* burstFirePatterns, int16_t* moddedFrames);
    bool isFrameOn(uint16_t frameNum, int16_t * burstFirePatterns);
    int product(int16_t * arr, uint8_t maxIndex);

#ifdef BF_DEBUG
    void printFrames();
#endif

    /* Global variable definitions. */
    const uint8_t MAX_RECURSIONS = 5;
    const uint16_t FRAMES = 100;

    uint8_t _ssrPin;
    uint16_t _currFrame; // The value of the current frame (From 1 to FRAMES)
    uint16_t _nextFrame; // The value to be used by the next frame
    int16_t* _burstFirePatterns; // The step values used to generate the frames
    uint8_t _targetPercentage; // The current targeted percentage
    bool _outputActive; // Whether we just passed a zero cross
    bool _invertOutput; // Should we invert the output
};

#endif
