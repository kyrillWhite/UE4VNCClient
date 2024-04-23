// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SMouseEvent.h"
#include "SKeyEvent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/PlayerInput.h"
#include "Components/WidgetComponent.h"
#include "Components/WidgetInteractionComponent.h"

/**
 *
 */
class UE4VNC_API Control
{
private:
    APlayerController* playerController;
    UStaticMeshComponent* mesh;
    bool inputEnabled;
    UWidgetComponent* keyboardWidgetComponent;
    UWidgetInteractionComponent* widgetInteractionComponent;
    FVector lastCursorPosition;
    SMouseEvent mouseState;

    FVector GetCameraLookProjectionOnPlane(bool& isOutOfBounds);
    FVector GetPositionFromWidgetInteractoin(bool& isOutOfBounds, bool& isWidget);

public:
    Control();
    ~Control();

    bool IsInputEnabled(FKey key);
    bool HandleMouseEvent(FKey key, bool isPressed, SMouseEvent& _event, uint16 width, uint16 height);
    bool HandleKeyboardEvent(FKey key, bool isPressed, SKeyEvent& _event, bool isShiftHolding);

    void SetSettings(
        APlayerController* _playerController,
        UStaticMeshComponent* _mesh,
        UWidgetComponent* _keyboardWidgetComponent,
        UWidgetInteractionComponent* _widgetInteractionComponent
    );
};