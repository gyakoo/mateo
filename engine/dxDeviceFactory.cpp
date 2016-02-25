#include "pch.h"
#include <engine/dxDeviceFactory.h>
#include <engine/dxDevice.h>
#include <engine/dxHelper.h>

using namespace Engine;
using namespace Concurrency;
using namespace DirectX;

#define DXDEVFACTORY_EMIT_CREATESHADER(typetoken) \
{\
    ID3D11##typetoken* pSh = nullptr; \
    ThrowIfFailed(dxDev->Create##typetoken(byteCode.getPtr(), byteCode.getLength(), NULL, &pSh));\
    shader.shader = pSh;\
}

#define DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(typetoken,listtoken) \
const Dx##typetoken& DxDeviceFactory::lock##typetoken(Id##typetoken id) const \
{\
    return DxLocker<Dx##typetoken>(listtoken).lock(id);\
}\
void DxDeviceFactory::unlock##typetoken(Id##typetoken id) const\
{\
	DxLocker<Dx##typetoken>(listtoken).unlock(id);\
}

#define DXDEVFACTORY_EMIT_CREATECOMMON(typetoken, count, ...) \
{\
	typedef ID3D11##typetoken* (_cdecl CommonStates::*FactoryMethod)() const;\
	FactoryMethod facMethods[count] = {__VA_ARGS__};\
	Dx##typetoken state; \
	for (int i = 0; i < count; ++i)\
	{\
		state.state = DXSTATE_LOADED;\
		state.stateObj = (commonStates.*facMethods[i])();\
		m_##typetoken##s.push_back(state);\
		m_##typetoken##sCommon[i] = Id##typetoken(i);\
	}\
}

#define DXDEVFACTORY_EMIT_STATECREATEFINAL(typetoken, descptr) \
	Id##typetoken id((uint32_t)m_##typetoken##s.size());\
	Dx##typetoken state; \
	state.state = DXSTATE_LOADED; \
	ThrowIfFailed(DxDevice::getInstance()->GetD3DDevice()->Create##typetoken(descptr, &state.stateObj)); \
	m_##typetoken##s.push_back(state);\
	return id;

#define DXDEVFACTORY_EMIT_FILLBUFFER(typetoken) 	\
	auto meshBuffer = lockMeshBuffer(mbId);\
	ThrowIfAssert(meshBuffer.##typetoken##StrideBytes != 0);\
	D3D11_BOX updateBox = { 0, 0, 0, meshBuffer.##typetoken##StrideBytes*##typetoken##Count, 1, 1 };\
	DxDevice::getInstance()->GetD3DDeviceContext()->UpdateSubresource(meshBuffer.##typetoken##Buffer, 0, &updateBox, typetoken##Data, 0, 0);\
	unlockMeshBuffer(mbId)

#define DXDEVFACTORY_EMIT_MAPBUFFER(typetoken) \
	auto meshBuffer = lockMeshBuffer(mbId);\
	typetoken##Count = meshBuffer.##typetoken##Count;\
	typetoken##StrideBytes = meshBuffer.##typetoken##StrideBytes;\
	D3D11_MAPPED_SUBRESOURCE mapped;\
	ThrowIfFailed(DxDevice::getInstance()->GetD3DDeviceContext()->Map(meshBuffer.##typetoken##Buffer, 0, mapType, 0, &mapped));\
	*outData = mapped.pData;\
	unlockMeshBuffer(mbId)

template<typename T>
std::size_t makeHash(const T& v)
{
	return std::hash<T>()(v);
}

template<typename T, typename... Args>
std::size_t makeHash(T first, Args ... args)
{
	//auto n = sizeof...(Args);
	std::size_t h = std::hash<T>()(first) ^ (makeHash(args...) << 1);
	return h;
}


template<typename CONT>
class DxLocker
{
public:
    DxLocker(const std::vector<CONT>& container)
        : m_cont(container)
    {}

    template<typename IDTYPE>
    const CONT& lock(IDTYPE id)
    {
		ThrowIfAssert(id.IsValid(), L"id not valid");
		ThrowIfAssert(id.Number() < m_cont.size(), L"id out of bounds");
        
		const CONT& retRes = m_cont[id.Number()];
		ThrowIfAssert(retRes.lockCount == 0, L"Locked already");
		++retRes.lockCount;
		return retRes;
    }

	template<typename IDTYPE>
	void unlock(IDTYPE id)
	{
		ThrowIfAssert(id.IsValid(), L"id not valid");
		ThrowIfAssert(id.Number() < m_cont.size(), L"id out of bounds");

		const CONT& retRes = m_cont[id.Number()];
		--retRes.lockCount;
		ThrowIfAssert(retRes.lockCount >= 0, L"Too many unlocks");
	}

    const std::vector<CONT>& m_cont;
};

DxDeviceFactory::DxDeviceFactory()
{
	createCommonStates();
}

DxDeviceFactory::~DxDeviceFactory()
{
	releaseCommonStates();
    ThrowIfAssert(m_renderTargets.empty());
    ThrowIfAssert(m_textures.empty());
    ThrowIfAssert(m_vertexLayouts.empty());
    ThrowIfAssert(m_byteCodes.empty());
    ThrowIfAssert(m_shaders.empty());
    ThrowIfAssert(m_meshBuffers.empty());
    ThrowIfAssert(m_BlendStates.empty());
    ThrowIfAssert(m_DepthStencilStates.empty());
    ThrowIfAssert(m_RasterizerStates.empty());
    ThrowIfAssert(m_SamplerStates.empty());
}

void DxDeviceFactory::releaseResources()
{
}

IdRenderTarget  DxDeviceFactory::createRenderTarget(int32_t width, int32_t height, DXGI_FORMAT texf, bool asTexture, bool asDepth)
{
    ID3D11Device* pd3dDev = DxDevice::getInstance()->GetD3DDevice();    
    DxRenderTarget renderTarget; ZeroMemory(&renderTarget, sizeof(DxRenderTarget));

    unsigned int bindFlags = D3D11_BIND_RENDER_TARGET;
    if (asTexture)
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;

    // create the texture 2d for the target
    ThrowIfFailed(DxHelper::CreateEmptyTexture2D(width, height, texf, bindFlags, &renderTarget.texture));

    // Create the render target view (uses the texture as render target)
    ThrowIfFailed(pd3dDev->CreateRenderTargetView(renderTarget.texture, NULL, &renderTarget.renderTargetView));

    // Create the shader resource view if it will be used as texture in a shader
    if (asTexture)
        ThrowIfFailed(pd3dDev->CreateShaderResourceView(renderTarget.texture, NULL, &renderTarget.textureShaderResourceView));

    if (asDepth)
    {
        ThrowIfFailed(DxHelper::CreateEmptyTexture2D(width, height, texf, D3D11_BIND_DEPTH_STENCIL, &renderTarget.renderTargetDepthStencilTexture));
        ThrowIfFailed(pd3dDev->CreateDepthStencilView(renderTarget.renderTargetDepthStencilTexture, NULL, &renderTarget.renderTargetDepthStencilView));
    }

    IdRenderTarget retId( (uint32_t)m_renderTargets.size());
    m_renderTargets.push_back(renderTarget);
	renderTarget.state = DXSTATE_LOADED;
    return retId;
}

task<IdTexture> DxDeviceFactory::createTexture(const std::wstring& filename, uint32_t flags)
{
    return create_task(Engine::ReadDataAsync(filename))
        .then([this](const std::vector<byte>& fileData) -> IdTexture
    {
        IdTexture texId((uint32_t)m_textures.size());
        DxTexture tex;
        tex.state = DXSTATE_LOADED;
        auto dxDev = DxDevice::getInstance()->GetD3DDevice();
        ThrowIfFailed(CreateWICTextureFromMemory((ID3D11Device*)dxDev, (uint8_t*)&fileData[0], fileData.size(),
            (ID3D11Resource**)&tex.texture, &tex.textureShaderResourceView));
        m_textures.push_back(tex);
        return texId;
    });	
}

IdTexture DxDeviceFactory::createTexture(IdRenderTarget rt)
{
    auto rtTex = lockRenderTarget(rt);
    
	DxTexture texture;
	texture.state = DXSTATE_LOADED;
    if (rtTex.texture)
    {
        texture.texture = rtTex.texture;
        texture.texture->AddRef();
    }
    if (rtTex.textureShaderResourceView)
    {
        texture.textureShaderResourceView = rtTex.textureShaderResourceView;
        texture.textureShaderResourceView->AddRef();
    }
	
    IdTexture retId((uint32_t)m_textures.size());
    m_textures.push_back(texture);
    return retId;
}

IdTexture DxDeviceFactory::createTexture(DXGI_FORMAT texf, uint8_t* data, uint32_t width, uint32_t height, bool asDynamic)
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
    int32_t bpp = DxHelper::SizeOfFormatElement(texf);
    D3D11_SUBRESOURCE_DATA srdata;
    srdata.pSysMem = (void*)data;
    srdata.SysMemPitch = width * bpp;
    srdata.SysMemSlicePitch = width * height * bpp;

    // texture object with the pointers
	DxTexture texture;
	texture.state = DXSTATE_LOADED;

    // create resource and shader view
    auto dxDev = DxDevice::getInstance()->GetD3DDevice();
    ThrowIfFailed(dxDev->CreateTexture2D(&desc, &srdata, &texture.texture));
    ThrowIfFailed( dxDev->CreateShaderResourceView(texture.texture, NULL, &texture.textureShaderResourceView) );

    // add to list
    IdTexture retId((uint32_t)m_textures.size());
    m_textures.push_back(texture);

    return retId;
}

IdVertexLayout  DxDeviceFactory::createVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, IdByteCode bcId)
{
    ThrowIfFailed(elements.empty() ? E_FAIL : S_OK);
    ThrowIfFailed(!bcId.IsValid() ? E_FAIL : S_OK);

    auto dxDev = DxDevice::getInstance()->GetD3DDevice();
    auto byteCode = lockByteCode(bcId);
	DxVertexLayout vertexLayout;
	vertexLayout.state = DXSTATE_LOADED;
    ThrowIfFailed(dxDev->CreateInputLayout(&elements[0], (UINT)elements.size(), byteCode.getPtr(), (UINT)byteCode.getLength(), &vertexLayout.inputLayout) );
    unlockByteCode(bcId);
	
	// compute vertex stride in bytes
	for (auto elm : elements)
		vertexLayout.vertexStrideBytes += DxHelper::SizeOfFormatElement(elm.Format);

    IdVertexLayout retId((uint32_t)m_vertexLayouts.size());
    m_vertexLayouts.push_back(vertexLayout);
    return retId;
}

task<IdByteCode> DxDeviceFactory::createShaderByteCode(const std::wstring& filename)
{
    return create_task( Engine::ReadDataAsync(filename) )
        .then( [this](const std::vector<byte>& fileData) -> IdByteCode
    {
        IdByteCode bcId((uint32_t)m_byteCodes.size());
        DxByteCode byteCode;
        byteCode.state = DXSTATE_LOADED;
        byteCode.data = std::make_shared<std::vector<byte>>(fileData);
        m_byteCodes.push_back(byteCode);
        return bcId;
    });
}

IdByteCode DxDeviceFactory::createShaderByteCode(const std::vector<byte>& bytecode)
{
    IdByteCode bcId((uint32_t)m_byteCodes.size());
    DxByteCode byteCode;
	byteCode.state = DXSTATE_LOADED;
    byteCode.data = std::make_shared<std::vector<byte>>(bytecode);
    m_byteCodes.push_back(byteCode);
    return bcId;
}

IdShader DxDeviceFactory::createShader(IdByteCode byteCodeId, eDxShaderStage stage)
{
    auto dxDev = DxDevice::getInstance()->GetD3DDevice();
    DxShader shader;

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
	shader.state = DXSTATE_LOADED;
    IdShader shId((uint32_t)m_shaders.size());
    m_shaders.push_back(shader);
    return shId;
}

IdMeshBuffer DxDeviceFactory::createMeshBuffer(const DxMeshBufferElementDesc* vertexDesc, const DxMeshBufferElementDesc* indexDesc)
{
	IdMeshBuffer mbId((uint32_t)m_meshBuffers.size());
	DxMeshBuffer meshBuffer;
	meshBuffer.state = DXSTATE_LOADED;
	m_meshBuffers.push_back(meshBuffer);

	if ( vertexDesc )
		createMeshBufferVertices(mbId, vertexDesc->data, vertexDesc->count, vertexDesc->strideBytes);

	if (indexDesc)
		createMeshBufferIndices(mbId, indexDesc->data, indexDesc->count, (eDxIndexFormat)indexDesc->strideBytes);

	return mbId;
}

static void createBufferInternal(UINT byteWidth, UINT bindFlags, const void* initData, ID3D11Buffer** pBuffer)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = byteWidth;
	bd.BindFlags = bindFlags;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA* pInitData = NULL;
	D3D11_SUBRESOURCE_DATA id;
	if (initData)
	{
		pInitData = &id;
		pInitData->SysMemPitch = 0;
		pInitData->SysMemSlicePitch = 0;
		pInitData->pSysMem = initData;
	}

	ThrowIfFailed(DxDevice::getInstance()->GetD3DDevice()->CreateBuffer(&bd, pInitData, pBuffer));
}

void	DxDeviceFactory::createMeshBufferVertices(IdMeshBuffer mbId, const void* vertexData, uint32_t vertexCount, uint32_t vertexStrideBytes)
{
	ThrowIfAssert(mbId.IsValid(), L"Mesh buffer not valid");
	ThrowIfAssert(vertexCount!=0, L"Vertex count cannot be 0");
	ThrowIfAssert(vertexStrideBytes!=0, L"Vertex stride cannot be 0");

	auto meshBuffer = lockMeshBuffer(mbId);
	createBufferInternal(vertexStrideBytes*vertexCount, D3D11_BIND_VERTEX_BUFFER, vertexData, &meshBuffer.vertexBuffer);
	meshBuffer.vertexCount = vertexCount;
	meshBuffer.vertexStrideBytes = vertexStrideBytes;
	unlockMeshBuffer(mbId);
}

void	DxDeviceFactory::createMeshBufferIndices(IdMeshBuffer mbId, const void* indexData, uint32_t indexCount, eDxIndexFormat indexFormat)
{
	ThrowIfAssert(mbId.IsValid(), L"Mesh buffer not valid");
	ThrowIfAssert(indexCount!=0, L"Index count cannot be 0");
	ThrowIfAssert(indexFormat == 2 || indexFormat == 4, L"Index stride has to be 2 or 4");

	auto meshBuffer = lockMeshBuffer(mbId);
	createBufferInternal(static_cast<int>(indexFormat) * indexCount, D3D11_BIND_INDEX_BUFFER, indexData, &meshBuffer.indexBuffer);
	meshBuffer.indexCount = indexCount;
	meshBuffer.indexStrideBytes = indexFormat;
	unlockMeshBuffer(mbId);
}

void	DxDeviceFactory::fillMeshBufferVertices(IdMeshBuffer mbId, const void* vertexData, uint32_t vertexCount)
{
	DXDEVFACTORY_EMIT_FILLBUFFER(vertex);
}

void	DxDeviceFactory::fillMeshBufferIndices(IdMeshBuffer mbId, const void* indexData, uint32_t indexCount)
{
	DXDEVFACTORY_EMIT_FILLBUFFER(index);
}

void DxDeviceFactory::mapMeshBufferVertices(IdMeshBuffer mbId, uint32_t& vertexCount, uint32_t& vertexStrideBytes, void** outData, D3D11_MAP mapType)
{
	DXDEVFACTORY_EMIT_MAPBUFFER(vertex);
}

void DxDeviceFactory::unmapMeshBufferVertices(IdMeshBuffer mbId)
{
	auto meshBuffer = lockMeshBuffer(mbId);
	DxDevice::getInstance()->GetD3DDeviceContext()->Unmap(meshBuffer.vertexBuffer, 0);
	unlockMeshBuffer(mbId);
}

void DxDeviceFactory::mapMeshBufferIndices(IdMeshBuffer mbId, uint32_t& indexCount, uint32_t& indexStrideBytes, void** outData, D3D11_MAP mapType)
{
	DXDEVFACTORY_EMIT_MAPBUFFER(index);
}

void DxDeviceFactory::unmapMeshBufferIndices(IdMeshBuffer mbId)
{
	auto meshBuffer = lockMeshBuffer(mbId);
	DxDevice::getInstance()->GetD3DDeviceContext()->Unmap(meshBuffer.indexBuffer, 0);
	unlockMeshBuffer(mbId);
}

IdBlendState DxDeviceFactory::createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend)
{
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.RenderTarget[0].BlendEnable = (srcBlend != D3D11_BLEND_ONE) || (destBlend != D3D11_BLEND_ZERO);
	desc.RenderTarget[0].SrcBlend = desc.RenderTarget[0].SrcBlendAlpha = srcBlend;
	desc.RenderTarget[0].DestBlend = desc.RenderTarget[0].DestBlendAlpha = destBlend;
	desc.RenderTarget[0].BlendOp = desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	DXDEVFACTORY_EMIT_STATECREATEFINAL(BlendState, &desc);
}


IdDepthStencilState	DxDeviceFactory::createDepthStencilState(bool enable, bool writeEnable)
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.DepthEnable = enable;
	desc.DepthWriteMask = writeEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	desc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	desc.StencilEnable = false;
	desc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	desc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

	desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;

	desc.BackFace = desc.FrontFace;

	DXDEVFACTORY_EMIT_STATECREATEFINAL(DepthStencilState, &desc);
}

IdRasterizerState DxDeviceFactory::createRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode)
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.CullMode = cullMode;
	desc.FillMode = fillMode;
	desc.DepthClipEnable = true;
	desc.MultisampleEnable = true;

	DXDEVFACTORY_EMIT_STATECREATEFINAL(RasterizerState, &desc);
}

