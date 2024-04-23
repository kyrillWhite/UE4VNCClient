#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#define REQUIRE_CXX11 1
#define SAVE_IMAGES 0

#include "SRTClient.h"
#include <cctype>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <cstring>
#include <csignal>
#include <chrono>
#include <thread>
#include <list>
#include <chrono>
#include "srt/srt.h"
#include <HighResScreenshot.h>
#include <ImageWriteQueue/Public/ImageWriteBlueprintLibrary.h>

using namespace std;


USRTClient::USRTClient() :
    source("srt://:9000"),
    host(""),
    port(0),
    videoDecoder(std::make_shared<VideoDecoder>()),
    buffering(10),
    pollid(-1),
    isClosed(false)
{
}

USRTClient::~USRTClient()
{
    Close();
}

bool USRTClient::Initialise(
    UStaticMeshComponent* mesh,
    FString _host,
    int _port,
    size_t _buffering,
    int sourceWidth,
    int sourceHeight,
    EAVCodecID codecID,
    EAVPixelFormat sourcePixelFormat,
    EAVHWDeviceType hwDeviceType
)
{
    texture = std::make_unique<DynamicTexture>();
    texture->Initialize(sourceWidth, sourceHeight, FLinearColor::White);

    auto material = mesh->GetMaterial(0);
    dynamicMaterial = UMaterialInstanceDynamic::Create(material, this);
    mesh->SetMaterial(0, dynamicMaterial);
    dynamicMaterial->SetTextureParameterValue(TEXT("Texture"), texture->GetTextureResource());

    if (!videoDecoder->Initialise(sourceWidth, sourceHeight, AVCodecID(codecID), AVPixelFormat(sourcePixelFormat), AVHWDeviceType(hwDeviceType))) {
        UE_LOG(LogTemp, Error, TEXT("Video Decoder initialisation error"));
        return false;
    }

    srt_startup();
    pollid = srt_epoll_create();
    if (pollid < 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Can't initialize epoll"));
        return false;
    }

    host = _host;
    port = _port;
    buffering = _buffering;

    if (host != "") {
        FIPv4Address ipAddress;

        if (!FIPv4Address::Parse(host, ipAddress)) {
            UE_LOG(LogTemp, Error, TEXT("Invalid host or port. host: %s, port: %i"), *host, port);
            return false;
        }

        FIPv4Endpoint endpoint(ipAddress, (uint16)port);
        source = FString("srt://") + endpoint.ToString();
    }
    else {
        source = FString("srt://:") + FString::FromInt((uint16)port);
    }

    return true;
}

