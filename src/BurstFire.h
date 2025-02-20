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
    BurstFire(int ssrPin);
    void recalulateFrames(int targetPercentage);
    void run();
    void zeroCross();
  private:
    void calculateFramesRecursion(int targetPer, int* currError, int* calculatedPercentage, int recursionDepth, int* burstFirePatterns, int* moddedFrames);
    bool isFrameOn(int frameNum, int * burstFireValues);
    int product(int * arr, int maxIndex);
//    void _zeroCrossInt(void);
    
    /* Global variable definitions. */
    const int MAX_RECURSIONS = 5;
    const int FRAMES = 100;

//    int _zeroCrossPin;
    int _ssrPin;
    int _currFrame; // The value of the current frame (From 1 to FRAMES)
    int _nextFrame; // The value to be used by the next frame
    int* _burstFirePatterns; // The step values used to generate the frames
    int _targetPercentage; // The current targeted percentage
    bool _outputActive; // Whether we just passed a zero cross
//    static size_t instanceCount;
//    static BurstFire* instances[5]; // Currently allows up to 5 BurstFire controllers to run on one Arduino! That should be enough right? :-)
};

#endif
