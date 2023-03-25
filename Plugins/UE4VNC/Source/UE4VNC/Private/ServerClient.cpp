#include "ServerClient.h"
#include "VNCClient.h"


ServerClient::ServerClient() :
    socket(nullptr),
    rectanglesLeftCount(0),
    zStreams(TArray<std::shared_ptr<z_stream>>())
{
    for (int i = 0; i < 4; i++) {
        auto zStream = std::make_shared<z_stream>();
        zStream->zalloc = Z_NULL;
        zStream->zfree = Z_NULL;
        zStream->opaque = Z_NULL;
        zStream->next_in = Z_NULL;
        zStream->avail_in = 0;
        inflateInit(zStream.get());
        zStreams.Add(zStream);
    }
}

ServerClient::~ServerClient()
{
    for (int i = 0; i < 4; i++) {
        inflateEnd(zStreams[i].get());
    }
}

void ServerClient::SetSocket(FSocket* _socket)
{
    socket = _socket;
}

void ServerClient::SetPixelFormat(SPixelFormat _pixelFormat)
{
    pixelFormat = _pixelFormat;
}

bool ServerClient::GetMessage(std::shared_ptr<StoCMessage>& message)
{
    if (rectanglesLeftCount > 0) {
        if (!FramebufferUpdate(message)) {
            return false;
        }
        return true;
    }

    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    EStoCMessageType messageType;
    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&messageType, 1, bytesRead)) {
        return false;
    }
    //UE_LOG(LogTemp, Error, TEXT("messageType: %i"), (uint8)messageType);

    switch (messageType) {
    case EStoCMessageType::FramebufferUpdate:
        if (!FramebufferUpdate(message)) {
            return false;
        }
        break;
    }

    return true;
}

uint16 ServerClient::GetRectanglesLeftCount()
{
    return rectanglesLeftCount;
}

bool ServerClient::FramebufferUpdate(std::shared_ptr<StoCMessage>& message)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;
    uint8 padding;

    if (rectanglesLeftCount == 0) {
        if (!UVNCClient::Recv(socket, waitTime, &padding, 1, bytesRead)) {
            return false;
        }
        uint16 rectanglesCount = 0;
        if (!UVNCClient::Recv(socket, waitTime, (uint8*)&rectanglesCount, 2, bytesRead)) {
            return false;
        }
        rectanglesLeftCount = rectanglesCount;
    }

    //UE_LOG(LogTemp, Error, TEXT("rectanglesLeftCount: %i"), rectanglesLeftCount);
    rectanglesLeftCount--;

    uint16 xPosition = 0;
    uint16 yPosition = 0;
    uint16 width = 0;
    uint16 height = 0;
    EEncodingType encodingType = EEncodingType::Raw;
    uint8 rectangleData[12];

    uint32 pendingData = 0;

    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&rectangleData, 12, bytesRead, false)) {
        return false;
    }

    if (FGenericPlatformProperties::IsLittleEndian()) {
        std::swap(rectangleData[0], rectangleData[1]);
        std::swap(rectangleData[2], rectangleData[3]);
        std::swap(rectangleData[4], rectangleData[5]);
        std::swap(rectangleData[6], rectangleData[7]);
        std::swap(rectangleData[8], rectangleData[11]);
        std::swap(rectangleData[9], rectangleData[10]);
    }

    std::memcpy(&xPosition, rectangleData, 2);
    std::memcpy(&yPosition, rectangleData + 2, 2);
    std::memcpy(&width, rectangleData + 4, 2);
    std::memcpy(&height, rectangleData + 6, 2);
    std::memcpy(&encodingType, rectangleData + 8, 4);

    //UE_LOG(LogTemp, Error, TEXT("rectangles data: %i, %i, %i, %i, %i"), xPosition, yPosition, width, height, (uint32)encodingType);

    std::shared_ptr<Encoding> encoding;

    switch (encodingType)
    {
    case EEncodingType::Raw:
        encoding = std::make_shared<RawEncoding>();
        break;
    case EEncodingType::CopyRect:
        encoding = std::make_shared<CopyRectEncoding>();
        break;
    case EEncodingType::Tight:
        encoding = std::make_shared<TightEncoding>();
        std::static_pointer_cast<TightEncoding>(encoding)->SetZStreams(zStreams);
        break;
    default:
        UE_LOG(LogTemp, Error, TEXT("Unknown encoding: %i"), encodingType);
        return false;
    }

    encoding->ReadData(socket, xPosition, yPosition, width, height, pixelFormat);

    message = std::make_shared<FramebufferRectangleMessage>(
        xPosition,
        yPosition,
        width,
        height,
        encoding
    );

    return true;
}
