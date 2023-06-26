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
            if (pixelFormat.bigEndianFlag != !FGenericPlatformProperties::IsLittleEndian()) {
                std::reverse((uint8*)&pixelData, (uint8*)&pixelData + bytesPerPixel);
            }
            uint8 r = (pixelData >> pixelFormat.redShift) & redMax;
            uint8 g = (pixelData >> pixelFormat.greenShift) & greenMax;
            uint8 b = (pixelData >> pixelFormat.blueShift) & blueMax;
            uint8 a = (pixelData >> pixelFormat.depth) & alphaMax;

            rawImageData[distOffset * 4 + 0] = r * 255 / redMax;
            rawImageData[distOffset * 4 + 1] = g * 255 / greenMax;
            rawImageData[distOffset * 4 + 2] = b * 255 / blueMax;
            rawImageData[distOffset * 4 + 3] = alphaMax == 0 ? 255 : (a * 255 / alphaMax);
        });
    });

    ImageData->Unlock();
}
