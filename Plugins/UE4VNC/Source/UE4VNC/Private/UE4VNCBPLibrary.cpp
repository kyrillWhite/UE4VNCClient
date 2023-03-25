// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE4VNCBPLibrary.h"
#include "UE4VNC.h"

UUE4VNCBPLibrary::UUE4VNCBPLibrary(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

UVNCClient* UUE4VNCBPLibrary::ConnectToVNCServer(
    UVNCClient* vncClient,
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
    if (!vncClient->Connect(host, port)) {
        UE_LOG(LogTemp, Error, TEXT("Connect error"));
        return nullptr;
    }
    if (!vncClient->Handshake(password)) {
        UE_LOG(LogTemp, Error, TEXT("Handshake error"));
        return nullptr;
    }
    if (!vncClient->Initialise(allowJPEG, jpegQuality, compression, shared, pixelSize)) {
        UE_LOG(LogTemp, Error, TEXT("Initialise error"));
        return nullptr;
    }

    return vncClient;
}

UTexture2D* UUE4VNCBPLibrary::UpdateClient(UVNCClient* vncClient)
{
    if (vncClient == nullptr || !vncClient->IsInitComplete()) {
        return nullptr;
    }
    if (!vncClient->ClientServerMessages()) {
        vncClient->Reconnect();
    }
    if (!vncClient->ServerClientMessages()) {
        vncClient->Reconnect();
    }
    UTexture2D* texture = vncClient->GetTexture();
    texture->UpdateResource();
    return texture;
}