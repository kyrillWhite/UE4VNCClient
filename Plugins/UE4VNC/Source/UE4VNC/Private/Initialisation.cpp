#include "Initialisation.h"
#include "VNCClient.h"

Initialisation::Initialisation()
{

}

Initialisation::~Initialisation()
{

}

bool Initialisation::ClientInit(FSocket* socket, bool shared)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesSent;

    if (socket == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("Socket is nullptr"));
        return false;
    }

    if (!UVNCClient::Send(socket, waitTime, (uint8*)&shared, 1, bytesSent, false)) {
        return false;
    }

    return true;
}

bool Initialisation::ServerInit(FSocket* socket, SPixelFormat& pixelFormat, uint16& framebufferWidth, uint16& framebufferHeight)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;

    uint32 nameLength = 0;

    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&framebufferWidth, 2, bytesRead)) {
        return false;
    }
    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&framebufferHeight, 2, bytesRead)) {
        return false;
    }
    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&pixelFormat, 16, bytesRead, false)) {
        return false;
    }
    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&nameLength, 4, bytesRead)) {
        return false;
    }
    uint8* nameArray = new uint8[nameLength];
    if (!UVNCClient::Recv(socket, waitTime, nameArray, nameLength, bytesRead, false)) {
        delete[] nameArray;
        return false;
    }

    FString name = UVNCClient::BytesToStringFixed(nameArray, nameLength);

    delete[] nameArray;
    return true;
}