IdSamplerState DxDeviceFactory::createSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode)
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));

	desc.Filter = filter;

	desc.AddressU = addressMode;
	desc.AddressV = addressMode;
	desc.AddressW = addressMode;
	desc.MaxAnisotropy = (DxDevice::getInstance()->GetD3DDevice()->GetFeatureLevel() > D3D_FEATURE_LEVEL_9_1) ? 16 : 2;

	desc.MaxLOD = FLT_MAX;
	desc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	DXDEVFACTORY_EMIT_STATECREATEFINAL(SamplerState, &desc);
}

template<typename CONT>
void releaseResourceInternal(CONT& container, uint32_t n)
{
	CONT::value_type& resource = container[n];
	resource.release();
	resource.state = DXSTATE_RELEASED;
}

void DxDeviceFactory::releaseResource(uint32_t ResourceId)
{
	ThrowIfAssert(IdGeneric::CreateFrom(ResourceId).IsValid()); // check if valid resource
	const uint32_t resType = IdGeneric::ExtractType(ResourceId);
	const uint32_t resNumber = IdGeneric::ExtractNumber(ResourceId);

	switch (resType)
	{
	case ID_RENDERTARGET: releaseResourceInternal(m_renderTargets, resNumber); break;
	case ID_VERTEXLAYOUT: releaseResourceInternal(m_vertexLayouts, resNumber); break;
	case ID_MESHBUFFER: releaseResourceInternal(m_meshBuffers, resNumber); break;
	case ID_TEXTURE: releaseResourceInternal(m_textures, resNumber); break;
	case ID_BYTECODE: releaseResourceInternal(m_byteCodes, resNumber); break;
	case ID_SHADER: releaseResourceInternal(m_shaders, resNumber); break;
	case ID_SAMPLERSTATE: releaseResourceInternal(m_SamplerStates, resNumber); break;
	case ID_DEPTHSTENCILSTATE: releaseResourceInternal(m_DepthStencilStates, resNumber); break;
	case ID_RASTERIZERSTATE: releaseResourceInternal(m_RasterizerStates, resNumber); break;
	case ID_BLENDSTATE: releaseResourceInternal(m_BlendStates, resNumber); break;
	default:
		ThrowIfAssert(false, L"Unknown resource to be released here");
	}
}

