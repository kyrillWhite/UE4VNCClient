// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Encoding.h"

/**
 *
 */
class UE4VNC_API CopyRectEncoding : public Encoding
{
private:
    uint16 srcXPosition;
    uint16 srcYPosition;

public:
    CopyRectEncoding();
    ~CopyRectEncoding();

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