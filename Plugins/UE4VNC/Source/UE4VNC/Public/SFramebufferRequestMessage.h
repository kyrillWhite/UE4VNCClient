// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct SFrameBufferRequestMessage {
    uint8  messageType;
    uint8  incremental;
    uint8  xPosition[2];
    uint8  yPosition[2];
    uint8  width[2];
    uint8  height[2];
};