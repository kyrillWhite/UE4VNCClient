// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EStoCMessageType.h"

/**
 *
 */
class UE4VNC_API StoCMessage
{
private:
    EStoCMessageType messageType;

public:
    StoCMessage();
    virtual ~StoCMessage();

    StoCMessage(EStoCMessageType _messageType);
    EStoCMessageType GetMessageType();
    virtual void Abstract() = 0;
};