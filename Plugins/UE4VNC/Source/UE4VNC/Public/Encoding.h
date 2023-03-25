// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EEncodingType.h"
#include "SPixelFormat.h"

/**
 *
 */
class UE4VNC_API Encoding
{
private:
    EEncodingType encodingType;

public:
    Encoding();
    Encoding(EEncodingType _encodingType);
    virtual ~Encoding();

    EEncodingType GetEncodingType();
    virtual bool ReadData(
        FSocket* socket,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        SPixelFormat pixelFormat) = 0;
    virtual void DrawToTexture(
        UTexture2D* texture,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        uint16 framebufferWidth,
        uint16 framebufferHeight,
        SPixelFormat pixelFormat
    ) = 0;
};