// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "Common/TcpSocketBuilder.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "ESecurityType.h"
#include "Handshaking.h"
#include "Initialisation.h"
#include "ClientServer.h"
#include "ServerClient.h"
#include "StoCMessage.h"
#include "Math/UnrealMathUtility.h"
#include "FKeysMap.h"
#include "Control.h"
#include "VNCClient.generated.h"

/**
 *
 */
UCLASS(BlueprintType)
class UE4VNC_API UVNCClient : public UObject
{
    GENERATED_BODY()
private:
    FSocket* socket;
    bool onlyControl;
    FString host; 
    int port;
    FString password;
    bool allowJPEG;
    int jpegQuality;
    int compression;
    bool shared;
    EPixelFormatType pixelFormatType;
    int majorVersion;
    int minorVersion;
    ESecurityType securityType;
    std::unique_ptr<Handshaking> handshaking;
    std::unique_ptr<Initialisation> initialisation;
    SPixelFormat pixelFormat;
    uint16 framebufferWidth;
    uint16 framebufferHeight;
    std::shared_ptr<ClientServer> clientServer;
    std::unique_ptr<ServerClient> serverClient;
    bool initComplete;
    UTexture2D* texture;
    TFuture<bool> serverClientMessagesResult;
    TQueue<std::shared_ptr<StoCMessage>> messagesQueue;
    TSharedFuture<void> connectingResult;
    FThreadSafeBool destruct;
    UStaticMeshComponent* mesh;
    std::unique_ptr<Control> control;

public:
    UVNCClient();
    ~UVNCClient();

    bool Connect(FString host, int port);
    bool FullConnect();
    bool Handshake(FString password);
    bool Initialise(
        bool _allowJPEG,
        int _jpegQuality,
        int _compression,
        bool _shared,
        EPixelFormatType _pixelSize
    );
    bool IsInitComplete();
    bool IsConnecting();
    bool IsConnected();
    bool ClientServerMessages();
    bool ServerClientMessages();
    bool HandleInputEvent(FKey key, bool isPressed, bool isShiftHolding = false);
    UTexture2D* CreateTexture(int32 inSizeX, int32 inSizeY, SPixelFormat _pixelFormat);
    UTexture2D* GetTexture();
    void UpdateTexture(std::shared_ptr<FramebufferRectangleMessage> framebufferUpdateMessage);
    void SetSettings(
        bool _onlyControl,
        FString _host,
        int _port,
        FString _password,
        bool _allowJPEG,
        int _jpegQuality,
        int _compression,
        bool _shared,
        EPixelFormatType _pixelSize,
        UStaticMeshComponent* _mesh,
        APlayerController* _playerController,
        UWidgetComponent* _keyboardWidgetComponent,
        UWidgetInteractionComponent* _widgetInteractionComponent
    );

    static bool Recv(FSocket* socket, FTimespan waitTime, uint8* data, int32 bufferSize, int32& bytesRead, bool reverse = true);
    static bool Send(FSocket* socket, FTimespan waitTime, const uint8* data, int32 count, int32& bytesSent, bool reverse = true);

    static FString GetConnectionFailedReason(FSocket* socket);
    static FString BytesToStringFixed(const uint8* In, int32 Count);
    static uint8 ReverseUint8(uint8 b);
};