// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Encoding.h"

/**
 *
 */
class UE4VNC_API RawEncoding : public Encoding
{
private:
    TArray<uint8> data;

public:
    RawEncoding();
    ~RawEncoding();

    bool ReadData(
        FSocket* socket,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        SPixelFormat pixelFormat) override;
    void DrawToTexture(
        UTexture2D* texture,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        uint16 framebufferWidth,
        uint16 framebufferHeight,
        SPixelFormat pixelFormat
    ) override;
};