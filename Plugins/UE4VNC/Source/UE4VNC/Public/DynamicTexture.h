#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RHI.h"

class UE4VNC_API DynamicTexture
{
private:
	UTexture2D* Texture;

	int32 TextureWidth;
	int32 TextureHeight;

	FLinearColor ClearColor;
	TUniquePtr<FUpdateTextureRegion2D> UpdateTextureRegionProxy;

public:
	TUniquePtr<uint8[]> PixelBuffer;

	void Initialize(int32 InWidth, int32 InHeight, FLinearColor InClearColor, TextureFilter FilterMethod = TextureFilter::TF_Bilinear);
	UTexture2D* GetTextureResource();
	void UpdateTexture();

	int32 GetWidth();
	int32 GetHeight();
};