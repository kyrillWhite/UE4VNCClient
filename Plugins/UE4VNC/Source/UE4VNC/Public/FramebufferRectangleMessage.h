// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "Encoding.h"
#include "StoCMessage.h"

/**
 *
 */
class UE4VNC_API FramebufferRectangleMessage : public StoCMessage
{
private:
    uint16 xPosition;
    uint16 yPosition;
    uint16 width;
    uint16 height;
    std::shared_ptr<Encoding> encoding;

public:
    FramebufferRectangleMessage();
    FramebufferRectangleMessage(
        uint16 _xPosition,
        uint16 _yPosition,
        uint16 _width,
        uint16 _height,
        std::shared_ptr<Encoding> _encoding
    );
    ~FramebufferRectangleMessage();

    uint16 GetXPosition();
    uint16 GetYPosition();
    uint16 GetWidth();
    uint16 GetHeight();
    std::shared_ptr<Encoding> GetEncoding();
    void DrawToTexture(
        UTexture2D* texture,
        SPixelFormat pixelFormat,
        uint16 framebufferWidth,
        uint16 framebufferHeight
    );
    void Abstract() override;
};