IdBlendState DxDeviceFactory::getCommonBlendState(DxCommonBlendStateType blendStateType) { return m_BlendStatesCommon[blendStateType]; }
IdDepthStencilState	DxDeviceFactory::getCommonDepthStencilState(DxCommonDepthStencilType depthStencilType) { return m_DepthStencilStatesCommon[depthStencilType]; }
IdRasterizerState DxDeviceFactory::getCommonRasterizerState(DxCommonRasterizerType rasterizerType) { return m_RasterizerStatesCommon[rasterizerType]; }
IdSamplerState DxDeviceFactory::getCommonSamplerState(DxCommonSamplerType samplerType) { return m_SamplerStatesCommon[samplerType]; }

void DxDeviceFactory::createCommonStates()
{
	using namespace DirectX;
	CommonStates commonStates(DxDevice::getInstance()->GetD3DDevice());
	
	// blend states
	DXDEVFACTORY_EMIT_CREATECOMMON(BlendState, COMMONBLEND_MAX, &CommonStates::Opaque, &CommonStates::AlphaBlend, &CommonStates::Additive, &CommonStates::NonPremultiplied);
	DXDEVFACTORY_EMIT_CREATECOMMON(DepthStencilState, COMMONDEPTHSTENCIL_MAX, &CommonStates::DepthNone, &CommonStates::DepthDefault, &CommonStates::DepthRead);
	DXDEVFACTORY_EMIT_CREATECOMMON(RasterizerState, COMMONRASTERIZER_MAX, &CommonStates::CullNone, &CommonStates::CullClockwise, &CommonStates::CullCounterClockwise, &CommonStates::Wireframe);
	DXDEVFACTORY_EMIT_CREATECOMMON(SamplerState, COMMONSAMPLER_MAX, &CommonStates::PointWrap, &CommonStates::PointClamp, &CommonStates::LinearWrap, &CommonStates::LinearClamp, &CommonStates::AnisotropicWrap, &CommonStates::AnisotropicClamp );
}

void DxDeviceFactory::releaseCommonStates()
{
	for (auto rid : m_BlendStatesCommon) releaseResource(rid);
	for (auto rid : m_DepthStencilStatesCommon) releaseResource(rid);
	for (auto rid : m_RasterizerStatesCommon) releaseResource(rid);
	for (auto rid : m_SamplerStatesCommon) releaseResource(rid);
}

DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(RenderTarget, m_renderTargets);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(Texture, m_textures);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(VertexLayout, m_vertexLayouts);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(ByteCode, m_byteCodes);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(Shader, m_shaders);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(MeshBuffer, m_meshBuffers);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(BlendState, m_BlendStates);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(DepthStencilState, m_DepthStencilStates);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(RasterizerState, m_RasterizerStates);
DXDEVFACTORY_EMIT_LOCKUNLOCK_IMPL(SamplerState, m_SamplerStates);