void USRTClient::Listen()
{
    int timeout = 0;
    int timeoutMode = 0;
    int chunkSize = -1;
    bool quiet = false;
    FString logfile;
    int bwReport = 0;
    bool srctime = false;
    int statsReport = 0;
    FString statsOut;
    bool autoReconnect = true;
    bool fullStats = false;
    std::unique_ptr<Source> src;
    bool srcConnected = false;

    size_t packetsGot = 0;
    size_t receivedBytes = 0;
    size_t wroteBytes = 0;
    size_t lostBytes = 0;
    size_t lastReportedtLostBytes = 0;
    std::time_t writeErrorLogTimer(std::time(nullptr));

    try {
        while (!isClosed)
        {
            if (!src.get())
            {
                src = Source::Create(TCHAR_TO_UTF8(*source));
                if (!src.get())
                {
                    UE_LOG(LogTemp, Error, TEXT("Unsupported source type"));
                    return;
                }
                int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
                switch (src->uri.type())
                {
                case UriParser::SRT:
                    if (srt_epoll_add_usock(pollid,
                        src->GetSRTSocket(), &events))
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to add SRT source to poll, %s"), src->GetSRTSocket());
                        return;
                    }
                    break;
                case UriParser::UDP:
                    if (srt_epoll_add_ssock(pollid,
                        src->GetSysSocket(), &events))
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to add UDP source to poll, %s"), src->GetSysSocket());
                        return;
                    }
                    break;
                case UriParser::FILE:
                    if (srt_epoll_add_ssock(pollid,
                        src->GetSysSocket(), &events))
                    {
                        UE_LOG(LogTemp, Error, TEXT("Failed to add FILE source to poll, %s"), src->GetSysSocket());
                        return;
                    }
                    break;
                default:
                    break;
                }

                receivedBytes = 0;
            }

            int srtrfdslen = 2;
            SRTSOCKET srtrwfds[2] = { SRT_INVALID_SOCK, SRT_INVALID_SOCK };
            SRTSOCKET emptyAddres;
            int sysrfdslen = 1;
            SYSSOCKET sysrfds[1];
            if (srt_epoll_wait(pollid,
                &srtrwfds[0], &srtrfdslen, &emptyAddres, 0,
                100,
                &sysrfds[0], &sysrfdslen, 0, 0) >= 0)
            {
                bool doabort = false;
                for (size_t i = 0; i < sizeof(srtrwfds) / sizeof(SRTSOCKET); i++)
                {
                    SRTSOCKET s = srtrwfds[i];
                    if (s == SRT_INVALID_SOCK)
                        continue;

                    // Remove duplicated sockets
                    for (size_t j = i + 1; j < sizeof(srtrwfds) / sizeof(SRTSOCKET); j++)
                    {
                        const SRTSOCKET next_s = srtrwfds[j];
                        if (next_s == s)
                            srtrwfds[j] = SRT_INVALID_SOCK;
                    }

                    bool issource = false;
                    if (src && src->GetSRTSocket() == s)
                    {
                        issource = true;
                    }

                    FString dirstring = (issource) ? "source" : "target";

                    SRT_SOCKSTATUS status = srt_getsockstate(s);
                    switch (status)
                    {
                    case SRTS_LISTENING:
                    {
                        const bool res = src->AcceptNewClient();
                        if (!res)
                        {
                            UE_LOG(LogTemp, Error, TEXT("Failed to accept SRT connection"));
                            doabort = true;
                            break;
                        }

                        srt_epoll_remove_usock(pollid, s);

                        SRTSOCKET ns = src->GetSRTSocket();
                        int events = SRT_EPOLL_IN | SRT_EPOLL_ERR;
                        if (srt_epoll_add_usock(pollid, ns, &events))
                        {
                            UE_LOG(LogTemp, Error, TEXT("Failed to add SRT client to poll, %s"), ns);
                            doabort = true;
                        }
                        else
                        {
                            if (!quiet)
                            {
                                UE_LOG(LogTemp, Error, TEXT("Accepted SRT %s connection"), *dirstring);
                            }
#ifndef _WIN32
                            if (timeoutMode == 1 && timeout > 0)
                            {
                                if (!quiet)
                                    UE_LOG(LogTemp, Error, TEXT("TIMEOUT: cancel"));
                                alarm(0);
                            }
#endif
                            if (issource)
                                srcConnected = true;
                            //else
                            //    tarConnected = true;
                        }
                    }
                    break;
                    case SRTS_BROKEN:
                    case SRTS_NONEXIST:
                    case SRTS_CLOSED:
                    {
                        if (issource)
                        {
                            if (srcConnected)
                            {
                                if (!quiet)
                                {
                                    UE_LOG(LogTemp, Error, TEXT("SRT source disconnected"));
                                }
                                srcConnected = false;
                            }
                        }

                        if (!autoReconnect)
                        {
                            doabort = true;
                        }
                        else
                        {
                            // force re-connection
                            srt_epoll_remove_usock(pollid, s);
                            src.reset();

#ifndef _WIN32
                            if (timeoutMode == 1 && timeout > 0)
                            {
                                if (!quiet)
                                    UE_LOG(LogTemp, Error, TEXT("TIMEOUT: will interrupt after %ds"), timeout);
                                alarm(timeout);
                            }
#endif
                        }
                    }
                    break;
                    case SRTS_CONNECTED:
                    {
                        if (issource)
                        {
                            if (!srcConnected)
                            {
                                if (!quiet)
                                    UE_LOG(LogTemp, Error, TEXT("SRT source connected"));
                                srcConnected = true;
                            }
                        }
                    }

                    default:
                    {
                        // No-Op
                    }
                    break;
                    }
                }

                if (doabort)
                {
                    break;
                }

                // read a few chunks at a time in attempt to deplete
                // read buffers as much as possible on each read event
                // note that this implies live streams and does not
                // work for cached/file sources
                std::list<std::shared_ptr<MediaPacket>> dataqueue;
                if (src.get() && src->IsOpen() && (srtrfdslen || sysrfdslen))
                {
                    while (dataqueue.size() < buffering)
                    {
                        std::shared_ptr<MediaPacket> pkt(new MediaPacket(transmit_chunk_size));
                        const int res = src->Read(transmit_chunk_size, *pkt);

                        if (res == SRT_ERROR && src->uri.type() == UriParser::SRT)
                        {
                            if (srt_getlasterror(NULL) == SRT_EASYNCRCV)
                                break;

                            throw std::runtime_error(
                                string("error: recvmsg: ") + string(srt_getlasterror_str())
                            );
                        }

                        if (res == 0 || pkt->payload.empty())
                        {
                            break;
                        }

                        dataqueue.push_back(pkt);
                        receivedBytes += pkt->payload.size();
                    }
                }

                // if there is no target, let the received data be lost
                while (!dataqueue.empty())
                {
                    std::shared_ptr<MediaPacket> pkt = dataqueue.front();

                    int processRes = videoDecoder->ProcessPacket(pkt, texture->PixelBuffer.Get());
                    if (processRes == 1) {
                        Async(EAsyncExecution::LargeThreadPool, [&]() {
                           UpdateTexture();
                        });
                    }

                    wroteBytes += pkt->payload.size();
                    dataqueue.pop_front();
                    packetsGot++;
                }

                if (!quiet)
                {
                    std::time_t now(std::time(nullptr));
                    if (std::difftime(now, writeErrorLogTimer) >= 5.0)
                    {
                        UE_LOG(LogTemp, Error, TEXT("%d bytes lost, %d bytes sent, %d bytes received, %d packets got"), lostBytes, wroteBytes, receivedBytes, packetsGot);
                        writeErrorLogTimer = now;
                        lastReportedtLostBytes = lostBytes;
                    }
                }
            }
        }
    }
    catch (std::exception& x)
    {
        UE_LOG(LogTemp, Error, TEXT("ERROR: %s"), *FString(x.what()));
        return;
    }
    srt_cleanup();
    videoDecoder->CleanUp();
}

