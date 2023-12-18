// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Common/TcpSocketBuilder.h"
#include <list>
#include <memory>
#include <fstream>
#include <srt/srt-live-transmit/transmitbase.hpp>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/opt.h>
    #include <libavutil/imgutils.h>
    #include <libswscale/swscale.h>
    #include <libavformat/avformat.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
}
/**
 *
 */
class UE4VNC_API VideoDecoder
{
private:
    AVFrame* frame;
    SwsContext* swsContext;
    AVCodecParserContext* parser;
    const AVCodec* codec;
    AVCodecContext* context;
    AVBufferRef* hw_device_ctx = NULL;
    AVCodecID codecID;

    AVPixelFormat sourcePixelFormat;

    int rawBufferSize;
    int dataRawBufferSize;

    void YUVFrameToRGBData(const AVFrame& frame, uint8_t* data);
    int Decode(AVPacket* packet, uint8* textureData);

public:
    VideoDecoder();
    ~VideoDecoder();

    bool Initialise(
        int sourceWith,
        int sourceHeight,
        AVCodecID codecID,
        AVPixelFormat _sourcePixelFormat,
        AVHWDeviceType hwDeviceType
    );
    int ProcessPacket(std::shared_ptr<MediaPacket> packet, uint8* textureData);
    void CleanUp();
};