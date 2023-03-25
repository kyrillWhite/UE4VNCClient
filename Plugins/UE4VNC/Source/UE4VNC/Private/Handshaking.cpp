#include "Handshaking.h"
#include "VNCClient.h"

Handshaking::Handshaking() : 
    socket(nullptr),
    majorVersion(0),
    minorVersion(0),
    securityType(ESecurityType::Invalid)
{
}

Handshaking::~Handshaking()
{
}

void Handshaking::SetSocket(FSocket* _socket)
{
    socket = _socket;
}

bool Handshaking::ProtocolVersion(int& _majorVersion, int& _minorVersion)
{
    if (socket == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("Socket is nullptr"));
        return false;
    }

    uint32 pendingDataSize = 12;

    uint8 buffer[12];
    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead, bytesSent;

    if (!UVNCClient::Recv(socket, waitTime, buffer, pendingDataSize, bytesRead, false)) {
        return false;
    }

    FString serverMessage = UVNCClient::BytesToStringFixed(buffer, pendingDataSize);
    _majorVersion = FCString::Atoi(*serverMessage.Mid(4, 3).TrimChar('0'));
    _minorVersion = FCString::Atoi(*serverMessage.Mid(8, 3).TrimChar('0'));
    majorVersion = _majorVersion;
    minorVersion = _minorVersion;
    UE_LOG(LogTemp, Log, TEXT("Handshake message: %s"), *serverMessage);
    UE_LOG(LogTemp, Log, TEXT("Protocol version: %i.%i"), majorVersion, minorVersion);

    if (!UVNCClient::Send(socket, waitTime, buffer, pendingDataSize, bytesSent, false)) {
        return false;
    }

    return true;
}

bool Handshaking::Security(ESecurityType& _securityType)
{
    if (socket == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("Socket is nullptr"));
        return false;
    }

    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead, bytesSent;

    if (majorVersion == 3 && minorVersion == 3) {
        uint8 _securityType;

        if (!UVNCClient::Recv(socket, waitTime, &_securityType, 1, bytesRead)) {
            return false;
        }

        if ((ESecurityType)_securityType == ESecurityType::Invalid) {
            FString reasonMessage = UVNCClient::GetConnectionFailedReason(socket);
            UE_LOG(LogTemp, Error, TEXT("Connection failed: %s"), *reasonMessage);
            return false;
        }
    }
    else {
        uint8 numberOfSecurityTypes;
        TArray<bool> securityTypes;
        securityTypes.Init(false, 256);

        if (!UVNCClient::Recv(socket, waitTime, &numberOfSecurityTypes, 1, bytesRead)) {
            return false;
        }

        if (numberOfSecurityTypes == 0) {
            FString reasonMessage = UVNCClient::GetConnectionFailedReason(socket);
            UE_LOG(LogTemp, Error, TEXT("Connection failed: %s"), *reasonMessage);
            return false;
        }

        for (int i = 0; i < numberOfSecurityTypes; i++) {
            uint8 _securityType;

            if (!UVNCClient::Recv(socket, waitTime, &_securityType, 1, bytesRead)) {
                return false;
            }

            securityTypes[_securityType] = true;
        }

        ESecurityType chosenSecurityType = ESecurityType::Invalid;

        if (securityTypes[(uint8)ESecurityType::VNCAuthentication]) {
            chosenSecurityType = ESecurityType::VNCAuthentication;
        }
        else if (securityTypes[(uint8)ESecurityType::None]) {
            chosenSecurityType = ESecurityType::None;
        }

        UE_LOG(LogTemp, Log, TEXT("Chosen security type: %i"), (uint8)chosenSecurityType);

        if (!UVNCClient::Send(socket, waitTime, (uint8*)&chosenSecurityType, 1, bytesSent)) {
            return false;
        }

        _securityType = chosenSecurityType;
        securityType = _securityType;
        UE_LOG(LogTemp, Log, TEXT("Chosen security type: %i"), (uint8)chosenSecurityType);
    }

    return true;
}

bool Handshaking::SecurityTypes(bool& passSecurityResult, FString password)
{
    switch (securityType)
    {
    case ESecurityType::None:
        passSecurityResult = majorVersion == 3 && (minorVersion == 3 || minorVersion == 7);
        break;
    case ESecurityType::VNCAuthentication:
        passSecurityResult = false;

        if (!VNCAuthentication(password)) {
            UE_LOG(LogTemp, Error, TEXT("VNC Authentication error"));
            return false;
        }

        break;
    }
    return true;
}

bool Handshaking::SecurityResult()
{
    if (socket == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("Socket is nullptr"));
        return false;
    }

    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead;
    ESecurityStatus securityStatus;
    uint32 securityStatus32;

    if (!UVNCClient::Recv(socket, waitTime, (uint8*)&securityStatus32, 4, bytesRead)) {
        return false;
    }

    securityStatus = (ESecurityStatus)securityStatus32;

    if (securityStatus == ESecurityStatus::Failed) {
        UE_LOG(LogTemp, Error, TEXT("Security status: failed"));

        if (majorVersion == 3 && minorVersion == 8) {
            FString reasonMessage = UVNCClient::GetConnectionFailedReason(socket);
            UE_LOG(LogTemp, Error, TEXT("Connection failed: %s"), *reasonMessage);
        }

        return false;
    }

    return true;
}

bool Handshaking::VNCAuthentication(FString password)
{
    if (socket == nullptr) {
        UE_LOG(LogTemp, Error, TEXT("Socket is nullptr"));
        return false;
    }

    FTimespan waitTime = FTimespan::FromSeconds(10);
    int bytesRead, bytesSent;
    TArray<uint8> challengeResponse;

    uint8 passwordUint8[8];

    for (int i = 0; i < 8; i++) {
        passwordUint8[i] = 0;
    }

    if (password.Len() > 8) {
        password = password.Left(8);
    }

    StringToBytes(password, passwordUint8, 8);

    for (int i = 0; i < 8; i++) {
        passwordUint8[i] = UVNCClient::ReverseUint8(passwordUint8[i] + 1);
    }

    TArray<uint8> key(passwordUint8, 8);

    DES des(key);

    for (int i = 0; i < 2; i++) {
        uint8 challenge[8];
        
        if (!UVNCClient::Recv(socket, waitTime, challenge, 8, bytesRead, false)) {
            return false;
        }

        TArray<uint8> text(challenge, 8);
        challengeResponse.Append(des.Encrypt(text));
    }

    if (!UVNCClient::Send(socket, waitTime, challengeResponse.GetData(), 16, bytesSent, false)) {
        return false;
    }

    return true;
}
