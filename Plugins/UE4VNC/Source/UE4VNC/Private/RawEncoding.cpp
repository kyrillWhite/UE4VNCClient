#include "RawEncoding.h"
#include "VNCCLient.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

RawEncoding::RawEncoding() :
    data(TArray<uint8>())
{
}

RawEncoding::~RawEncoding()
{
}

bool RawEncoding::ReadData(
    FSocket* socket,
    uint16 xPosition,
    uint16 yPosition,
    uint16 width,
    uint16 height,
    SPixelFormat pixelFormat)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    uint8 bytesPerPixel = pixelFormat.bitsPerPixel / 8;
    data.Init(0, width * height * bytesPerPixel);

    if (!UVNCClient::Recv(socket, waitTime, data.GetData(), width * height * bytesPerPixel, bytesRead, false)) {
        return false;
    }

    return true;
}

void RawEncoding::DrawToTexture(
    UTexture2D* texture,
    uint16 xPosition,
    uint16 yPosition,
    uint16 width,
    uint16 height,
    uint16 framebufferWidth,
    uint16 framebufferHeight,
    SPixelFormat pixelFormat
)
{
    FTexture2DMipMap* MipMap = &texture->PlatformData->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint8* rawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);

    uint8 pixelDataOrder[4] = {0, 1, 2, 3};
    if (pixelFormat.bigEndianFlag) {
        std::swap(pixelDataOrder[0], pixelDataOrder[3]);
        std::swap(pixelDataOrder[1], pixelDataOrder[2]);
    }
    int bytesPerPixel = pixelFormat.bitsPerPixel / 8;
    uint16 redMax = (pixelFormat.redMax[0] << 8) | pixelFormat.redMax[1];
    uint16 greenMax = (pixelFormat.greenMax[0] << 8) | pixelFormat.greenMax[1];
    uint16 blueMax = (pixelFormat.blueMax[0] << 8) | pixelFormat.blueMax[1];
    uint64 alphaMax = ((((uint64)1) << (pixelFormat.bitsPerPixel - pixelFormat.depth)) - 1);

    ParallelFor(height, [&](int32 i) {
        ParallelFor(width, [&](int32 j) {
            int sourceOffset = i * width + j;
            int distOffset = (i + yPosition) * framebufferWidth + (j + xPosition);
            uint64 pixelData;
            std::memcpy(&pixelData, data.GetData() + sourceOffset * bytesPerPixel, bytesPerPixel);
            uint8 r = (pixelData >> pixelFormat.redShift) & redMax;
            uint8 g = (pixelData >> pixelFormat.greenShift) & greenMax;
            uint8 b = (pixelData >> pixelFormat.blueShift) & blueMax;
            uint8 a = (pixelData >> pixelFormat.depth) & alphaMax;
            uint8 components[4] = {
                r * 255 / redMax,
                g * 255 / greenMax,
                b * 255 / blueMax,
                alphaMax == 0 ? 255 : (a * 255 / alphaMax)
            };

            rawImageData[distOffset * 4 + 0] = components[pixelDataOrder[0]];
            rawImageData[distOffset * 4 + 1] = components[pixelDataOrder[1]];
            rawImageData[distOffset * 4 + 2] = components[pixelDataOrder[2]];
            rawImageData[distOffset * 4 + 3] = components[pixelDataOrder[3]];
        });
    });

    ImageData->Unlock();
}
