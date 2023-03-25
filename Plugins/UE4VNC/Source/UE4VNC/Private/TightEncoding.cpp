#include "TightEncoding.h"
#include "VNCCLient.h"
#include "Runtime/Core/Public/Async/ParallelFor.h"

TightEncoding::TightEncoding() :
    data(TArray<uint8>()),
    zStreams(TArray<std::shared_ptr<z_stream>>()),
    compressionType(ETightCompressionType::Invalid),
    filterType(ETightFilterType::CopyFilter),
    palette(TArray<uint8>()),
    usingZStreamId(0)
{
}

TightEncoding::~TightEncoding()
{
}

void TightEncoding::SetZStreams(TArray<std::shared_ptr<z_stream>> _zStreams) {
    zStreams = _zStreams;
}

bool TightEncoding::ReadData(
    FSocket* socket,
    uint16 xPosition,
    uint16 yPosition,
    uint16 width,
    uint16 height,
    SPixelFormat pixelFormat)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    uint8 compressionControl = 0;

    if (!UVNCClient::Recv(socket, waitTime, &compressionControl, 1, bytesRead)) {
        return false;
    }

    for (int i = 0; i < 4; i++) {
        bool resetStream = compressionControl & (1 << i);
        if (resetStream) {
            inflateEnd(zStreams[i].get());
            zStreams[i].reset();
            zStreams[i] = std::make_shared<z_stream>();
            zStreams[i]->zalloc = Z_NULL;
            zStreams[i]->zfree = Z_NULL;
            zStreams[i]->opaque = Z_NULL;
            zStreams[i]->next_in = Z_NULL;
            zStreams[i]->avail_in = 0;
            inflateInit(zStreams[i].get());
        }
    }

    if (!(compressionControl & 128)) {
        compressionType = ETightCompressionType::Basic;
        usingZStreamId = compressionControl >> 4 & 3;
        bool readFilterId = compressionControl & 64;

        if (!ReadDataBasic(socket, readFilterId, width, height, pixelFormat)) {
            return false;
        }
    }
    else {
        uint8 binCompressionType = compressionControl >> 4 & 31;

        switch (binCompressionType)
        {
        case 8:
            compressionType = ETightCompressionType::Fill;
            if (!ReadDataFill(socket, pixelFormat)) {
                return false;
            }
            break;
        case 9:
            compressionType = ETightCompressionType::Jpeg;
            if (!ReadDataJPEG(socket)) {
                return false;
            }
            break;
        default:
            compressionType = ETightCompressionType::Invalid;
            UE_LOG(LogTemp, Error, TEXT("Incorrect Tight compression type"));
            return false;
        }
    }

    return true;
}

void TightEncoding::DecompressData(TArray<uint8>& dist)
{
    const int CHUNK_SIZE = 16384;
    uint8 outbuff[CHUNK_SIZE];
    std::shared_ptr<z_stream> stream = zStreams[usingZStreamId];

    stream->avail_in = data.Num();

    if (stream->avail_in == 0) {
        return;
    }

    stream->next_in = data.GetData();

    do {
        stream->avail_out = CHUNK_SIZE;
        stream->next_out = outbuff;
        int result = inflate(stream.get(), Z_NO_FLUSH);
        if (result == Z_NEED_DICT || result == Z_DATA_ERROR ||
            result == Z_MEM_ERROR)
        {
            return;
        }

        uint32 nbytes = CHUNK_SIZE - stream->avail_out;
        dist.Append(outbuff, nbytes);
    } while (stream->avail_out == 0);
}

bool TightEncoding::ReadCompactLength(FSocket* socket, uint32& length)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    uint8 bytes[3] = { 0, 0, 0 };

    for (int i = 0; i < 3; i++) {
        if (!UVNCClient::Recv(socket, waitTime, &bytes[i], 1, bytesRead, false)) {
            return false;
        }
        if (!(bytes[i] & 128)) {
            break;
        }
    }
    length = (bytes[0] & 127) | (((uint32)bytes[1] & 127) << 7) | (((uint32)bytes[2] & 255) << 14);

    return true;
}

bool TightEncoding::ReadDataFill(FSocket* socket, SPixelFormat pixelFormat)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    uint32 pixelSize = pixelFormat.IsTPIXEL() ? 3 : (pixelFormat.bitsPerPixel / 8);
    data.SetNum(pixelSize);

    if (!UVNCClient::Recv(socket, waitTime, data.GetData(), pixelSize, bytesRead, false)) {
        return false;
    }

    return true;
}

