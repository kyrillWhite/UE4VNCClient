#include "VideoDecoder.h"

VideoDecoder::VideoDecoder()
{
}

VideoDecoder::~VideoDecoder()
{
    CleanUp();
}


bool VideoDecoder::Initialise(
    int sourceWith,
    int sourceHeight,
    AVCodecID _codecID,
    AVPixelFormat _sourcePixelFormat,
    AVHWDeviceType hwDeviceType
)
{
    codecID = _codecID;
    if (codecID == AVCodecID::AV_CODEC_ID_RAWVIDEO) {
        rawBufferSize = sourceWith * sourceHeight * 4;
        dataRawBufferSize = 0;
        return true;
    }

    frame = av_frame_alloc();
    if (!frame) {
        UE_LOG(LogTemp, Error, TEXT("Could not allocate video frame"));
        return false;
    }

    codec = avcodec_find_decoder(codecID);

    if (!codec) {
        UE_LOG(LogTemp, Error, TEXT("Could not found codec by given id"));
        return false;
    }

    parser = av_parser_init(codec->id);
    if (!parser) {
        UE_LOG(LogTemp, Error, TEXT("Could not allocate parser context"));
        return false;
    }

    context = avcodec_alloc_context3(codec);
    if (!context) {
        UE_LOG(LogTemp, Error, TEXT("Could not allocate avcodec context"));
        return false;
    }

    sourcePixelFormat = _sourcePixelFormat;
    context->width = sourceWith;
    context->height = sourceHeight;
    context->pix_fmt = sourcePixelFormat;
    context->delay = 0;
    context->flags |= AV_CODEC_FLAG_LOW_DELAY;
    context->flags2 |= AV_CODEC_FLAG2_NO_OUTPUT | AV_CODEC_FLAG2_FAST;
    context->rc_buffer_size = 0;

    if (hwDeviceType) {
        hw_device_ctx = NULL;
        av_hwdevice_ctx_create(&hw_device_ctx, hwDeviceType, NULL, NULL, 0);
        context->hw_device_ctx = av_buffer_ref(hw_device_ctx);
    }

    if (avcodec_open2(context, codec, nullptr) < 0) {
        UE_LOG(LogTemp, Error, TEXT("Could not initialize avcodec context"));
        return false;
    }

    return true;
}

void VideoDecoder::CleanUp()
{
    if (codecID != AVCodecID::AV_CODEC_ID_RAWVIDEO) {
        if (frame) {
            av_frame_free(&frame);
        }
        if (context) {
            avcodec_free_context(&context);
        }
        if (hw_device_ctx) {
            av_buffer_unref(&hw_device_ctx);
        }
        if (parser) {
            av_parser_close(parser);
        }
    }
}

void VideoDecoder::YUVFrameToRGBData(const AVFrame& _frame, uint8_t* data) {
    auto _swsContext = sws_getContext(
        _frame.width, _frame.height,
        AVPixelFormat(_frame.format),
        _frame.width, _frame.height,
        AV_PIX_FMT_RGBA,
        SWS_BICUBIC, nullptr, nullptr, nullptr);

    int width = _frame.width;
    int height = _frame.height;

    uint8_t* outData[1] = { data };
    int outLineSize[1] = { 4 * _frame.width };
    sws_scale(_swsContext, _frame.data, _frame.linesize, 0, _frame.height, outData, outLineSize);
    sws_freeContext(_swsContext);
}

int VideoDecoder::Decode(AVPacket* packet, uint8* textureData) {
    int ret = 0;

    ret = avcodec_send_packet(context, packet);
    if (ret < 0) {
        UE_LOG(LogTemp, Error, TEXT("avcodec_send_packet: %d"), ret);
        return 0;
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(context, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return 0;
        }
        else if (ret < 0) {
            UE_LOG(LogTemp, Error, TEXT("avcodec_receive_frame: %d"), ret);
            return 0;
        }

        if (
            frame->format == AV_PIX_FMT_DXVA2_VLD ||
            frame->format == AV_PIX_FMT_VAAPI ||
            frame->format == AV_PIX_FMT_VDPAU ||
            frame->format == AV_PIX_FMT_QSV ||
            frame->format == AV_PIX_FMT_MMAL ||
            frame->format == AV_PIX_FMT_D3D11VA_VLD ||
            frame->format == AV_PIX_FMT_CUDA ||
            frame->format == AV_PIX_FMT_D3D11 ||
            frame->format == AV_PIX_FMT_OPENCL ||
            frame->format == AV_PIX_FMT_VULKAN
        ) {
            /* retrieve data from GPU to CPU */
            AVFrame* tempFrame = av_frame_alloc();

            tempFrame->width = context->width;
            tempFrame->height = context->height;
            int res = av_hwframe_transfer_data(tempFrame, frame, 0);
            av_frame_free(&frame);
            frame = tempFrame;
        }
        YUVFrameToRGBData(*frame, textureData);
    }

    return 1;
}

int VideoDecoder::ProcessPacket(std::shared_ptr<MediaPacket> _packet, uint8* textureData)
{
    uint8* in_data = reinterpret_cast<unsigned char*>(_packet->payload.data());
    int in_len = _packet->payload.size();

    AVPacket* packet = av_packet_alloc();
    av_init_packet(packet);

    uint8* data = in_data;
    int data_size = in_len;
    int ret = 0;
    int gotNewFrame = 0;

    if (codecID == AV_CODEC_ID_RAWVIDEO) {
        while (data_size > 0) {
            ret = min(data_size, rawBufferSize - dataRawBufferSize);
            memcpy(textureData + dataRawBufferSize, data, ret);
            dataRawBufferSize += ret;
            if (dataRawBufferSize == rawBufferSize) {
                dataRawBufferSize = 0;
                gotNewFrame = 1;
            }
            data += ret;
            data_size -= ret;
        }
        return gotNewFrame;
    }

    while (data_size > 0) {
        ret = av_parser_parse2(parser, context, &packet->data, &packet->size,
            data, data_size, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
        if (ret < 0) {
            fprintf(stderr, "Error while parsing\n");
            return 0;
        }
        data += ret;
        data_size -= ret;

        if (packet->size) {
            Decode(packet, textureData);
            gotNewFrame = 1;
        }
    }
    av_packet_free(&packet);

    return gotNewFrame;
}