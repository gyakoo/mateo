#include "pch.h"
#include <engine/dxDeviceFactory.h>
#include <engine/dxDevice.h>
#include <engine/dxHelper.h>

using namespace engine;

#define DXDEVFACTORY_EMIT_CREATESHADER(typetoken) {\
    ID3D11##typetoken* pSh = nullptr; \
    ThrowIfFailed(dxDev->Create##typetoken(byteCode.getPtr(), byteCode.getLength(), NULL, &pSh));\
    shader.shader = pSh;}

#define DXDEVFACTORY_EMIT_LOCKUNLOCK(typetoken,listtoken) \
const dx##typetoken& dxDeviceFactory::lock##typetoken(id##typetoken id) const \
{\
    return dxLocker<dx##typetoken>(listtoken).lock(id);\
}\
void dxDeviceFactory::unlock##typetoken(id##typetoken id) const{ }

template<typename CONT>
class dxLocker
{
public:
    dxLocker(const std::vector<CONT>& container)
        : m_cont(container)
    {}

    template<typename IDTYPE>
    const CONT& lock(IDTYPE id)
    {
        HRESULT hr = S_OK;
        if (!id.isValid()) hr = E_FAIL;
        if (id.number() >= m_cont.size()) hr = E_FAIL;
        ThrowIfFailed(hr);
        return m_cont[id.number()];
    }
    const std::vector<CONT>& m_cont;
};

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
    dxRenderTarget renderTarget; ZeroMemory(&renderTarget, sizeof(dxRenderTarget));

    unsigned int bindFlags = D3D11_BIND_RENDER_TARGET;
    if (asTexture)
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;

    // create the texture 2d for the target
    ThrowIfFailed(dxHelper::createEmptyTexture2D(width, height, texf, bindFlags, &renderTarget.pTexture));

    // Create the render target view (uses the texture as render target)
    ThrowIfFailed(pd3dDev->CreateRenderTargetView(renderTarget.pTexture, NULL, &renderTarget.pRTView));

    // Create the shader resource view if it will be used as texture in a shader
    if (asTexture)
        ThrowIfFailed(pd3dDev->CreateShaderResourceView(renderTarget.pTexture, NULL, &renderTarget.pShaderRV));

    if (asDepth)
    {
        ThrowIfFailed(dxHelper::createEmptyTexture2D(width, height, texf, D3D11_BIND_DEPTH_STENCIL, &renderTarget.pRTDSTexture));
        ThrowIfFailed(pd3dDev->CreateDepthStencilView(renderTarget.pRTDSTexture, NULL, &renderTarget.pRTDSView));
    }

    idRenderTarget retId( (uint32_t)m_renderTargets.size());
    m_renderTargets.push_back(renderTarget);
    return retId;
}

idTexture dxDeviceFactory::createTexture(const std::wstring& filename, bool async, uint32_t flags)
{
    idTexture texId((uint32_t)m_textures.size());
    auto readTask = engine::ReadDataAsync(filename);
    readTask.then([this](const std::vector<byte>& fileData)
    {
        dxTexture tex = { nullptr };
        auto dxDev = dxDevice::getInstance()->GetD3DDevice();
        auto hr = DirectX::CreateWICTextureFromMemory((ID3D11Device*)dxDev, (uint8_t*)&fileData[0], fileData.size(), (ID3D11Resource**)&tex.pTexture, &tex.pShaderRV);
        ThrowIfFailed(hr);
        m_textures.push_back(tex);
    });

    if ( !async )
        readTask.wait();
    return texId;
}

idTexture dxDeviceFactory::createTexture(idRenderTarget rt)
{
    auto rtTex = lockRenderTarget(rt);
    
    dxTexture texture = { nullptr };
    if (rtTex.pTexture)
    {
        texture.pTexture = rtTex.pTexture;
        texture.pTexture->AddRef();
    }
    if (rtTex.pShaderRV)
    {
        texture.pShaderRV = rtTex.pShaderRV;
        texture.pShaderRV->AddRef();
    }

    idTexture retId((uint32_t)m_textures.size());
    m_textures.push_back(texture);
    return retId;
}

