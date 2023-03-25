// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <memory>

#include "CoreMinimal.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "Encoding.h"
#include "ETightCompressionType.h"
#include "ETightFilterType.h"

#if defined(_WIN64)
#include "ThirdParty/zlib/v1.2.8/include/Win64/VS2015/zlib.h"
#elif defined(_WIN32)
#include "ThirdParty/zlib/v1.2.8/include/Win32/VS2015/zlib.h"
#elif defined(__linux__)
#include "ThirdParty/zlib/v1.2.8/include/Linux/x86_64-unknown-linux-gnu/zlib.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "ThirdParty/zlib/v1.2.8/include/Mac/zlib.h"
#endif


/**
 *
 */
class UE4VNC_API TightEncoding : public Encoding
{
private:
    TArray<uint8> data;
    TArray<std::shared_ptr<z_stream>> zStreams;
    ETightCompressionType compressionType;
    ETightFilterType filterType;
    TArray<uint8> palette;
    uint8 usingZStreamId;

    bool ReadCompactLength(FSocket* socket, uint32& length);
    bool ReadDataFill(FSocket* socket, SPixelFormat pixelFormat);
    bool ReadDataJPEG(FSocket* socket);
    bool ReadDataBasic(FSocket* socket, bool readFilterId, uint16 width, uint16 height, SPixelFormat pixelFormat);

    void DecompressData(TArray<uint8>& dist);
    void GetRawDataCopyFilter(TArray<uint8>& rawData, uint16 width, uint16 height, SPixelFormat pixelFormat);
    void GetRawDataPaletteFilter(TArray<uint8>& rawData, uint16 width, uint16 height, SPixelFormat pixelFormat);

    void DrawToTextureBasic(
        UTexture2D* texture,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        uint16 framebufferWidth,
        uint16 framebufferHeight,
        SPixelFormat pixelFormat
    );

    void DrawToTextureFill(
        UTexture2D* texture,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        uint16 framebufferWidth,
        uint16 framebufferHeight,
        SPixelFormat pixelFormat
    );

    void DrawToTextureJPEG(
        UTexture2D* texture,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        uint16 framebufferWidth,
        uint16 framebufferHeight,
        SPixelFormat pixelFormat
    );

    void DrawToTextureGradient(
        UTexture2D* texture,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        uint16 framebufferWidth,
        uint16 framebufferHeight,
        SPixelFormat pixelFormat
    );

public:
    TightEncoding();
    ~TightEncoding();

    void SetZStreams(TArray<std::shared_ptr<z_stream>> _zStreams);

    bool ReadData(
        FSocket* socket,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        SPixelFormat pixelFormat
    ) override;

    void DrawToTexture(
        UTexture2D* texture,
        uint16 xPosition,
        uint16 yPosition,
        uint16 width,
        uint16 height,
        uint16 framebufferWidth,
        uint16 framebufferHeight,
        SPixelFormat pixelFormat
    ) override;
};