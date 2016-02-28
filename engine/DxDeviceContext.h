#pragma once
#include <engine/DxTypes.h>

namespace Engine
{
    //
    // DxDeviceContextState
    //
    struct DxDeviceContextState
    {
        DxDeviceContextState();

        // RS
        D3D11_VIEWPORT m_RSViewport;
        IdRasterizerState m_RSRasterState;

        // IA
        D3D11_PRIMITIVE_TOPOLOGY m_IAPrimtiveTopology;
        IdVertexLayout m_IAVertexLayout;
        IdIndexBuffer m_IAIndexBuffer;
        std::array<IdVertexBuffer, D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT> m_IAVertexBuffers;

        // PS
        std::array<IdSamplerState, D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT> m_PSSamplerStates;

        // OM states
        std::array<IdRenderTarget, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> m_OMRenderTargets;
        IdRenderTarget m_OMDepthStencilTarget;
        IdBlendState m_OMBlendState;
        IdDepthStencilState m_OMDepthStencilState;

        // Shaders
        std::array<IdShader, SHADER_MAX> m_Shaders;

        bool operator ==(const DxDeviceContextState& rhs) const;
    };

    //
    // DxDeviceContext
    //
    class DxDeviceContext
    {
    public:
        DxDeviceContext(ID3D11DeviceContext* deviceContext);
        
		// RS
		void SetViewport(const D3D11_VIEWPORT& viewport);
		void SetRasterizerState(IdRasterizerState rasterizerState);

		// IA
		void SetInputLayout(IdVertexLayout vertexLayout);
		void SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology);
		void SetVertexBuffer(IdVertexBuffer vertexBuffer, uint32_t slot);
		void SetVertexBuffers(std::initializer_list<IdVertexBuffer> vertexBuffers, bool discardLeft = true);
        void SetIndexBuffer(IdIndexBuffer indexBuffer);
		
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

        void BeginMarker(const std::wstring& name);
        void EndMarker();

        void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation=0, uint32_t baseVertexLocation=0);
        void Draw(uint32_t vertexCount, uint32_t startVertexLocation = 0);
        void DrawAuto();

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
        void ApplyStatesDiff();

        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
        Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> m_userAnnotation;

		DxDeviceContextState m_lastState;
		DxDeviceContextState m_currentState;
    };
};
