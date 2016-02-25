#pragma once

namespace Engine
{
    class DxDeviceContext
    {
    public:
        DxDeviceContext(ID3D11DeviceContext* deviceContext);
        
        /*
          // -- getters and operations
  void Present( );
  void ClearRenderTarget( gyIDRenderTarget* rts, uint32_t numRts, uint32_t clearFlags = CS_CLEAR_ALL, float* rgba=NULL, float depth=1.0f, uint8_t stencil=0 );  

  // -- states
  int32_t SetDepthTarget( gyIDRenderTarget rtDepth );
  int32_t SetRenderTarget( gyIDRenderTarget* rts, uint32_t numRts, uint32_t startSlot=0 );
  int32_t SetMainRenderTarget( );
  void SetViewport( float left, float top, float width, float height );
  void SetInputLayout( gyIDVertexLayout ilayout );
  void SetMeshBuffers( gyIDMeshBuffer* mbId, uint32_t numMbs, uint32_t master=0, uint32_t startStream=0 );
  void SetDepthStencilState( gyIDDepthStencilState dsId, uint32_t stencilRef=~0 );
  void SetRasterState( gyIDRasterState rsId );
  void SetVSShaderCB( gyIDShader shId );
  void SetVSShader( gyIDShader shId );
  void SetPSSampler( gyIDSamplerState sId, uint32_t slot );
  void SetPSTexture( gyIDTexture tId, uint32_t slot );
  void SetPSShaderCB( gyIDShader shId, bool applyConstants=true, bool applyTextures=true, bool applySamplers=true );
  void SetPSShader( gyIDShader shId );
  void SetBlendState( gyIDBlendState bsId );        

  void Draw( );
        */

    private:
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
    };
};
