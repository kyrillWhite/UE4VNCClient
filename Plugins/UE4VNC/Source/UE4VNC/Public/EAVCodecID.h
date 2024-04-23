// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

extern "C" {
#include <libavcodec/codec_id.h>
}
#include "EAVCodecID.generated.h"
/**
 * 
 */
UENUM()
enum class EAVCodecID : uint8 {
    AV_CODEC_ID_H264     = AVCodecID::AV_CODEC_ID_H264     UMETA(DisplayName = "H264"),
    AV_CODEC_ID_HEVC     = AVCodecID::AV_CODEC_ID_HEVC     UMETA(DisplayName = "H265 (HEVC)"),
    AV_CODEC_ID_RAWVIDEO = AVCodecID::AV_CODEC_ID_RAWVIDEO UMETA(DisplayName = "Raw Video"),
    //AV_CODEC_ID_H261     = AVCodecID::AV_CODEC_ID_H261     UMETA(DisplayName = "H261"),
    //AV_CODEC_ID_H263     = AVCodecID::AV_CODEC_ID_H263     UMETA(DisplayName = "H263"),
    //AV_CODEC_ID_MPEG4    = AVCodecID::AV_CODEC_ID_MPEG4    UMETA(DisplayName = "MPEG4"),
    //AV_CODEC_ID_VP8      = AVCodecID::AV_CODEC_ID_VP8      UMETA(DisplayName = "VP8"),
    //AV_CODEC_ID_VP9      = AVCodecID::AV_CODEC_ID_VP9      UMETA(DisplayName = "VP9"),
    //AV_CODEC_ID_AV1      = AVCodecID::AV_CODEC_ID_AV1      UMETA(DisplayName = "AV1")
};
