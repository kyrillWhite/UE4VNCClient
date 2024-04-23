// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

extern "C" {
#include <libavutil/hwcontext.h>
}
#include "EAVHWDeviceType.generated.h"
/**
*
*/
UENUM()
    enum class EAVHWDeviceType : uint8 {
    AV_HWDEVICE_TYPE_NONE            = AVHWDeviceType::AV_HWDEVICE_TYPE_NONE         UMETA(DisplayName = "NONE"),
    AV_HWDEVICE_TYPE_D3D11VA         = AVHWDeviceType::AV_HWDEVICE_TYPE_D3D11VA      UMETA(DisplayName = "D3D11VA"),
    AV_HWDEVICE_TYPE_OPENCL          = AVHWDeviceType::AV_HWDEVICE_TYPE_OPENCL       UMETA(DisplayName = "OPENCL"),
    AV_HWDEVICE_TYPE_VULKAN          = AVHWDeviceType::AV_HWDEVICE_TYPE_VULKAN       UMETA(DisplayName = "VULKAN"),

    AV_HWDEVICE_TYPE_CUDA            = AVHWDeviceType::AV_HWDEVICE_TYPE_CUDA         UMETA(DisplayName = "CUDA"),
    AV_HWDEVICE_TYPE_DXVA2           = AVHWDeviceType::AV_HWDEVICE_TYPE_DXVA2        UMETA(DisplayName = "DXVA2"),
    AV_HWDEVICE_TYPE_QSV             = AVHWDeviceType::AV_HWDEVICE_TYPE_QSV          UMETA(DisplayName = "QSV"),
    AV_HWDEVICE_TYPE_DRM             = AVHWDeviceType::AV_HWDEVICE_TYPE_DRM          UMETA(DisplayName = "DRM"),
    //AV_HWDEVICE_TYPE_VIDEOTOOLBOX  = AVHWDeviceType::AV_HWDEVICE_TYPE_VIDEOTOOLBOX UMETA(DisplayName = "VIDEOTOOLBOX"),
    //AV_HWDEVICE_TYPE_VAAPI         = AVHWDeviceType::AV_HWDEVICE_TYPE_VAAPI        UMETA(DisplayName = "VAAPI"),
    //AV_HWDEVICE_TYPE_VDPAU         = AVHWDeviceType::AV_HWDEVICE_TYPE_VDPAU        UMETA(DisplayName = "VDPAU"),
    //AV_HWDEVICE_TYPE_MEDIACODEC    = AVHWDeviceType::AV_HWDEVICE_TYPE_MEDIACODEC   UMETA(DisplayName = "MEDIACODEC"),
};