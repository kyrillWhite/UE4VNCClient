// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/TcpSocketBuilder.h"
#include "ECtoSMessageType.h"
#include "EEncodingType.h"
#include "SPixelFormat.h"
#include "EPixelFormatType.h"
#include "SFrameBufferRequestMessage.h"

/**
 *
 */
class UE4VNC_API ClientServer
{
private:
    FSocket* socket;

public:
    ClientServer();
    ~ClientServer();

    void SetSocket(FSocket* _socket);
    bool FramebufferUpdateRequest(uint16 xPosition, uint16 yPosition, uint16 width, uint16 height, bool incremental = true);
    bool SetEncodings(TArray<EEncodingType> encodings);
    bool SetPixelFormatType(EPixelFormatType pixelFormatType, SPixelFormat& pixelFormat);
};