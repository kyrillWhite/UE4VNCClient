// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/TcpSocketBuilder.h"
#include "DES.h"
#include "ESecurityType.h"
#include "ESecurityStatus.h"

/**
 *
 */
class UE4VNC_API Handshaking
{
private:
    FSocket* socket;
    int majorVersion;
    int minorVersion;
    ESecurityType securityType;


public:
    Handshaking();
    ~Handshaking();

    void SetSocket(FSocket* _socket);
    bool ProtocolVersion(int& _majorVersion, int& _minorVersion);
    bool Security(ESecurityType& _securityType);
    bool SecurityTypes(bool& passSecurityResult, FString password);
    bool SecurityResult();

    bool VNCAuthentication(FString password);
};