bool TightEncoding::ReadDataJPEG(FSocket* socket)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    uint32 length = 0;
    if (!ReadCompactLength(socket, length)) {
        return false;
    }
    data.SetNum(length);

    if (!UVNCClient::Recv(socket, waitTime, data.GetData(), length, bytesRead, false)) {
        return false;
    }

    return true;
}

bool TightEncoding::ReadDataBasic(FSocket* socket, bool readFilterId, uint16 width, uint16 height, SPixelFormat pixelFormat)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    if (readFilterId) {
        if (!UVNCClient::Recv(socket, waitTime, (uint8*)&filterType, 1, bytesRead, false)) {
            return false;
        }
    }
    int32 bytesPerPixel = pixelFormat.IsTPIXEL() ? 3 : (pixelFormat.bitsPerPixel / 8);
    uint32 dataSize = width * height * bytesPerPixel;

    if (filterType == ETightFilterType::PaletteFilter) {
        uint8 paletteSize = 0;
        if (!UVNCClient::Recv(socket, waitTime, &paletteSize, 1, bytesRead, false)) {
            return false;
        }

        palette.SetNum((paletteSize + 1) * bytesPerPixel);

        if (!UVNCClient::Recv(socket, waitTime, palette.GetData(), (paletteSize + 1) * bytesPerPixel, bytesRead, false)) {
            return false;
        }

        if (paletteSize + 1 == 2) {
            dataSize = (width + 7) / 8 * height;
        }
        else {
            dataSize = width * height;
        }
    }

    if (dataSize < 12) {
        data.SetNum(dataSize);

        if (!UVNCClient::Recv(socket, waitTime, data.GetData(), dataSize, bytesRead, false)) {
            return false;
        }
    }
    else {
        uint32 length = 0;
        if (!ReadCompactLength(socket, length)) {
            return false;
        }
        data.SetNum(length);

        if (!UVNCClient::Recv(socket, waitTime, data.GetData(), length, bytesRead, false)) {
            return false;
        }
    }

    return true;
}

void TightEncoding::DrawToTexture(
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
    switch (compressionType)
    {
    case ETightCompressionType::Basic:
        DrawToTextureBasic(texture, xPosition, yPosition, width, height, framebufferWidth, framebufferHeight, pixelFormat);
        break;
    case ETightCompressionType::Fill:
        DrawToTextureFill(texture, xPosition, yPosition, width, height, framebufferWidth, framebufferHeight, pixelFormat);
        break;
    case ETightCompressionType::Jpeg:
        DrawToTextureJPEG(texture, xPosition, yPosition, width, height, framebufferWidth, framebufferHeight, pixelFormat);
        break;
    }
}

void TightEncoding::GetRawDataCopyFilter(TArray<uint8>& rawData, uint16 width, uint16 height, SPixelFormat pixelFormat)
{
    int32 bytesPerPixel = pixelFormat.IsTPIXEL() ? 3 : (pixelFormat.bitsPerPixel / 8);
    if (width * height * bytesPerPixel >= 12) {
        DecompressData(rawData);
    }
    else {
        rawData = data;
    }
}

void TightEncoding::GetRawDataPaletteFilter(TArray<uint8>& rawData, uint16 width, uint16 height, SPixelFormat pixelFormat)
{
    TArray<uint8> decompressedData;
    int32 bytesPerPixel = pixelFormat.IsTPIXEL() ? 3 : (pixelFormat.bitsPerPixel / 8);
    uint32 paletteSize = palette.Num() / bytesPerPixel;

    bool dataNotCompressed = paletteSize > 2 ? (width * height < 12) : ((width + 7) / 8 * height < 12);

    if (!dataNotCompressed) {
        DecompressData(decompressedData);
    }
    else {
        decompressedData = data;
    }
    rawData.SetNum(width * height * bytesPerPixel);

    if (paletteSize > 2) {
        ParallelFor(decompressedData.Num(), [&](int32 i) {
            for (int c = 0; c < bytesPerPixel; c++) {
                rawData[i * bytesPerPixel + c] = palette[decompressedData[i] * bytesPerPixel + c];
            }
        });
    }
    else {
        uint32 bytesPerRow = (width + 7) / 8;
        ParallelFor(height, [&](int32 i) {
            uint32 rowStart = bytesPerRow * i;

            ParallelFor(width, [&](int32 j) {
                uint8 curByte = decompressedData[rowStart + j / 8];
                uint8 shift = j % 8;
                uint32 distOffset = i * width + j;
                uint32 colorNum = (curByte >> shift) & 1;
                for (int c = 0; c < bytesPerPixel; c++) {
                    rawData[distOffset * bytesPerPixel + c] = palette[colorNum * bytesPerPixel + c];
                }
            });
        });
    }
}

