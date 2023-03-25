// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ETightCompressionType.generated.h"

/**
 *
 */
UENUM()
enum class ETightCompressionType : uint8 {
    Basic,
    Fill,
    Jpeg,
    Invalid
};