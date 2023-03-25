// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "EPixelFormatType.generated.h"

/**
 * 
 */
UENUM()
enum class EPixelFormatType : uint8 {
    Full UMETA(DisplayName = "FULL - 32 bit"),
    High UMETA(DisplayName = "HIGH - 16 bit"),
    Low  UMETA(DisplayName = "LOW - 8 bit")
};
