#include "CopyRectEncoding.h"
#include "VNCCLient.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

CopyRectEncoding::CopyRectEncoding() :
    srcXPosition(0),
    srcYPosition(0)
{
}

CopyRectEncoding::~CopyRectEncoding()
{
}

bool CopyRectEncoding::ReadData(
    FSocket* socket,
    uint16 xPosition,
    uint16 yPosition,
    uint16 width,
    uint16 height,
    SPixelFormat pixelFormat)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    uint8 buffer[4];

    if (!UVNCClient::Recv(socket, waitTime, buffer, 4, bytesRead, false)) {
        return false;
    }

    if (FGenericPlatformProperties::IsLittleEndian()) {
        std::swap(buffer[0], buffer[1]);
        std::swap(buffer[2], buffer[3]);
    }

    std::memcpy(&srcXPosition, buffer, 2);
    std::memcpy(&srcYPosition, buffer + 2, 2);

    return true;
}

void CopyRectEncoding::DrawToTexture(
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

    bool iNotInverse = yPosition < srcYPosition;
    int iStart = iNotInverse ? 0 : height - 1;
    int iEnd = iNotInverse ? height - 1 : 0;
    int iDelta = iNotInverse ? 1 : -1;

    bool jNotInverse = xPosition < srcXPosition;
    int jStart = jNotInverse ? 0 : width - 1;
    int jEnd = jNotInverse ? width - 1 : 0;
    int jDelta = jNotInverse ? 1 : -1;

    for (int i = iStart; iNotInverse ? (i <= iEnd) : (i >= iEnd); i += iDelta) {
        for (int j = jStart; jNotInverse ? (j <= jEnd) : (j >= jEnd); j += jDelta) {
            int sourceOffset = (i + srcYPosition) * framebufferWidth + (j + srcXPosition);
            int distOffset = (i + yPosition) * framebufferWidth + (j + xPosition);
            rawImageData[distOffset * 4 + 0] = rawImageData[sourceOffset * 4 + 0];
            rawImageData[distOffset * 4 + 1] = rawImageData[sourceOffset * 4 + 1];
            rawImageData[distOffset * 4 + 2] = rawImageData[sourceOffset * 4 + 2];
            rawImageData[distOffset * 4 + 3] = rawImageData[sourceOffset * 4 + 3];
        }
    }

    ImageData->Unlock();
}
