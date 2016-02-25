#pragma once

#define DxSafeRelease(ptr) { if (ptr) { (ptr)->Release(); (ptr)=nullptr; } }

#define DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(typetoken) \
        const Dx##typetoken& lock##typetoken(Id##typetoken id) const;\
        void unlock##typetoken(Id##typetoken id) const;

#define DXDEVFACTORY_EMIT_STATE_DECL(typetoken) \
	struct Dx##typetoken: public DxResource \
	{ \
		Dx##typetoken() :stateObj(nullptr) {} \
		ID3D11##typetoken* stateObj; \
		void release() { DxSafeRelease(stateObj); }\
	}

#define DXDEVFACTORY_EMIT_STATE_ARRAYS(typetoken, count)\
		std::vector<Dx##typetoken> m_##typetoken##s;\
		std::array<Id##typetoken, count> m_##typetoken##sCommon

namespace Engine
{
    enum eDxShaderStage 
    { 
		SHADER_NONE = -1,
        SHADER_VERTEX = 0, 
        SHADER_HULL, 
        SHADER_DOMAIN, 
        SHADER_GEOMETRY, 
        SHADER_PIXEL, 
        SHADER_COMPUTE 
    };

	enum eDxResourceState
	{
		DXSTATE_INVALID = 0,
		DXSTATE_LOADED,
		DXSTATE_LOADING,
		DXSTATE_RELEASED
	};

	enum eDxIndexFormat
	{
		INDEX_16 = 2,
		INDEX_32 = 4
	};

	struct DxResource
	{
		DxResource() : state(DXSTATE_INVALID), hashValue(0), lockCount(0) {}

		eDxResourceState state;
		std::size_t hashValue;
		mutable	int32_t lockCount;
	};

    struct DxTexture : public DxResource
    {
		DxTexture() : texture(nullptr), textureShaderResourceView(nullptr) {}
		void release()
		{
			DxSafeRelease(texture);
			DxSafeRelease(textureShaderResourceView);
		}

        ID3D11Texture2D*            texture;
        ID3D11ShaderResourceView*   textureShaderResourceView;
    };

    struct DxRenderTarget : public DxTexture
    {
		DxRenderTarget() : renderTargetView(nullptr), renderTargetDepthStencilTexture(nullptr), renderTargetDepthStencilView(nullptr) {}
		void release()
		{
			DxSafeRelease(renderTargetView);
			DxSafeRelease(renderTargetDepthStencilTexture);
			DxSafeRelease(renderTargetDepthStencilView);
		}

        ID3D11RenderTargetView*     renderTargetView;
        ID3D11Texture2D*            renderTargetDepthStencilTexture;
        ID3D11DepthStencilView*     renderTargetDepthStencilView;
    };

    struct DxVertexLayout : public DxResource
    {
		DxVertexLayout() : inputLayout(nullptr), vertexStrideBytes(0){}
		void release()
		{
			DxSafeRelease(inputLayout);
		}

        ID3D11InputLayout* inputLayout;
		uint32_t vertexStrideBytes;
    };
    
    struct DxByteCode : public DxResource
    {
        std::shared_ptr< std::vector<byte> > data;
		void release()
		{
			data = nullptr;
		}

        const void* getPtr() const { return &(*data.get())[0]; }
        uint32_t getLength() const { return (uint32_t)data->size(); }
    };

    struct DxShader : public DxResource
    {
		DxShader() : stage(SHADER_NONE), shader(nullptr) {}
		void release()
		{
			DxSafeRelease(shader);
		}

        eDxShaderStage   stage;
        ID3D11DeviceChild* shader;
    };

    struct DxMeshBuffer : public DxResource
    {
		DxMeshBuffer() : vertexBuffer(nullptr), vertexCount(0)
			, indexBuffer(nullptr), indexCount(0)
			, vertexStrideBytes(0), indexStrideBytes(INDEX_16){}
		void release()
		{
			DxSafeRelease(vertexBuffer);
			DxSafeRelease(indexBuffer);
		}

        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;

        uint32_t vertexCount;
        uint32_t indexCount;

		uint32_t vertexStrideBytes;
		eDxIndexFormat indexStrideBytes;
    };

	struct DxMeshBufferElementDesc
	{
		const void* data;
		uint32_t count;
		uint32_t strideBytes;
	};

