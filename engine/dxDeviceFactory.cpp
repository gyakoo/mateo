#include "pch.h"
#include <engine/dxDeviceFactory.h>
#include <engine/dxDevice.h>
#include <engine/dxHelper.h>

using namespace engine;

dxDeviceFactory::dxDeviceFactory()
{
}

dxDeviceFactory::~dxDeviceFactory()
{
}

void dxDeviceFactory::releaseResources()
{
}


idRenderTarget  dxDeviceFactory::createRenderTarget(int32_t width, int32_t height, DXGI_FORMAT texf, bool asTexture, bool asDepth)
{
    ID3D11Device* pd3dDev = dxDevice::getInstance()->GetD3DDevice();    
    dxRenderTarget renderTarget = { nullptr };

    unsigned int bindFlags = D3D11_BIND_RENDER_TARGET;
    if (asTexture)
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;

    // create the texture 2d for the target
    ThrowIfFailed(dxHelper::createEmptyTexture2D(width, height, texf, bindFlags, &renderTarget.pRTTexture));

    // Create the render target view (uses the texture as render target)
    ThrowIfFailed(pd3dDev->CreateRenderTargetView(renderTarget.pRTTexture, NULL, &renderTarget.pRTView));

    // Create the shader resource view if it will be used as texture in a shader
    if (asTexture)
        ThrowIfFailed(pd3dDev->CreateShaderResourceView(renderTarget.pRTTexture, NULL, &renderTarget.pSRView));

    if (asDepth)
    {
        ThrowIfFailed(dxHelper::createEmptyTexture2D(width, height, texf, D3D11_BIND_DEPTH_STENCIL, &renderTarget.pRTDSTexture));
        ThrowIfFailed(pd3dDev->CreateDepthStencilView(renderTarget.pRTDSTexture, NULL, &renderTarget.pRTDSView));
    }

    idRenderTarget retId( (uint32_t)m_renderTargets.size());
    m_renderTargets.push_back(renderTarget);
    return retId;
}

idTexture dxDeviceFactory::createTexture(const char* filename, uint32_t flags)
{

    return idTexture::INVALID();
}

idTexture dxDeviceFactory::createTexture(idRenderTarget rt)
{

    return idTexture::INVALID();
}

idTexture dxDeviceFactory::createTexture(DXGI_FORMAT texf, uint8_t* data, uint32_t width, uint32_t height, bool asDynamic)
{
    ThrowIfFailed(asDynamic ? E_FAIL : S_OK); // shout out if dynamic 

    D3D11_TEXTURE2D_DESC desc = { 0 };
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = texf;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem = (void*)data;
    srdata.SysMemPitch = width * gyGetComponentFormatSize(texf);
    srdata.SysMemSlicePitch = width * height * gyGetComponentFormatSize(texf);

    auto dxDev = dxDevice::getInstance()->GetD3DDevice();
    dxDev->CreateTexture2D()

        return idTexture::INVALID();
}