void TightEncoding::DrawToTextureBasic(UTexture2D* texture, uint16 xPosition, uint16 yPosition, uint16 width, uint16 height, uint16 framebufferWidth, uint16 framebufferHeight, SPixelFormat pixelFormat)
{
    TArray<uint8> rawData;

    switch (filterType)
    {
    case ETightFilterType::CopyFilter:
        GetRawDataCopyFilter(rawData, width, height, pixelFormat);
        break;
    case ETightFilterType::PaletteFilter:
        GetRawDataPaletteFilter(rawData, width, height, pixelFormat);
        break;
    case ETightFilterType::GradientFilter:
        UE_LOG(LogTemp, Warning, TEXT("DrawToTextureGradient"));
        DrawToTextureGradient(texture, xPosition, yPosition, width, height, 
            framebufferWidth, framebufferHeight, pixelFormat);
        return;
    }

    FTexture2DMipMap* MipMap = &texture->PlatformData->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint8* rawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);

    uint8 pixelDataOrder[4] = { 0, 1, 2, 3 };
    if (pixelFormat.bigEndianFlag) {
        std::swap(pixelDataOrder[0], pixelDataOrder[3]);
        std::swap(pixelDataOrder[1], pixelDataOrder[2]);
    }
    if (pixelFormat.IsTPIXEL() && pixelFormat.redShift > pixelFormat.blueShift) {
        std::swap(pixelDataOrder[0], pixelDataOrder[2]);
    }

    int32 bytesPerPixel = pixelFormat.IsTPIXEL() ? 3 : (pixelFormat.bitsPerPixel / 8);
    uint16 redMax = (pixelFormat.redMax[0] << 8) | pixelFormat.redMax[1];
    uint16 greenMax = (pixelFormat.greenMax[0] << 8) | pixelFormat.greenMax[1];
    uint16 blueMax = (pixelFormat.blueMax[0] << 8) | pixelFormat.blueMax[1];
    uint64 alphaMax = ((((uint64)1) << (pixelFormat.bitsPerPixel - pixelFormat.depth)) - 1);

    ParallelFor(height, [&](int32 i) {
        ParallelFor(width, [&](int32 j) {
            int sourceOffset = i * width + j;
            int distOffset = (i + yPosition) * framebufferWidth + (j + xPosition);
            uint64 pixelData;
            std::memcpy(&pixelData, rawData.GetData() + sourceOffset * bytesPerPixel, bytesPerPixel);
            uint8 r = (pixelData >> pixelFormat.redShift) & redMax;
            uint8 g = (pixelData >> pixelFormat.greenShift) & greenMax;
            uint8 b = (pixelData >> pixelFormat.blueShift) & blueMax;
            uint8 a = (pixelData >> pixelFormat.depth) & alphaMax;
            uint8 components[4] = {
                r * 255 / redMax,
                g * 255 / greenMax,
                b * 255 / blueMax,
                255
            };

            if (pixelFormat.IsTPIXEL()) {
                components[3] = 255;
            }
            else {
                components[3] = alphaMax == 0 ? 255 : (a * 255 / alphaMax);
            }

            rawImageData[distOffset * 4 + 0] = components[pixelDataOrder[0]];
            rawImageData[distOffset * 4 + 1] = components[pixelDataOrder[1]];
            rawImageData[distOffset * 4 + 2] = components[pixelDataOrder[2]];
            rawImageData[distOffset * 4 + 3] = components[pixelDataOrder[3]];
        });
    });
    ImageData->Unlock();
}