	enum DxCommonBlendStateType { COMMONBLEND_OPAQUE=0, COMMONBLEND_ALPHABLEND, COMMONBLEND_ADDITIVE, COMMONBLEND_NONPREMULTIPLIED, COMMONBLEND_MAX};
	enum DxCommonDepthStencilType { COMMONDEPTHSTENCIL_NONE=0, COMMONDEPTHSTENCIL_DEFAULT, COMMONDEPTHSTENCIL_READ, COMMONDEPTHSTENCIL_MAX };
	enum DxCommonRasterizerType { COMMONRASTERIZER_NONE =0, COMMONRASTERIZER_CULLCW, COMMONRASTERIZER_CULLCCW, COMMONRASTERIZER_WIREFRAME, COMMONRASTERIZER_MAX };
	enum DxCommonSamplerType { COMMONSAMPLER_POINTWRAP=0, COMMONSAMPLER_POINTCLAMP, COMMONSAMPLER_LINEARWRAP, COMMONSAMPLER_LINEARCLAMP, COMMONSAMPLER_ANISOWRAP, COMMONSAMPLER_ANISOCLAMP, COMMONSAMPLER_MAX };
	DXDEVFACTORY_EMIT_STATE_DECL(BlendState);
	DXDEVFACTORY_EMIT_STATE_DECL(DepthStencilState);
	DXDEVFACTORY_EMIT_STATE_DECL(RasterizerState);
	DXDEVFACTORY_EMIT_STATE_DECL(SamplerState);

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
		IdMeshBuffer    createMeshBuffer(const DxMeshBufferElementDesc* vertexDesc=nullptr, const DxMeshBufferElementDesc* indexDesc=nullptr);
		void			createMeshBufferVertices(IdMeshBuffer mbId, const void* vertexData, uint32_t vertexCount, uint32_t vertexStrideBytes);
		void			createMeshBufferIndices(IdMeshBuffer mbId, const void* indexData, uint32_t indexCount, eDxIndexFormat indexFormat);
		IdBlendState		createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend);
		IdDepthStencilState	createDepthStencilState(bool enable, bool writeEnable);
		IdRasterizerState	createRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode);
		IdSamplerState		createSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);

		void releaseResource(uint32_t ResourceId);

		IdBlendState		getCommonBlendState(DxCommonBlendStateType blendStateType);
		IdDepthStencilState	getCommonDepthStencilState(DxCommonDepthStencilType depthStencilType);
		IdRasterizerState	getCommonRasterizerState(DxCommonRasterizerType rasterizerType);
		IdSamplerState		getCommonSamplerState(DxCommonSamplerType samplerType);

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
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(MeshBuffer);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(BlendState);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(DepthStencilState);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(RasterizerState);
		DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(SamplerState);


		/*
        // Destroying resources
        int32_t ReleaseResource(uint32_t resId);
        template<typename T>
        void ReleaseResources(T* resources, uint32_t count)
        {
            for (uint32_t i = 0; i < count; ++i)
                ReleaseResource((uint32_t)resources[i]);
        }
*/
    protected:
		void createCommonStates();
		void releaseCommonStates();

	protected:
        std::vector<DxRenderTarget> m_renderTargets;
        std::vector<DxTexture> m_textures;
        std::vector<DxVertexLayout> m_vertexLayouts;
        std::vector<DxByteCode> m_byteCodes;
        std::vector<DxShader> m_shaders;
        std::vector<DxMeshBuffer> m_meshBuffers;
		DXDEVFACTORY_EMIT_STATE_ARRAYS(BlendState, COMMONBLEND_MAX);
		DXDEVFACTORY_EMIT_STATE_ARRAYS(DepthStencilState, COMMONDEPTHSTENCIL_MAX);
		DXDEVFACTORY_EMIT_STATE_ARRAYS(RasterizerState, COMMONRASTERIZER_MAX);
		DXDEVFACTORY_EMIT_STATE_ARRAYS(SamplerState, COMMONSAMPLER_MAX);
		
/*        gyIDTexture InternalAddTexture(gyTexture* pTex, int initRes);

        gyResourceMgr<gyTexture, ID_TEXTURE> textureArray;
        gyResourceMgr<gyVertexLayout, ID_VERTEXLAYOUT> vertexLayoutArray;
        gyResourceMgr<gyMeshBuffer, ID_MESHBUFFER> meshBuffArray;
        gyResourceMgr<gyShader, ID_SHADER> shaderArray;
        gyResourceMgr<gySamplerState, ID_SAMPLERSTATE> samplersArray;
        gyResourceMgr<gyRasterState, ID_RASTERSTATE> rasterArray;
        gyResourceMgr<gyDepthStencilState, ID_DEPTHSTENCILSTATE> depthStateArray;
        gyResourceMgr<gyBlendState, ID_BLENDSTATE> blendStateArray;
        */
    };

};
