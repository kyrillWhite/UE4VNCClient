// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class UE4VNC_API DES
{
private:
	TArray<uint8> key;
	TArray<TBitArray<>> keys;

	void GenerateKeys();
	TBitArray<> TArray8ToTBitArray(TArray<uint8> tArray8);

public:
	DES();
	DES(TArray<uint8> key);
	~DES();

	TArray<uint8> Encrypt(TArray<uint8> data);
};