// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE4VNCBPLibrary.h"
#include "UE4VNC.h"

UUE4VNCBPLibrary::UUE4VNCBPLibrary(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UVNCClient* UUE4VNCBPLibrary::ConnectToVNCServer(
    UVNCClient* vncClient,
    UStaticMeshComponent* mesh,
    APlayerController* playerController,
    UWidgetComponent* keyboardWidgetComponent,
    UWidgetInteractionComponent* widgetInteractionComponent,
    bool onlyControl,
    FString host,
    int port,
    FString password,
    bool allowJPEG,
    int jpegQuality,
    int compression,
    bool shared,
    EPixelFormatType pixelSize
)
{
    vncClient->SetSettings(onlyControl, host, port, password, allowJPEG, jpegQuality, compression, shared, pixelSize, mesh, playerController, keyboardWidgetComponent, widgetInteractionComponent);
    vncClient->FullConnect();
    return vncClient;
}

UTexture2D* UUE4VNCBPLibrary::UpdateClient(UVNCClient* vncClient)
{
    if (vncClient == nullptr || !vncClient->IsConnected() || !vncClient->IsInitComplete()) {
        if (vncClient != nullptr && !vncClient->IsConnecting() && !vncClient->IsConnected()) {
            vncClient->FullConnect();
        }
        return nullptr;
    }
    if (!vncClient->ClientServerMessages()) {
        vncClient->FullConnect();
        return nullptr;
    }
    if (!vncClient->ServerClientMessages()) {
        vncClient->FullConnect();
        return nullptr;
    }
    UTexture2D* texture = vncClient->GetTexture();
    #undef UpdateResource
    texture->UpdateResource();
    return texture;
}

void UUE4VNCBPLibrary::HandleAnyKeyPress(UVNCClient* vncClient, FKey key, bool isShiftHolding)
{
    if (vncClient) {
        vncClient->HandleInputEvent(key, true, isShiftHolding);
    }
}

void UUE4VNCBPLibrary::HandleAnyKeyRelease(UVNCClient* vncClient, FKey key)
{
    if (vncClient) {
        vncClient->HandleInputEvent(key, false);
    }
}

USRTClient* UUE4VNCBPLibrary::CreateSRTClient(
    USRTClient* srtClient,
    UStaticMeshComponent* mesh,
    FString host,
    int port,
    int buffering,
    int sourceWidth,
    int sourceHeight,
    EAVCodecID codecID,
    EAVHWDeviceType hwDeviceType
)
{
    if (!srtClient->Initialise(mesh, host, port, buffering, sourceWidth, sourceHeight, codecID, EAVPixelFormat::AV_PIX_FMT_YUV420P, hwDeviceType)) {
        return nullptr;
    }
    return srtClient;
}

void UUE4VNCBPLibrary::StartSRTListen(USRTClient* srtClient)
{
    if (srtClient) {
        auto future = Async(EAsyncExecution::Thread, [&]() {
            srtClient->Listen();
        });
    }
}

void UUE4VNCBPLibrary::CloseSRTClient(USRTClient* srtClient)
{
    if (srtClient) {
        srtClient->Close();
    }
}
