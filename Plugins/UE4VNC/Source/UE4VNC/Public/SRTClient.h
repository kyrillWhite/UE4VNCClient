#pragma once

#include <memory>

#include <Exporters/TextureExporterBMP.h>
#include "CoreMinimal.h"
#include "DynamicTexture.h"
#include "VideoDecoder.h"
#include "EAVPixelFormat.h"
#include "EAVHWDeviceType.h"
#include "EAVCodecID.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "srt/srt-live-transmit/transmitbase.hpp"
#include "SRTClient.generated.h"


/**
 *
 */
UCLASS(BlueprintType)
class UE4VNC_API USRTClient : public UObject
{
    GENERATED_BODY()
private:
    FString source;
    FString host;
    int port;
    size_t buffering;
    int pollid;
    std::unique_ptr<DynamicTexture> texture;
    std::shared_ptr<VideoDecoder> videoDecoder;
    UMaterialInstanceDynamic* dynamicMaterial;
    bool isClosed;


public:
    mutable FCriticalSection DataGuard;

    USRTClient();
    ~USRTClient();

    bool Initialise(
        UStaticMeshComponent* mesh,
        FString _host,
        int _port,
        size_t _buffering,
        int sourceWidth,
        int sourceHeight,
        EAVCodecID codecID,
        EAVPixelFormat sourcePixelFormat,
        EAVHWDeviceType hwDeviceType
    );
    void Listen();
    UTexture2D* CreateTexture(int32 inSizeX, int32 inSizeY);
    UTexture2D* GetTexture();
    void UpdateTexture();
    void SaveTexture(FString filename);
    void Close();
};