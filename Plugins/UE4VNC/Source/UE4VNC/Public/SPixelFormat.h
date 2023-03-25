// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct SPixelFormat {
    uint8  bitsPerPixel;
    uint8  depth;
    uint8  bigEndianFlag;
    uint8  trueColourFlag;
    uint8  redMax[2];
    uint8  greenMax[2];
    uint8  blueMax[2];
    uint8  redShift;
    uint8  greenShift;
    uint8  blueShift;
    uint8  padding[3];

    bool IsTPIXEL() {
        if (bitsPerPixel != 32) {
            return false;
        }
        if (depth != 24) {
            return false;
        }
        if (redMax[1] != 255) {
            return false;
        }
        if (greenMax[1] != 255) {
            return false;
        }
        if (blueMax[1] != 255) {
            return false;
        }
        if ((redShift & 7) != 0) {
            return false;
        }
        if ((greenShift & 7) != 0) {
            return false;
        }
        if ((blueShift & 7) != 0) {
            return false;
        }

        return true;
    }
};