#include <pch.h>
#include <engine/dxHelper.h>
#include <engine/dxDevice.h>

using namespace engine;
using namespace engine::dxHelper;


int32_t createEmptyTexture2D(int32_t width, int32_t height, DXGI_FORMAT texf, uint32_t bindflags, ID3D11Texture2D** pOutTex)
{
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width;
    descDepth.Height = height;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = texf;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = bindflags;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    return dxDevice::getInstance()->GetD3DDevice()->CreateTexture2D(&descDepth, NULL, pOutTex);
}


