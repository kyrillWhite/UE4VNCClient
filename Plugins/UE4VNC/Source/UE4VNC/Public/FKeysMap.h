// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <iostream>
#include <map>
#include "CoreMinimal.h"

/**
 *
 */
static class UE4VNC_API FKeysMap
{
public:
    static std::map<FString, uint32> FKeyToASCII;
    static std::map<FString, uint32> FKeyToShiftedASCII;
};