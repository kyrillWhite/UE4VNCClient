// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
enum class EEncodingType : int32 {
    Raw = 0,
    CopyRect = 1,
    Tight = 7,

    JPEGQualityLevel = -23,
    CompressionLevel = -247
};