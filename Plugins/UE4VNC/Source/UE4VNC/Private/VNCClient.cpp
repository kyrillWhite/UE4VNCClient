#include "VNCClient.h"

#include <algorithm>

UVNCClient::UVNCClient() :
    host(""),
    port(5900),
    password(""),
    allowJPEG(true),
    jpegQuality(8),
    compression(9),
    shared(false),
    pixelFormatType(EPixelFormatType::Full),
    socket(nullptr),
    majorVersion(0),
    minorVersion(0),
    securityType(ESecurityType::Invalid),
    handshaking(std::make_unique<Handshaking>()),
    initialisation(std::make_unique<Initialisation>()),
    framebufferWidth(0),
    framebufferHeight(0),
    clientServer(std::make_unique<ClientServer>()),
    serverClient(std::make_unique<ServerClient>()),
    initComplete(false),
    texture(nullptr),
    serverClientMessagesResult(TFuture<bool>()),
    connectingResult(TSharedFuture<void>()),
    destruct(false)
{
}

UVNCClient::~UVNCClient()
{
    destruct = true;
    if (socket != nullptr) {
        if (socket->GetConnectionState() == ESocketConnectionState::SCS_Connected) {
            socket->Close();
        }
        delete socket;
    }
}

bool UVNCClient::Connect(FString _host, int _port)
{
    host = _host;
    port = _port;
    FIPv4Address ipAddress;

    if (!FIPv4Address::Parse(host, ipAddress)) {
        UE_LOG(LogTemp, Error, TEXT("Invalid host or port. host: %s, port: %i"), *host, port);
        return false;
    }

    FIPv4Endpoint endpoint(ipAddress, (uint16)port);

    socket = FTcpSocketBuilder(TEXT("TcpSocket")).AsReusable();
    ISocketSubsystem* socketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

    if (!socket->Connect(*socketSubsystem->CreateInternetAddr(endpoint.Address.Value, endpoint.Port))) {
        UE_LOG(LogTemp, Error, TEXT("No connect. host: %s, port: %i"), *host, port);
        return false;
    }

    handshaking->SetSocket(socket);
    clientServer->SetSocket(socket);
    serverClient->SetSocket(socket);

    UE_LOG(LogTemp, Log, TEXT("Connected. host: %s, port: %i"), *host, port);
    return true;
}

bool UVNCClient::FullConnect()
{
    serverClient->ReopenZStreams();
    if (connectingResult.IsValid() && !connectingResult.IsReady()) {
        return false;
    }
    //FAsyncTask<void>* NewTimer = new FAsyncTask<YimerTh>();

    connectingResult = Async(EAsyncExecution::TaskGraph, [&]() {
        bool reconnectSuccessful = false;

        while (!reconnectSuccessful) {
            if (destruct) {
                return;
            }
            if (socket != nullptr) {
                if (!socket->Close()) {
                    continue;
                }
            }
            if (!Connect(host, port)) {
                UE_LOG(LogTemp, Error, TEXT("Reconnect error. host: %s, port: %i"), *host, port);
                continue;
            }
            if (!Handshake(password)) {
                UE_LOG(LogTemp, Error, TEXT("Handshake error"));
                continue;
            }
            if (!Initialise(allowJPEG, jpegQuality, compression, shared, pixelFormatType)) {
                UE_LOG(LogTemp, Error, TEXT("Initialise error"));
                continue;
            }

            UE_LOG(LogTemp, Log, TEXT("Reconnected. host: %s, port: %i"), *host, port);
            reconnectSuccessful = true;
        }
    });
    return true;
}

