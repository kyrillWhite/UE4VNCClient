#include "Encoding.h"

Encoding::Encoding() :
    encodingType(EEncodingType::Raw)
{
}

Encoding::Encoding(EEncodingType _encodingType) :
    encodingType(_encodingType)
{
}

Encoding::~Encoding()
{
}

EEncodingType Encoding::GetEncodingType()
{
    return encodingType;
}
