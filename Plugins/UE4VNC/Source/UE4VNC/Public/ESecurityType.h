// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ESecurityType.generated.h"

/**
 * 
 */
UENUM()
enum class ESecurityType : uint8 {
    Invalid = 0,
    None = 1,
    VNCAuthentication = 2
};
