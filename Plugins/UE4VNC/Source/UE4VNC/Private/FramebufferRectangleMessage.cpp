#include "FramebufferRectangleMessage.h"

FramebufferRectangleMessage::FramebufferRectangleMessage() :
    xPosition(0),
    yPosition(0),
    width(0),
    height(0),
    encoding(nullptr)
{
}

FramebufferRectangleMessage::FramebufferRectangleMessage(
    uint16 _xPosition,
    uint16 _yPosition,
    uint16 _width,
    uint16 _height,
    std::shared_ptr<Encoding> _encoding) :

    xPosition(_xPosition),
    yPosition(_yPosition),
    width(_width),
    height(_height),
    encoding(_encoding)
{
}

FramebufferRectangleMessage::~FramebufferRectangleMessage()
{
}

uint16 FramebufferRectangleMessage::GetXPosition()
{
    return xPosition;
}

uint16 FramebufferRectangleMessage::GetYPosition()
{
    return yPosition;
}

uint16 FramebufferRectangleMessage::GetWidth()
{
    return width;
}

uint16 FramebufferRectangleMessage::GetHeight()
{
    return height;
}

std::shared_ptr<Encoding> FramebufferRectangleMessage::GetEncoding()
{
    return encoding;
}

void FramebufferRectangleMessage::DrawToTexture(
    UTexture2D* texture,
    SPixelFormat pixelFormat,
    uint16 framebufferWidth,
    uint16 framebufferHeight)
{
    encoding->DrawToTexture(
        texture,
        xPosition,
        yPosition,
        width,
        height,
        framebufferWidth,
        framebufferHeight,
        pixelFormat
    );
}

void FramebufferRectangleMessage::Abstract()
{
}