bool UVNCClient::Handshake(FString _password)
{
    password = _password;
    bool passSecurityResult;

    if (!handshaking->ProtocolVersion(majorVersion, minorVersion)) {
        UE_LOG(LogTemp, Error, TEXT("Handshake protocol version error"));
        return false;
    }
    if (!handshaking->Security(securityType)) {
        UE_LOG(LogTemp, Error, TEXT("Handshake security error"));
        return false;
    }
    if (!handshaking->SecurityTypes(passSecurityResult, password)) {
        UE_LOG(LogTemp, Error, TEXT("Handshake security types error"));
        return false;
    }
    if (!passSecurityResult) {
        if (!handshaking->SecurityResult()) {
            UE_LOG(LogTemp, Error, TEXT("Handshake security result error"));
            return false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Handshake done"));
    return true;
}

bool UVNCClient::Initialise(
    bool _allowJPEG,
    int _jpegQuality,
    int _compression,
    bool _shared,
    EPixelFormatType _pixelSize
)
{
    allowJPEG = _allowJPEG;
    jpegQuality = _jpegQuality;
    compression = _compression;
    shared = _shared;
    pixelFormatType = _pixelSize;

    if (jpegQuality > 9) {
        jpegQuality = 9;
    } 
    else if (jpegQuality < 0) {
        jpegQuality = 0;
    }
    if (compression > 9) {
        compression = 9;
    }
    else if (compression < 0) {
        compression = 0;
    }


    if (!initialisation->ClientInit(socket, shared)) {
        UE_LOG(LogTemp, Error, TEXT("Client initialisation error"));
        return false;
    }
    if (!initialisation->ServerInit(socket, pixelFormat, framebufferWidth, framebufferHeight)) {
        UE_LOG(LogTemp, Error, TEXT("Server initialisation error"));
        return false;
    }

    if (!clientServer->SetPixelFormatType(pixelFormatType, pixelFormat)) {
        UE_LOG(LogTemp, Error, TEXT("Set pixel format error"));
        return false;
    }
        
    TArray<EEncodingType> encodigTypes = { EEncodingType::Tight, EEncodingType::CopyRect, EEncodingType::Raw};
    if (allowJPEG) {
        encodigTypes.Add((EEncodingType)((int32)EEncodingType::JPEGQualityLevel - (9 - jpegQuality)));
    }
    encodigTypes.Add((EEncodingType)((int32)EEncodingType::CompressionLevel - (9 - compression)));
    
    if (!clientServer->SetEncodings(encodigTypes)) {
        UE_LOG(LogTemp, Error, TEXT("Set encodings error"));
        return false;
    }

    serverClient->SetPixelFormat(pixelFormat);
    if (texture == nullptr) {
        texture = CreateTexture(framebufferWidth, framebufferHeight, pixelFormat);
    }

    UE_LOG(LogTemp, Error, TEXT("Initialisation complete"));
    initComplete = true;

    return true;
}

bool UVNCClient::IsInitComplete()
{
    return initComplete;
}

bool UVNCClient::IsConnecting()
{
    return connectingResult.IsValid() && !connectingResult.IsReady();
}

bool UVNCClient::IsConnected()
{
    return connectingResult.IsValid() && connectingResult.IsReady();
}

bool UVNCClient::ClientServerMessages()
{
    if (!clientServer->FramebufferUpdateRequest(0, 0, framebufferWidth, framebufferHeight)) {
        return false;
    }

    return true;
}

bool UVNCClient::ServerClientMessages()
{
    while (!messagesQueue.IsEmpty()) {
        std::shared_ptr<StoCMessage> message;
        messagesQueue.Peek(message);
        messagesQueue.Pop();

        switch (message->GetMessageType())
        {
        case EStoCMessageType::FramebufferUpdate:
            UpdateTexture(std::static_pointer_cast<FramebufferRectangleMessage>(message));
            break;
        }
    }

    if (serverClientMessagesResult.IsValid()) {
        if (serverClientMessagesResult.IsReady()) {
            if (!serverClientMessagesResult.Get()) {
                return false;
            }
        }
        else {
            return true;
        }
    }

    serverClientMessagesResult = Async(EAsyncExecution::TaskGraph, [&]() {
        uint32 pendingDataSize;
        while (socket->HasPendingData(pendingDataSize)) {
            std::shared_ptr<StoCMessage> message;

            if (!serverClient->GetMessage(message)) {
                return false;
            }

            while (serverClient->GetRectanglesLeftCount() > 0 &&
                !socket->HasPendingData(pendingDataSize)) {
                if (!clientServer->FramebufferUpdateRequest(0, 0, framebufferWidth, framebufferHeight)) {
                    return false;
                }
            }
            messagesQueue.Enqueue(message);
        }
        return true;
    });

    return true;
}

UTexture2D* UVNCClient::CreateTexture(int32 inSizeX, int32 inSizeY, SPixelFormat _pixelFormat)
{
    EPixelFormat _ePixelFormat = EPixelFormat::PF_R8G8B8A8;
    return UTexture2D::CreateTransient(inSizeX, inSizeY, _ePixelFormat);
}

UTexture2D* UVNCClient::GetTexture()
{
    return texture;
}

void UVNCClient::UpdateTexture(std::shared_ptr<FramebufferRectangleMessage> framebufferUpdateMessage)
{
    framebufferUpdateMessage->DrawToTexture(texture, pixelFormat, framebufferWidth, framebufferHeight);
}



void UVNCClient::SetSettings(FString _host, int _port, FString _password, bool _allowJPEG, int _jpegQuality, int _compression, bool _shared, EPixelFormatType _pixelSize)
{
    host = _host;
    port = _port;
    password = _password;
    allowJPEG = _allowJPEG;
    jpegQuality = _jpegQuality;
    compression = _compression;
    shared = _shared;
    pixelFormatType = _pixelSize;
}

bool UVNCClient::Recv(FSocket* socket, FTimespan waitTime, uint8* data, int32 bufferSize, int32& bytesRead, bool reverse)
{
    int32 allBytesRead = 0;

    while (allBytesRead < bufferSize) {
        if (!socket->Wait(ESocketWaitConditions::WaitForRead, waitTime)) {
            UE_LOG(LogTemp, Error, TEXT("Server not available for recieve. All bytes read: %i; Bytes read: %i; Buffer size: %i"), allBytesRead, bytesRead, bufferSize);
            return false;
        }
        if (!socket->Recv(data + allBytesRead, bufferSize - allBytesRead, bytesRead)) {
            UE_LOG(LogTemp, Error, TEXT("No answer in recieving"));
            return false;
        }

        allBytesRead += bytesRead;
    }

    if (reverse && FGenericPlatformProperties::IsLittleEndian()) {
        std::reverse(data, data + bufferSize);
    }

    return true;
}

bool UVNCClient::Send(FSocket* socket, FTimespan waitTime, const uint8* data, int32 count, int32& bytesSent, bool reverse)
{
    uint8* sendingData = new uint8[count];
    memcpy(sendingData, data, count);

    if (reverse && FGenericPlatformProperties::IsLittleEndian()) {
        std::reverse(sendingData, sendingData + count);
    }

    int32 allBytesSent = 0;

    while (allBytesSent < count) {
        if (!socket->Wait(ESocketWaitConditions::WaitForWrite, waitTime)) {
            UE_LOG(LogTemp, Error, TEXT("Server not available for send"));
            delete[] sendingData;
            return false;
        }
        if (!socket->Send(sendingData + allBytesSent, count - allBytesSent, bytesSent)) {
            UE_LOG(LogTemp, Error, TEXT("No answer in sending"));
            delete[] sendingData;
            return false;
        }
        if (bytesSent == 0) {
            return true;
        }

        allBytesSent += bytesSent;
    }

    delete[] sendingData;
    return true;
}

FString UVNCClient::GetConnectionFailedReason(FSocket* socket)
{
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;
    uint32 reasonLength;

    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&reasonLength, 4, bytesRead)) {
        return "";
    }

    uint8* buffer = new uint8[reasonLength];

    if (!UVNCClient::Recv(socket, waitTime, buffer, reasonLength, bytesRead, false)) {
        return "";
    }

    FString reasonMessage = BytesToStringFixed(buffer, reasonLength);

    delete[] buffer;
    return reasonMessage;
}

FString UVNCClient::BytesToStringFixed(const uint8* In, int32 Count)
{
    FString Broken = BytesToString(In, Count);
    FString Fixed;

    for (int i = 0; i < Broken.Len(); i++)
    {
        const TCHAR c = Broken[i] - 1;
        Fixed.AppendChar(c);
    }

    return Fixed;
}

uint8 UVNCClient::ReverseUint8(uint8 b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}
