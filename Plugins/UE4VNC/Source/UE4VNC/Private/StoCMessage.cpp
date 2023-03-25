#include "StoCMessage.h"

StoCMessage::StoCMessage() :
    messageType(EStoCMessageType::FramebufferUpdate)
{
}

StoCMessage::StoCMessage(EStoCMessageType _messageType) :
    messageType(_messageType)
{
}

EStoCMessageType StoCMessage::GetMessageType()
{
    return messageType;
}

StoCMessage::~StoCMessage()
{
}
