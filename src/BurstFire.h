/*
  BurstFire.h - Library for flashing Morse code.
  Released into the public domain.
*/
#ifndef BurstFire_h
#define BurstFire_h

#include "Arduino.h"

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
    void calculateFramesRecursion(uint8_t targetPer, uint8_t* currError, uint8_t* calculatedPercentage, uint8_t recursionDepth, uint8_t* burstFirePatterns, uint16_t* moddedFrames);
    bool isFrameOn(uint16_t frameNum, uint8_t * burstFirePatterns);
    int product(uint8_t * arr, uint8_t maxIndex);
    
    /* Global variable definitions. */
    const uint8_t MAX_RECURSIONS = 5;
    const uint16_t FRAMES = 100;

    uint8_t _ssrPin;
    uint16_t _currFrame; // The value of the current frame (From 1 to FRAMES)
    uint16_t _nextFrame; // The value to be used by the next frame
    uint8_t* _burstFirePatterns; // The step values used to generate the frames
    uint8_t _targetPercentage; // The current targeted percentage
    bool _outputActive; // Whether we just passed a zero cross
    bool _invertOutput; // Should we invert the output
};

#endif
