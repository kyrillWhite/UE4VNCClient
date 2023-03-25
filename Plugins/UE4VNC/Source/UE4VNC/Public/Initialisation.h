// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/TcpSocketBuilder.h"
#include "SPixelFormat.h"

/**
 *
 */
class UE4VNC_API Initialisation
{
public:
    Initialisation();
    ~Initialisation();

    bool ClientInit(FSocket* socket, bool shared);
    bool ServerInit(FSocket* socket, SPixelFormat& pixelFormat, uint16& framebufferWidth, uint16& framebufferHeight);
};