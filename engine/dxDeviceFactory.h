#pragma once
#include <Engine/DxTypes.h>

#define DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(typetoken) \
        Dx##typetoken& lock##typetoken(Id##typetoken id);\
        void unlock##typetoken(Id##typetoken id);

#define DXDEVFACTORY_EMIT_STATE_ARRAYS(typetoken, count)\
		std::vector<Dx##typetoken> m_##typetoken##s;\
		std::array<Id##typetoken, count> m_##typetoken##sCommon

typedef HRESULT(WINAPI *D3DReflectFunc)(LPCVOID pSrcData,
    SIZE_T SrcDataSize,
    REFIID pInterface,
    void** ppReflector);

namespace Engine
{
    	class DxDeviceFactory
	{
	public:
		DxDeviceFactory();
		~DxDeviceFactory();
		void  releaseResources();

		// Creation
		IdRenderTarget  createRenderTarget(int32_t width, int32_t height, DXGI_FORMAT texf, bool asTexture = true, bool asDepth = false);
		Concurrency::task<IdTexture> createTexture(const std::wstring& filename, uint32_t flags = 0);
		IdTexture       createTexture(IdRenderTarget rt);
		IdTexture       createTexture(DXGI_FORMAT texf, uint8_t* data, uint32_t width, uint32_t height, bool asDynamic = false);
		IdVertexLayout  createVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, IdByteCode bytecode);
        Concurrency::task<IdByteCode> createShaderByteCode(const std::wstring& filename);
		IdByteCode      createShaderByteCode(const std::vector<byte>& bytecode);
		IdShader        createShader(IdByteCode byteCodeId, eDxShaderStage stage);
        IdConstantBuffer createConstantBuffer(IdByteCode byteCode);
		IdMeshBuffer    createMeshBuffer(const DxMeshBufferElementDesc* vertexDesc=nullptr, const DxMeshBufferElementDesc* indexDesc=nullptr);
		void			createMeshBufferVertices(IdMeshBuffer mbId, const void* vertexData, uint32_t vertexCount, uint32_t vertexStrideBytes);
		void			createMeshBufferIndices(IdMeshBuffer mbId, const void* indexData, uint32_t indexCount, eDxIndexFormat indexFormat);
		IdBlendState		createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend);
		IdDepthStencilState	createDepthStencilState(bool enable, bool writeEnable);
		IdRasterizerState	createRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode);
		IdSamplerState		createSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);

		void releaseResource(uint32_t ResourceId);

		IdBlendState		getCommonBlendState(eDxCommonBlendStateType blendStateType);
		IdDepthStencilState	getCommonDepthStencilState(eDxCommonDepthStencilType depthStencilType);
		IdRasterizerState	getCommonRasterizerState(eDxCommonRasterizerType rasterizerType);
		IdSamplerState		getCommonSamplerState(eDxCommonSamplerType samplerType);
		IdRenderTarget		getCommonRenderTarget();

		void			fillMeshBufferVertices(IdMeshBuffer mbId, const void* vertexData, uint32_t vertexCount);
		void			fillMeshBufferIndices(IdMeshBuffer mbId, const void* indexData, uint32_t indexCount);
		void			mapMeshBufferVertices(IdMeshBuffer mbId, uint32_t& outVertexCount, uint32_t& outVertexStrideBytes, void** outDat, D3D11_MAP mapType=D3D11_MAP_WRITE);
		void			mapMeshBufferIndices(IdMeshBuffer mbId, uint32_t& outIndexCount, uint32_t& outIndexStrideBytes, void** outData, D3D11_MAP mapType = D3D11_MAP_WRITE);
		void			unmapMeshBufferVertices(IdMeshBuffer mbId);
		void			unmapMeshBufferIndices(IdMeshBuffer mbId);

        // Accessing to internal resources 
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(RenderTarget);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(Texture);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(ByteCode);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(VertexLayout);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(Shader);
        DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(ConstantBuffer);
        DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(MeshBuffer);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(BlendState);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(DepthStencilState);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(RasterizerState);
        DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(SamplerState);

    protected:
		void createCommonStates();
		void releaseCommonStates();

	protected:
        std::vector<DxRenderTarget> m_renderTargets;
        std::vector<DxTexture> m_textures;
        std::vector<DxVertexLayout> m_vertexLayouts;
        std::vector<DxByteCode> m_byteCodes;
        std::vector<DxShader> m_shaders;
        std::vector<DxConstantBuffer> m_constantBuffers;
        std::vector<DxMeshBuffer> m_meshBuffers;
		DXDEVFACTORY_EMIT_STATE_ARRAYS(BlendState, COMMONBLEND_MAX);
		DXDEVFACTORY_EMIT_STATE_ARRAYS(DepthStencilState, COMMONDEPTHSTENCIL_MAX);
		DXDEVFACTORY_EMIT_STATE_ARRAYS(RasterizerState, COMMONRASTERIZER_MAX);
		DXDEVFACTORY_EMIT_STATE_ARRAYS(SamplerState, COMMONSAMPLER_MAX);
		IdRenderTarget m_commonRenderTarget;
        HMODULE m_d3dDLLCompiler;
        D3DReflectFunc m_d3dDLLReflect;
    };

};
