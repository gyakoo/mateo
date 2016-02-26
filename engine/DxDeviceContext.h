#pragma once
#include <engine/DxTypes.h>

namespace Engine
{
    class DxDeviceContext
    {
    public:
        DxDeviceContext(ID3D11DeviceContext* deviceContext);
        
		void Apply();
	
		// RS
		void SetViewport(const D3D11_VIEWPORT& viewport);
		void SetRasterizerState(IdRasterizerState rasterizerState);

		// IA
		void SetInputLayout(IdVertexLayout vertexLayout);
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		void SetIndexBuffer(IdMeshBuffer indexBuffer);
		void SetVertexBuffer(IdMeshBuffer vertexBuffer, uint32_t slot);
		void SetMeshBuffers(std::initializer_list<IdMeshBuffer> vertexBuffers, IdMeshBuffer indexBuffer, bool discardLeft = true);
		
		// OM
		void ClearRenderTarget(std::initializer_list<IdRenderTarget> arrayRTs, const float* color, uint32_t depthStencilFlags, float depth = 1.0f, uint8_t stencil = 0);
		void ClearRenderTargetDefault(const float* color);
		void SetRenderTarget(std::initializer_list<IdRenderTarget> renderTargets, IdRenderTarget depthStencilTarget, bool discardLeft = true);
		void SetRenderTarget(IdRenderTarget renderTarget, uint32_t slot);
		void SetRenderTargetDefault();
		void SetBlendState(IdBlendState blendState);
		void SetDepthStencilState(IdDepthStencilState depthStencilState);

		// PS
		void SetSamplerState(std::initializer_list<IdSamplerState> samplerStates, bool discardLeft=true);
		void SetSamplerState(IdSamplerState samplerState, uint32_t slot);

		void SetShader(eDxShaderStage shaderType, IdShader shader);

        /*
          // -- getters and operations
  // -- states
  void SetVSShaderCB( gyIDShader shId );
  void SetVSShader( gyIDShader shId );
  void SetPSShaderCB( gyIDShader shId, bool applyConstants=true, bool applyTextures=true, bool applySamplers=true );
  void SetPSShader( gyIDShader shId );
  void SetPSTexture( gyIDTexture tId, uint32_t slot );

  void Draw( );
        */

    private:
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;

		DxDeviceContextState m_lastState;
		DxDeviceContextState m_currentState;
    };
};
