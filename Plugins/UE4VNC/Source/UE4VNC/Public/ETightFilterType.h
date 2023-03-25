// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ETightFilterType.generated.h"

/**
 *
 */
UENUM()
enum class ETightFilterType : uint8 {
    CopyFilter = 0,
    PaletteFilter = 1,
    GradientFilter = 2
};