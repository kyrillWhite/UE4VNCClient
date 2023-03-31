// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/TcpSocketBuilder.h"
#include "EStoCMessageType.h"
#include "RawEncoding.h"
#include "CopyRectEncoding.h"
#include "TightEncoding.h"
#include "FramebufferRectangleMessage.h"
#include "SPixelFormat.h"

/**
 *
 */
class UE4VNC_API ServerClient
{
private:
    FSocket* socket;
    SPixelFormat pixelFormat;
    uint16 rectanglesLeftCount;
    TArray<std::shared_ptr<z_stream>> zStreams;

    void OpenZStreams();
    void CloseZStreams();

public:
    ServerClient();
    ~ServerClient();

    void SetSocket(FSocket* _socket);
    void SetPixelFormat(SPixelFormat _pixelFormat);
    bool GetMessage(std::shared_ptr<StoCMessage>& message);
    uint16 GetRectanglesLeftCount();
    void ReopenZStreams();

    bool FramebufferUpdate(std::shared_ptr<StoCMessage>& message);
};