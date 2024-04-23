// DynamicTexture

#include "DynamicTexture.h"

#define DYNAMIC_TEXTURE_BYTES_PER_PIXEL 4

void DynamicTexture::Initialize(int32 InWidth, int32 InHeight, FLinearColor InClearColor, TextureFilter FilterMethod)
{
    TextureWidth = InWidth;
    TextureHeight = InHeight;
    ClearColor = InClearColor;

    Texture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, EPixelFormat::PF_R8G8B8A8);
    Texture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
    Texture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
    Texture->SRGB = 1;
    Texture->Filter = FilterMethod;
    Texture->UpdateResource();

    UpdateTextureRegionProxy = MakeUnique<FUpdateTextureRegion2D>(0, 0, 0, 0, TextureWidth, TextureHeight);

    SIZE_T BufferSize = TextureWidth * TextureHeight * DYNAMIC_TEXTURE_BYTES_PER_PIXEL;
    PixelBuffer = MakeUnique<uint8[]>(BufferSize);
}

UTexture2D* DynamicTexture::GetTextureResource()
{
    return Texture;
}

void DynamicTexture::UpdateTexture()
{
    if (UpdateTextureRegionProxy.IsValid() && Texture)
    {
        Texture->UpdateTextureRegions(
            0,
            1,
            UpdateTextureRegionProxy.Get(),
            TextureWidth * DYNAMIC_TEXTURE_BYTES_PER_PIXEL,
            DYNAMIC_TEXTURE_BYTES_PER_PIXEL,
            PixelBuffer.Get()
        );
    }
}

int32 DynamicTexture::GetWidth()
{
    return TextureWidth;
}

int32 DynamicTexture::GetHeight()
{
    return TextureHeight;
}