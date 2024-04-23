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
#include "SRTClient.h"
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

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SRT Connect"), Category = "UE4VNC")
	static USRTClient* CreateSRTClient(
		USRTClient* srtClient,
		UStaticMeshComponent* mesh,
		FString host,
		int port = 9000,
		int buffering = 10,
		int sourceWidth = 1920,
		int sourceHeight = 1080,
		EAVCodecID codecID = EAVCodecID::AV_CODEC_ID_H264,
		EAVHWDeviceType hwDeviceType = EAVHWDeviceType::AV_HWDEVICE_TYPE_NONE
	);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SRT Start Listen"), Category = "UE4VNC")
	static void StartSRTListen(USRTClient* srtClient);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SRT Close"), Category = "UE4VNC")
	static void CloseSRTClient(USRTClient* srtClient);
};