UTexture2D* USRTClient::CreateTexture(int32 inSizeX, int32 inSizeY)
{
    EPixelFormat _ePixelFormat = EPixelFormat::PF_R8G8B8A8;
    auto _texture = UTexture2D::CreateTransient(inSizeX, inSizeY, _ePixelFormat, "Dynamic Texture");
    #undef UpdateResource
    _texture->UpdateResource();
    return _texture;
}

UTexture2D* USRTClient::GetTexture()
{
    return texture->GetTextureResource();
}

void USRTClient::UpdateTexture()
{
    uint8* ptr = texture->PixelBuffer.Get();
    texture->UpdateTexture();
#if SAVE_IMAGES
    uint64 ms = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    FString testNumber = "18";
    int isQuality = 1;
    FString testType = isQuality ? "quality" : "latency";
    SaveTexture(
        "C:\\Users\\kirill\\Desktop\\NW\\Quality and Latency Tests\\" +
        testNumber + "\\" + testType + "\\" +
        FString::Printf(TEXT("%llu"), ms)
    );
#endif
    dynamicMaterial->SetTextureParameterValue(TEXT("Texture"), texture->GetTextureResource());
}

void USRTClient::SaveTexture(FString filename)
{
    FImageWriteOptions options;
    options.bAsync = true;
    options.bOverwriteFile = true;
    options.CompressionQuality = 1;
    options.Format = EDesiredImageFormat::PNG;
    UImageWriteBlueprintLibrary::ExportToDisk(texture->GetTextureResource(), filename, options);
}

void USRTClient::Close()
{
    isClosed = true;
}
