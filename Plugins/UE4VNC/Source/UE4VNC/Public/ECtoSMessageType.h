// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ECtoSMessageType.generated.h"

/**
 * 
 */
UENUM()
enum class ECtoSMessageType : uint8 {
    SetPixelFormat = 0,
    SetEncodings = 2,
    FramebufferUpdateRequest = 3
};
