// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 *
 */
struct SMouseEvent {
    uint16  x;
    uint16  y;
    char    leftPressed;
    char    middlePressed;
    char    rightPressed;
    char    wheelUp;
    char    wheelDown;
    char    wheelLeft;
    char    wheelRight;
};