void TightEncoding::DrawToTextureFill(UTexture2D* texture, uint16 xPosition, uint16 yPosition, uint16 width, uint16 height, uint16 framebufferWidth, uint16 framebufferHeight, SPixelFormat pixelFormat)
{
    FTexture2DMipMap* MipMap = &texture->PlatformData->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint8* rawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);

    uint8 pixelDataOrder[4] = { 0, 1, 2, 3 };
    if (pixelFormat.bigEndianFlag) {
        std::swap(pixelDataOrder[0], pixelDataOrder[3]);
        std::swap(pixelDataOrder[1], pixelDataOrder[2]);
    }
    if (pixelFormat.IsTPIXEL() && pixelFormat.redShift > pixelFormat.blueShift) {
        std::swap(pixelDataOrder[0], pixelDataOrder[2]);
    }

    int bytesPerPixel = pixelFormat.bitsPerPixel / 8;
    uint16 redMax = (pixelFormat.redMax[0] << 8) | pixelFormat.redMax[1];
    uint16 greenMax = (pixelFormat.greenMax[0] << 8) | pixelFormat.greenMax[1];
    uint16 blueMax = (pixelFormat.blueMax[0] << 8) | pixelFormat.blueMax[1];
    uint64 alphaMax = ((((uint64)1) << (pixelFormat.bitsPerPixel - pixelFormat.depth)) - 1);

    uint64 pixelData;
    std::memcpy(&pixelData, data.GetData(), bytesPerPixel);
    uint8 r = (pixelData >> pixelFormat.redShift) & redMax;
    uint8 g = (pixelData >> pixelFormat.greenShift) & greenMax;
    uint8 b = (pixelData >> pixelFormat.blueShift) & blueMax;
    uint8 a = (pixelData >> pixelFormat.depth) & alphaMax;
    
    uint8 components[4] = {
        r * 255 / redMax,
        g * 255 / greenMax,
        b * 255 / blueMax,
        255
    };

    if (pixelFormat.IsTPIXEL()) {
        components[3] = 255;
    }
    else {
        components[3] = alphaMax == 0 ? 255 : (a * 255 / alphaMax);
    }

    ParallelFor(height, [&](int32 i) {
        ParallelFor(width, [&](int32 j) {
            int distOffset = (i + yPosition) * framebufferWidth + (j + xPosition);
            rawImageData[distOffset * 4 + 0] = components[pixelDataOrder[0]];
            rawImageData[distOffset * 4 + 1] = components[pixelDataOrder[1]];
            rawImageData[distOffset * 4 + 2] = components[pixelDataOrder[2]];
            rawImageData[distOffset * 4 + 3] = components[pixelDataOrder[3]];
        });
    });

    ImageData->Unlock();
}

void TightEncoding::DrawToTextureJPEG(UTexture2D* texture, uint16 xPosition, uint16 yPosition, uint16 width, uint16 height, uint16 framebufferWidth, uint16 framebufferHeight, SPixelFormat pixelFormat)
{
    IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

    if (!ImageWrapper.IsValid()) {
        return;
    }

    if (!ImageWrapper->SetCompressed(data.GetData(), data.Num())) {
        return;
    }

    TArray<uint8> rawData;
    if (!ImageWrapper->GetRaw(ERGBFormat::RGBA, 8, rawData)) {
        return;
    }

    uint8 pixelDataOrder[4] = { 0, 1, 2, 3 };
    if (pixelFormat.bigEndianFlag) {
        std::swap(pixelDataOrder[0], pixelDataOrder[3]);
        std::swap(pixelDataOrder[1], pixelDataOrder[2]);
    }
    
    FTexture2DMipMap* MipMap = &texture->PlatformData->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint8* rawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);

    ParallelFor(height, [&](int32 i) {
        ParallelFor(width, [&](int32 j) {
            int sourceOffset = i * width + j;
            int distOffset = (i + yPosition) * framebufferWidth + (j + xPosition);
            rawImageData[distOffset * 4 + 0] = rawData[sourceOffset * 4 + pixelDataOrder[0]];
            rawImageData[distOffset * 4 + 1] = rawData[sourceOffset * 4 + pixelDataOrder[1]];
            rawImageData[distOffset * 4 + 2] = rawData[sourceOffset * 4 + pixelDataOrder[2]];
            rawImageData[distOffset * 4 + 3] = rawData[sourceOffset * 4 + pixelDataOrder[3]];
        });
    });

    ImageData->Unlock();
}

