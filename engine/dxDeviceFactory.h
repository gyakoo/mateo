#pragma once

namespace engine
{
    enum dxShaderStage 
    { 
        SHADER_VERTEX = 0, 
        SHADER_HULL, 
        SHADER_DOMAIN, 
        SHADER_GEOMETRY, 
        SHADER_PIXEL, 
        SHADER_COMPUTE 
    };

    struct dxTexture
    {
        ID3D11Texture2D*            pTexture;
        ID3D11ShaderResourceView*   pShaderRV;
    };

    struct dxRenderTarget : public dxTexture
    {
        ID3D11RenderTargetView*     pRTView;
        ID3D11Texture2D*            pRTDSTexture;
        ID3D11DepthStencilView*     pRTDSView;
    };

    struct dxVertexLayout
    {
        ID3D11InputLayout* pInputLayout;
    };
    
    struct dxByteCode
    {
        std::shared_ptr< std::vector<byte> > data;

        const void* getPtr() const { return &(*data.get())[0]; }
        uint32_t getLength() const { return (uint32_t)data->size(); }
    };

    struct dxShader
    {
        dxShaderStage   stage;
        ID3D11DeviceChild* shader;
    };

    struct dxMeshBuffer
    {
        ID3D11Buffer* vertexBuffer;
        ID3D11Buffer* indexBuffer;
        uint32_t vbCount;
        uint32_t ibCount;
    };

    class dxDeviceFactory
    {
    public:
        dxDeviceFactory();
        ~dxDeviceFactory();
        void  releaseResources();

        // Creation
        idRenderTarget  createRenderTarget(int32_t width, int32_t height, DXGI_FORMAT texf, bool asTexture = true, bool asDepth = false);
        
        idTexture       createTexture(const std::wstring& filename, bool async = true, uint32_t flags = 0);
        idTexture       createTexture(idRenderTarget rt);
        idTexture       createTexture(DXGI_FORMAT texf, uint8_t* data, uint32_t width, uint32_t height, bool asDynamic=false);

        idVertexLayout  createVertexLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC>& elements, idByteCode bytecode);

        idByteCode      createShaderByteCode(const std::wstring& filename, bool async=true);
        idByteCode      createShaderByteCode(const std::vector<byte>& bytecode);

        idShader        createShader(idByteCode byteCodeId, dxShaderStage stage);
        idMeshBuffer    createMeshBuffer(idVertexLayout vlId, int indexStrideBytes);

/*      
        gyIDSamplerState  CreateSamplerState(gyFilterType filter, gyAddress u, gyAddress v, gyAddress w, float lodBias = 0.0f, uint8_t maxAniso = 16, const float* bordercol = 0, gyComparisonFunc compFunc = COMP_NONE);
        gyIDDepthStencilState CreateDepthStencilState(bool depthTest, bool depthWrite, gyComparisonFunc depthFunc = COMP_LESS_EQUAL,
            bool stencilTest = false, uint8_t stencilReadMask = 0xff, uint8_t stencilWriteMask = 0xff,
            gyStencilOp frontFail = SOP_KEEP, gyStencilOp frontDepthFail = SOP_KEEP, gyStencilOp frontPass = SOP_KEEP, gyComparisonFunc frontFunc = COMP_ALWAYS,
            gyStencilOp backFail = SOP_KEEP, gyStencilOp backDepthFail = SOP_KEEP, gyStencilOp backPass = SOP_KEEP, gyComparisonFunc backFunc = COMP_ALWAYS);
        gyIDRasterState CreateRasterState(gyFillMode fm = FM_SOLID, gyCullMode cm = CULL_BACK, int32_t dbias = 0, float dbclamp = 0, float ssdb = 0, uint8_t flags = RASTER_DEPTHCLIP_ENABLED);
        gyIDBlendState CreateBlendState(bool aphaToCoverage, gyBlend src, gyBlend dst, gyBlendOp op, gyBlend srcA = BLEND_ONE, gyBlend dstA = BLEND_ZERO, gyBlendOp opA = BLENDOP_ADD, uint8_t writeMask = 0xff);

        // Shader Techniques
        int32_t CreateShaderTechnique(gyShaderTechnique* pOutTech, gyShaderSourceDesc* stageSources);
        int32_t CreateShaderTechnique(gyShaderTechnique* pOutTech, gyShaderByteCode** stageByteCodes);
        int32_t CreateShaderTechnique(gyShaderTechnique* pOutTech, gyIDShader* stageShaders);
        int32_t CreateShaderTechnique(gyShaderTechnique* pOutTech, gyShaderSourceDesc* stageSources, gyShaderType* types, int32_t count);
*/
        // Accessing to internal resources (only for local operations, do not use the resource pointers. use ids instead)
        const dxRenderTarget& lockRenderTarget(idRenderTarget rtId) const;
        const dxByteCode& lockByteCode(idByteCode bcId) const;
        const dxVertexLayout& lockVertexLayout(idVertexLayout vlId) const;
        const dxShader& lockShader(idShader shId) const;
        const dxMeshBuffer& lockMeshBuffer(idMeshBuffer mbId) const;
/*
        gyVertexLayout*   LockVertexLayout(gyIDVertexLayout vlId);
        gyMeshBuffer*     LockMeshBuffer(gyIDMeshBuffer mbId);
        gyShader*         LockShader(gyIDShader shId);
        gyTexture*        LockTexture(gyIDTexture texId);
        gySamplerState*   LockSamplerState(gyIDSamplerState ssId);
        gyRasterState*    LockRasterState(gyIDRasterState rsId);
        gyDepthStencilState* LockDepthStencilState(gyIDDepthStencilState dsId);
        gyBlendState*     LockBlendState(gyIDBlendState bsId);
*/
        void unlockRenderTarget(idRenderTarget rtId) const;
        void unlockByteCode(idByteCode bcId) const;
        void unlockVertexLayout(idVertexLayout vlId) const;
        void unlockShader(idShader shId) const;
        void unlockMeshBuffer(idMeshBuffer mbId) const;
/*        void UnlockVertexLayout(gyIDVertexLayout vlId);
        void UnlockMeshBuffer(gyIDMeshBuffer mbId);
        void UnlockShader(gyIDShader shId);
        void UnlockTexture(gyIDTexture texId);
        void UnlockSamplerState(gyIDSamplerState ssId);
        void UnlockRasterState(gyIDRasterState rsId);
        void UnlockDepthStencilState(gyIDDepthStencilState dsId);
        void UnlockBlendState(gyIDBlendState bsId);

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
        std::vector<dxRenderTarget> m_renderTargets;
        std::vector<dxTexture> m_textures;
        std::vector<dxVertexLayout> m_vertexLayouts;
        std::vector<dxByteCode> m_byteCodes;
        std::vector<dxShader> m_shaders;
        std::vector<dxMeshBuffer> m_meshBuffers;
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
