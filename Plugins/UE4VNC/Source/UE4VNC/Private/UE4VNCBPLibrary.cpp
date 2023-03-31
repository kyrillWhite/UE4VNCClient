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
    vncClient->SetSettings(host, port, password, allowJPEG, jpegQuality, compression, shared, pixelSize);
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
    texture->UpdateResource();
    return texture;
}