void TightEncoding::DrawToTextureGradient(UTexture2D* texture, uint16 xPosition, uint16 yPosition, uint16 width, uint16 height, uint16 framebufferWidth, uint16 framebufferHeight, SPixelFormat pixelFormat)
{
    TArray<uint8> decompressedData;
    int32 bytesPerPixel = pixelFormat.IsTPIXEL() ? 3 : (pixelFormat.bitsPerPixel / 8);
    int32 preprocBytesPerPixel = pixelFormat.IsTPIXEL() ? 3 : 4;

    if (width * height * bytesPerPixel >= 12) {
        DecompressData(decompressedData);
    }
    else {
        decompressedData = data;
    }

    /*TArray<uint8> preprocessedData;
    preprocessedData.SetNum(width * height * preprocBytesPerPixel);

    TArray<uint8> pixelDataOrder = { 0, 1, 2, 3 };
    if (pixelFormat.bigEndianFlag) {
        std::swap(pixelDataOrder[0], pixelDataOrder[3]);
        std::swap(pixelDataOrder[1], pixelDataOrder[2]);
    }
    if (pixelFormat.IsTPIXEL() && pixelFormat.redShift > pixelFormat.blueShift) {
        std::swap(pixelDataOrder[0], pixelDataOrder[2]);
    }

    uint16 redMax = (pixelFormat.redMax[0] << 8) | pixelFormat.redMax[1];
    uint16 greenMax = (pixelFormat.greenMax[0] << 8) | pixelFormat.greenMax[1];
    uint16 blueMax = (pixelFormat.blueMax[0] << 8) | pixelFormat.blueMax[1];
    uint64 alphaMax = ((((uint64)1) << (pixelFormat.bitsPerPixel - pixelFormat.depth)) - 1);

    ParallelFor(height, [&](int32 i) {
        ParallelFor(width, [&](int32 j) {
            int sourceOffset = i * width + j;
            int distOffset = (i + yPosition) * framebufferWidth + (j + xPosition);
            uint64 pixelData;
            std::memcpy(&pixelData, decompressedData.GetData() + sourceOffset * bytesPerPixel, bytesPerPixel);
            uint8 r = (pixelData >> pixelFormat.redShift) & redMax;
            uint8 g = (pixelData >> pixelFormat.greenShift) & greenMax;
            uint8 b = (pixelData >> pixelFormat.blueShift) & blueMax;
            uint8 a = (pixelData >> pixelFormat.depth) & alphaMax;
            uint8 components[4] = {
                r * 255 / redMax,
                g * 255 / greenMax,
                b * 255 / blueMax,
                255
            };

            if (pixelFormat.IsTPIXEL()) {
                components[3] = 255;
            }
            else {
                components[3] = alphaMax == 0 ? 255 : (a * 255 / alphaMax);
            }

            preprocessedData[distOffset * 4 + 0] = components[pixelDataOrder[0]];
            preprocessedData[distOffset * 4 + 1] = components[pixelDataOrder[1]];
            preprocessedData[distOffset * 4 + 2] = components[pixelDataOrder[2]];
            preprocessedData[distOffset * 4 + 3] = components[pixelDataOrder[3]];
        });
    });
    FTexture2DMipMap* MipMap = &texture->PlatformData->Mips[0];
    FByteBulkData* ImageData = &MipMap->BulkData;
    uint8* rawImageData = (uint8*)ImageData->Lock(LOCK_READ_WRITE);

    for (int c = 0; c < 4; c++) {
        rawImageData[c] = decompressedData[c];
    }

    for (int i = 1; i < width; i++) {
        for (int c = 0; c < 3; c++) {
            rawData[i * 3 + c] = decompressedData[i * 3 + c] + rawData[(i - 1) * 3 + c];
        }
    }

    for (int i = 1; i < height; i++) {
        for (int c = 0; c < 3; c++) {
            rawData[i * width * 3 + c] = decompressedData[i * width * 3 + c] + rawData[(i - 1) * width * 3 + c];
        }

        for (int j = 1; j < width; j++) {
            int offsetIJ = i * width + j;
            int offset_IJ = (i - 1) * width + j;
            int offsetI_J = i * width + j - 1;
            int offset_I_J = (i - 1) * width + j - 1;
            for (int c = 0; c < 3; c++) {
                rawData[offsetIJ * 3 + c] =
                    decompressedData[offsetIJ * 3 + c] +
                    rawData[offset_IJ * 3 + c] +
                    rawData[offsetI_J * 3 + c] -
                    rawData[offset_I_J * 3 + c];
            }
        }
    }

    ImageData->Unlock();*/
}
