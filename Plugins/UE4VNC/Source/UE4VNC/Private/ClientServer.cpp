#include "ClientServer.h"
#include "VNCClient.h"

ClientServer::ClientServer() :
    socket(nullptr)
{
}

ClientServer::~ClientServer()
{
}

void ClientServer::SetSocket(FSocket* _socket)
{
    socket = _socket;
}

bool ClientServer::FramebufferUpdateRequest(uint16 xPosition, uint16 yPosition, uint16 width, uint16 height, bool incremental)
{
    if (socket == nullptr) {
        return false;
    }

    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesSent;
    ECtoSMessageType messageType = ECtoSMessageType::FramebufferUpdateRequest;
    SFrameBufferRequestMessage message = {
        (uint8)messageType,
        (uint8)incremental,
        (uint8)(xPosition & 255),
        (uint8)(xPosition >> 8),
        (uint8)(yPosition & 255),
        (uint8)(yPosition >> 8),
        (uint8)(width & 255),
        (uint8)(width >> 8),
        (uint8)(height & 255),
        (uint8)(height >> 8)
    };

    if (FGenericPlatformProperties::IsLittleEndian()) {
        std::swap(message.xPosition[0], message.xPosition[1]);
        std::swap(message.yPosition[0], message.yPosition[1]);
        std::swap(message.width[0], message.width[1]);
        std::swap(message.height[0], message.height[1]);
    }

    if (!UVNCClient::Send(socket, waitTime, (uint8*)&message, 10, bytesSent, false)) {
        return false;
    }

    return true;
}

bool ClientServer::SetEncodings(TArray<EEncodingType> encodings)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesSent;
    ECtoSMessageType messageType = ECtoSMessageType::SetEncodings;

    uint16 encodingsCount = encodings.Num();
    TArray<uint8> message = {
        (uint8)messageType,
        0,
        (uint8)(encodingsCount & 255),
        (uint8)(encodingsCount >> 8)
    };

    if (FGenericPlatformProperties::IsLittleEndian()) {
        std::swap(message[2], message[3]);
    }

    for (int i = 0; i < encodingsCount; i++) {
        uint8* encoding = static_cast<uint8*>(static_cast<void*>(&encodings[i]));

        if (FGenericPlatformProperties::IsLittleEndian()) {
            std::reverse(encoding, encoding + 4);
        }

        message.Append(encoding, 4);
    }

    if (!UVNCClient::Send(socket, waitTime, message.GetData(), message.Num(), bytesSent, false)) {
        return false;
    }

    return true;
}

bool ClientServer::SetPixelFormatType(EPixelFormatType pixelFormatType, SPixelFormat& pixelFormat)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesSent;
    SPixelFormat newPixelFormat;

    switch (pixelFormatType)
    {
    case EPixelFormatType::Full:
        newPixelFormat = { 32, 24, 0, 1, 0, 255, 0, 255, 0, 255, 16, 8, 0 };
        break;
    case EPixelFormatType::High:
        newPixelFormat = { 16, 12, 0, 1, 0, 15, 0, 15, 0, 15, 8, 4, 0 };
        break;
    case EPixelFormatType::Low:
        newPixelFormat = { 8, 8, 0, 1, 0, 7, 0, 7, 0, 3, 5, 2, 0 };
        break;
    default:
        UE_LOG(LogTemp, Error, TEXT("Incorrect pixel format type"));
        return false;
    }

    ECtoSMessageType messageType = ECtoSMessageType::SetPixelFormat;
    TArray<uint8> message = { (uint8)messageType, 0, 0, 0 };
    message.Append((uint8*)&newPixelFormat, sizeof(newPixelFormat));

    if (!UVNCClient::Send(socket, waitTime, message.GetData(), message.Num(), bytesSent, false)) {
        return false;
    }

    pixelFormat = newPixelFormat;

    return true;
}

bool ClientServer::KeyEvent(SKeyEvent keyEvent)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesSent;

    ECtoSMessageType messageType = ECtoSMessageType::KeyEvent;
    TArray<uint8> message = {
        (uint8)messageType,
        keyEvent.isDown,
        0,
        0,
        uint8(keyEvent.key >> 24 & 255),
        uint8(keyEvent.key >> 16 & 255),
        uint8(keyEvent.key >> 8 & 255),
        uint8(keyEvent.key & 255),
    };

    if (!UVNCClient::Send(socket, waitTime, message.GetData(), message.Num(), bytesSent, false)) {
        return false;
    }

    return true;
}

bool ClientServer::PointerEvent(SMouseEvent mouseEvent)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesSent;
    uint8 buttonMask =
        mouseEvent.leftPressed        |
        mouseEvent.middlePressed << 1 |
        mouseEvent.rightPressed  << 2 |
        mouseEvent.wheelUp       << 3 |
        mouseEvent.wheelDown     << 4 |
        mouseEvent.wheelLeft     << 5 |
        mouseEvent.wheelRight    << 6;

    ECtoSMessageType messageType = ECtoSMessageType::PointerEvent;
    TArray<uint8> message = {
        (uint8)messageType,
        buttonMask,
        uint8(mouseEvent.x >> 8 & 255),
        uint8(mouseEvent.x & 255),
        uint8(mouseEvent.y >> 8 & 255),
        uint8(mouseEvent.y & 255),
    };

    //message.Append((uint8*)&position, sizeof(position));

    if (!UVNCClient::Send(socket, waitTime, message.GetData(), message.Num(), bytesSent, false)) {
        return false;
    }

    return true;
}
