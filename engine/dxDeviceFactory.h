#pragma once

#define dxSafeRelease(ptr) { if (ptr) { (ptr)->Release(); (ptr)=nullptr; } }

#define DXDEVFACTORY_EMIT_LOCKUNLOCK_DECL(typetoken) \
        const dx##typetoken& lock##typetoken(id##typetoken id) const;\
        void unlock##typetoken(id##typetoken id) const;

#define DXDEVFACTORY_EMIT_STATE_DECL(typetoken) \
	struct dx##typetoken: public dxResource \
	{ \
		dx##typetoken() :stateObj(nullptr) {} \
		ID3D11##typetoken* stateObj; \
		void release() { dxSafeRelease(stateObj); }\
	}

#define DXDEVFACTORY_EMIT_STATE_ARRAYS(typetoken, count)\
		std::vector<dx##typetoken> m_##typetoken##s;\
		std::array<id##typetoken, count> m_##typetoken##sCommon

namespace engine
{
    enum dxShaderStage 
    { 
		SHADER_NONE = -1,
        SHADER_VERTEX = 0, 
        SHADER_HULL, 
        SHADER_DOMAIN, 
        SHADER_GEOMETRY, 
        SHADER_PIXEL, 
        SHADER_COMPUTE 
    };

	enum dxResourceState
	{
		DXSTATE_INVALID = 0,
		DXSTATE_LOADED,
		DXSTATE_LOADING,
		DXSTATE_RELEASED
	};

	enum dxIndexFormat
	{
		INDEX_16 = 2,
		INDEX_32 = 4
	};

	struct dxResource
	{
		dxResource() : state(DXSTATE_INVALID), hashValue(0), lockCount(0) {}

		dxResourceState state;
		std::size_t hashValue;
		mutable	int32_t lockCount;
	};

    struct dxTexture : public dxResource
    {
		dxTexture() : texture(nullptr), textureShaderResourceView(nullptr) {}
		void release()
		{
			dxSafeRelease(texture);
			dxSafeRelease(textureShaderResourceView);
		}

        ID3D11Texture2D*            texture;
        ID3D11ShaderResourceView*   textureShaderResourceView;
    };

    struct dxRenderTarget : public dxTexture
    {
		dxRenderTarget() : renderTargetView(nullptr), renderTargetDepthStencilTexture(nullptr), renderTargetDepthStencilView(nullptr) {}
		void release()
		{
			dxSafeRelease(renderTargetView);
			dxSafeRelease(renderTargetDepthStencilTexture);
			dxSafeRelease(renderTargetDepthStencilView);
		}

        ID3D11RenderTargetView*     renderTargetView;
        ID3D11Texture2D*            renderTargetDepthStencilTexture;
        ID3D11DepthStencilView*     renderTargetDepthStencilView;
    };

    struct dxVertexLayout : public dxResource
    {
		dxVertexLayout() : inputLayout(nullptr), vertexStrideBytes(0){}
		void release()
		{
			dxSafeRelease(inputLayout);
		}

        ID3D11InputLayout* inputLayout;
		uint32_t vertexStrideBytes;
    };
    
    struct dxByteCode : public dxResource
    {
        std::shared_ptr< std::vector<byte> > data;
		void release()
		{
			data = nullptr;
		}

        const void* getPtr() const { return &(*data.get())[0]; }
        uint32_t getLength() const { return (uint32_t)data->size(); }
    };

    struct dxShader : public dxResource
    {
		dxShader() : stage(SHADER_NONE), shader(nullptr) {}
		void release()
		{
			dxSafeRelease(shader);
		}

        dxShaderStage   stage;
        ID3D11DeviceChild* shader;
    };

    struct dxMeshBuffer : public dxResource
    {
		dxMeshBuffer() : vertexBuffer(nullptr), vertexCount(0)
			, indexBuffer(nullptr), indexCount(0)
			, vertexStrideBytes(0), indexStrideBytes(INDEX_16){}
		void release()
		{
			dxSafeRelease(vertexBuffer);
			dxSafeRelease(indexBuffer);
		}

        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;

        uint32_t vertexCount;
        uint32_t indexCount;

		uint32_t vertexStrideBytes;
		dxIndexFormat indexStrideBytes;
    };

	struct dxMeshBufferElementDesc
	{
		const void* data;
		uint32_t count;
		uint32_t strideBytes;
	};

