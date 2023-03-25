// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include <memory>

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Common/TcpSocketBuilder.h"
#include "ESecurityType.h"
#include "ESecurityStatus.h"
#include "DES.h"
#include "VNCClient.h"
#include "UE4VNCBPLibrary.generated.h"

UCLASS()
class UUE4VNCBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "VNC Connect"), Category = "UE4VNC")
	static UVNCClient* ConnectToVNCServer(
		UVNCClient* vncClient,
		FString host,
		int port = 5900,
		FString password = "",
		bool allowJPEG = true,
		int jpegQuality = 8,
		int compression = 9,
		bool shared = false,
		EPixelFormatType pixelSize = EPixelFormatType::Full
	);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "VNC Update Client"), Category = "UE4VNC")
	static UTexture2D* UpdateClient(UVNCClient* vncClient);
};