idTexture dxDeviceFactory::createTexture(DXGI_FORMAT texf, uint8_t* data, uint32_t width, uint32_t height, bool asDynamic)
{
    ThrowIfFailed(asDynamic ? E_FAIL : S_OK); // shout out if dynamic 

    // create texture with this description
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

    // initial data
    int32_t bpp = dxHelper::sizeOfFormatElement(texf);
    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem = (void*)data;
    srdata.SysMemPitch = width * bpp;
    srdata.SysMemSlicePitch = width * height * bpp;

    // texture object with the pointers
    dxTexture texture = { nullptr };

    // create resource and shader view
    auto dxDev = dxDevice::getInstance()->GetD3DDevice();
    ThrowIfFailed(dxDev->CreateTexture2D(&desc, &srdata, &texture.pTexture));
    ThrowIfFailed( dxDev->CreateShaderResourceView(texture.pTexture, NULL, &texture.pShaderRV) );

    // add to list
    idTexture retId((uint32_t)m_textures.size());
    m_textures.push_back(texture);

    return retId;
}

idVertexLayout  dxDeviceFactory::createVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, idByteCode bcId)
{
    ThrowIfFailed(elements.empty() ? E_FAIL : S_OK);
    ThrowIfFailed(!bcId.isValid() ? E_FAIL : S_OK);

    auto dxDev = dxDevice::getInstance()->GetD3DDevice();
    auto byteCode = lockByteCode(bcId);
    dxVertexLayout vertexLayout = { nullptr };

    ThrowIfFailed(dxDev->CreateInputLayout(&elements[0], (UINT)elements.size(), byteCode.getPtr(), (UINT)byteCode.getLength(), &vertexLayout.pInputLayout) );
    unlockByteCode(bcId);

    idVertexLayout retId((uint32_t)m_vertexLayouts.size());
    m_vertexLayouts.push_back(vertexLayout);
    return retId;
}

idByteCode dxDeviceFactory::createShaderByteCode(const std::wstring& filename, bool async)
{
    idByteCode bcId((uint32_t)m_byteCodes.size());
    auto readTask = engine::ReadDataAsync(filename);
    readTask.then([this](const std::vector<byte>& fileData)
    {
        dxByteCode byteCode;
        byteCode.data = std::make_shared<std::vector<byte>>(fileData);
        m_byteCodes.push_back(byteCode);
    });

    if (!async)
        readTask.wait();
    return bcId;
}

idByteCode dxDeviceFactory::createShaderByteCode(const std::vector<byte>& bytecode)
{
    idByteCode bcId((uint32_t)m_byteCodes.size());
    dxByteCode byteCode;
    byteCode.data = std::make_shared<std::vector<byte>>(bytecode);
    m_byteCodes.push_back(byteCode);
    return bcId;
}

idShader dxDeviceFactory::createShader(idByteCode byteCodeId, dxShaderStage stage)
{
    auto dxDev = dxDevice::getInstance()->GetD3DDevice();
    dxShader shader;

    auto byteCode = lockByteCode(byteCodeId);
    switch (stage)
    {
    case SHADER_VERTEX  : DXDEVFACTORY_EMIT_CREATESHADER(VertexShader); break;
    case SHADER_HULL    : DXDEVFACTORY_EMIT_CREATESHADER(HullShader); break;
    case SHADER_DOMAIN  : DXDEVFACTORY_EMIT_CREATESHADER(DomainShader); break;
    case SHADER_GEOMETRY: DXDEVFACTORY_EMIT_CREATESHADER(GeometryShader); break;
    case SHADER_PIXEL   : DXDEVFACTORY_EMIT_CREATESHADER(PixelShader); break;
    case SHADER_COMPUTE : DXDEVFACTORY_EMIT_CREATESHADER(ComputeShader); break;
    default: ThrowIfFailed(E_FAIL); // unrecognized shader stage
    }
    unlockByteCode(byteCodeId);

    idShader shId((uint32_t)m_shaders.size());
    m_shaders.push_back(shader);
    return shId;
}

idMeshBuffer dxDeviceFactory::createMeshBuffer(idVertexLayout vlId, int indexStrideBytes)
{
    idMeshBuffer mbId((uint32_t)m_meshBuffers.size());

    return mbId;
}

DXDEVFACTORY_EMIT_LOCKUNLOCK(VertexLayout, m_vertexLayouts);
DXDEVFACTORY_EMIT_LOCKUNLOCK(ByteCode, m_byteCodes);
DXDEVFACTORY_EMIT_LOCKUNLOCK(Shader, m_shaders);
DXDEVFACTORY_EMIT_LOCKUNLOCK(MeshBuffer, m_meshBuffers);