	enum dxCommonBlendStateType { COMMONBLEND_OPAQUE=0, COMMONBLEND_ALPHABLEND, COMMONBLEND_ADDITIVE, COMMONBLEND_NONPREMULTIPLIED, COMMONBLEND_MAX};
	enum dxCommonDepthStencilType { COMMONDEPTHSTENCIL_NONE=0, COMMONDEPTHSTENCIL_DEFAULT, COMMONDEPTHSTENCIL_READ, COMMONDEPTHSTENCIL_MAX };
	enum dxCommonRasterizerType { COMMONRASTERIZER_NONE =0, COMMONRASTERIZER_CULLCW, COMMONRASTERIZER_CULLCCW, COMMONRASTERIZER_WIREFRAME, COMMONRASTERIZER_MAX };
	enum dxCommonSamplerType { COMMONSAMPLER_POINTWRAP=0, COMMONSAMPLER_POINTCLAMP, COMMONSAMPLER_LINEARWRAP, COMMONSAMPLER_LINEARCLAMP, COMMONSAMPLER_ANISOWRAP, COMMONSAMPLER_ANISOCLAMP, COMMONSAMPLER_MAX };
	DXDEVFACTORY_EMIT_STATE_DECL(BlendState);
	DXDEVFACTORY_EMIT_STATE_DECL(DepthStencilState);
	DXDEVFACTORY_EMIT_STATE_DECL(RasterizerState);
	DXDEVFACTORY_EMIT_STATE_DECL(SamplerState);

	class dxDeviceFactory
	{
	public:
		dxDeviceFactory();
		~dxDeviceFactory();
		void  releaseResources();

		// Creation
		idRenderTarget  createRenderTarget(int32_t width, int32_t height, DXGI_FORMAT texf, bool asTexture = true, bool asDepth = false);
		Concurrency::task<idTexture> createTexture(const std::wstring& filename, uint32_t flags = 0);
		idTexture       createTexture(idRenderTarget rt);
		idTexture       createTexture(DXGI_FORMAT texf, uint8_t* data, uint32_t width, uint32_t height, bool asDynamic = false);
		idVertexLayout  createVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, idByteCode bytecode);
        Concurrency::task<idByteCode> createShaderByteCode(const std::wstring& filename);
		idByteCode      createShaderByteCode(const std::vector<byte>& bytecode);
		idShader        createShader(idByteCode byteCodeId, dxShaderStage stage);
		idMeshBuffer    createMeshBuffer(const dxMeshBufferElementDesc* vertexDesc=nullptr, const dxMeshBufferElementDesc* indexDesc=nullptr);
		void			createMeshBufferVertices(idMeshBuffer mbId, const void* vertexData, uint32_t vertexCount, uint32_t vertexStrideBytes);
		void			createMeshBufferIndices(idMeshBuffer mbId, const void* indexData, uint32_t indexCount, dxIndexFormat indexFormat);
		idBlendState		createBlendState(D3D11_BLEND srcBlend, D3D11_BLEND destBlend);
		idDepthStencilState	createDepthStencilState(bool enable, bool writeEnable);
		idRasterizerState	createRasterizerState(D3D11_CULL_MODE cullMode, D3D11_FILL_MODE fillMode);
		idSamplerState		createSamplerState(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);

		void releaseResource(uint32_t resourceId);

		idBlendState		getCommonBlendState(dxCommonBlendStateType blendStateType);
		idDepthStencilState	getCommonDepthStencilState(dxCommonDepthStencilType depthStencilType);
		idRasterizerState	getCommonRasterizerState(dxCommonRasterizerType rasterizerType);
		idSamplerState		getCommonSamplerState(dxCommonSamplerType samplerType);

		void			fillMeshBufferVertices(idMeshBuffer mbId, const void* vertexData, uint32_t vertexCount);
		void			fillMeshBufferIndices(idMeshBuffer mbId, const void* indexData, uint32_t indexCount);
		void			mapMeshBufferVertices(idMeshBuffer mbId, uint32_t& outVertexCount, uint32_t& outVertexStrideBytes, void** outDat, D3D11_MAP mapType=D3D11_MAP_WRITE);
		void			mapMeshBufferIndices(idMeshBuffer mbId, uint32_t& outIndexCount, uint32_t& outIndexStrideBytes, void** outData, D3D11_MAP mapType = D3D11_MAP_WRITE);
		void			unmapMeshBufferVertices(idMeshBuffer mbId);
		void			unmapMeshBufferIndices(idMeshBuffer mbId);

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
        std::vector<dxRenderTarget> m_renderTargets;
        std::vector<dxTexture> m_textures;
        std::vector<dxVertexLayout> m_vertexLayouts;
        std::vector<dxByteCode> m_byteCodes;
        std::vector<dxShader> m_shaders;
        std::vector<dxMeshBuffer> m_meshBuffers;
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
