// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

extern "C" {
#include <libavutil/pixfmt.h>
}
#include "EAVPixelFormat.generated.h"
/**
 * 
 */
UENUM()
enum class EAVPixelFormat : uint8 {
    AV_PIX_FMT_YUV420P   = AVPixelFormat::AV_PIX_FMT_YUV420P    UMETA(DisplayName = "YUV420P 12bpp"),
    AV_PIX_FMT_RGB24     = AVPixelFormat::AV_PIX_FMT_RGB24      UMETA(DisplayName = "RGB24 24bpp"),
    AV_PIX_FMT_BGR24     = AVPixelFormat::AV_PIX_FMT_BGR24      UMETA(DisplayName = "BGR24 24bpp"),
    AV_PIX_FMT_YUV422P   = AVPixelFormat::AV_PIX_FMT_YUV422P    UMETA(DisplayName = "YUV422P 16bpp"),
    AV_PIX_FMT_MONOBLACK = AVPixelFormat::AV_PIX_FMT_MONOBLACK  UMETA(DisplayName = "MONOBLACK 1bpp"),
    AV_PIX_FMT_ARGB      = AVPixelFormat::AV_PIX_FMT_ARGB       UMETA(DisplayName = "ARGB 32bpp"),
    AV_PIX_FMT_RGBA      = AVPixelFormat::AV_PIX_FMT_RGBA       UMETA(DisplayName = "RGBA 32bpp"),
    AV_PIX_FMT_GRAY8     = AVPixelFormat::AV_PIX_FMT_GRAY8      UMETA(DisplayName = "GRAY8 8bpp"),
    AV_PIX_FMT_NV12      = AVPixelFormat::AV_PIX_FMT_NV12       UMETA(DisplayName = "NV12 12bpp"),
};
