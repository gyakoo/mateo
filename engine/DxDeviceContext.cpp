#include <Pch.h>
#include <engine/DxDeviceContext.h>
#include <engine/DxHelper.h>
#include <engine/DxDevice.h>

using namespace Engine;
using namespace Concurrency;
using namespace DirectX;

template<typename SRCCONT, typename DSTCONT>
void SetArrayOfElements(SRCCONT& srcCont, DSTCONT& dstCont, bool discardLeft)
{
	size_t count = 0;
	size_t bound = std::min(srcCont.size(), dstCont.size());
	for (auto elm = srcCont.begin(); count < bound; ++elm, ++count)
	{
		dstCont[count] = *elm;
	}

	// discard the rest of elements ids?
	if (discardLeft)
	{
		for (auto i = bound; i < dstCont.size(); ++i)
		{
			dstCont[i].Invalidate();
		}
	}
}

DxDeviceContext::DxDeviceContext(ID3D11DeviceContext* deviceContext)
    : m_deviceContext(deviceContext)
{
}

void DxDeviceContext::ClearRenderTarget(std::initializer_list<IdRenderTarget> arrayRTs, const float* color, uint32_t depthStencilFlags, float depth, uint8_t stencil)
{
	auto& factory = DxDevice::GetInstance()->GetFactory();

	for (auto rt = arrayRTs.begin(); rt != arrayRTs.end(); ++rt)
	{
		auto renderTarget = factory.lockRenderTarget(*rt);
		
		if (renderTarget.renderTargetView)
			m_deviceContext->ClearRenderTargetView(renderTarget.renderTargetView.Get(), color);

		if (depthStencilFlags != 0 && renderTarget.renderTargetDepthStencilView)
			m_deviceContext->ClearDepthStencilView(renderTarget.renderTargetDepthStencilView.Get(), depthStencilFlags, depth, stencil);
		
		factory.unlockRenderTarget(*rt);
	}
}

void DxDeviceContext::ClearRenderTargetDefault(const float* color)
{
	auto dxDev = DxDevice::GetInstance();
	
	auto backBufferRTV = dxDev->GetBackBufferRenderTargetView();
	m_deviceContext->ClearRenderTargetView(backBufferRTV, color);

	auto depthStencilV = dxDev->GetDepthStencilView();
	m_deviceContext->ClearDepthStencilView(depthStencilV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void DxDeviceContext::SetRenderTarget(std::initializer_list<IdRenderTarget> renderTargetIds, IdRenderTarget depthStencilTargetId, bool discardLeft)
{
	SetArrayOfElements(renderTargetIds, m_currentState.m_OMRenderTargets, discardLeft);
	m_currentState.m_OMDepthStencilTarget = depthStencilTargetId;
}

void DxDeviceContext::SetRenderTarget(IdRenderTarget renderTarget, uint32_t slot)
{
	m_currentState.m_OMRenderTargets[slot] = renderTarget;
}

void DxDeviceContext::SetRenderTargetDefault()
{
	auto commonRenderTarget = DxDevice::GetInstance()->GetFactory().getCommonRenderTarget();
	SetRenderTarget({ commonRenderTarget }, commonRenderTarget);
}

// RS
void DxDeviceContext::SetViewport(const D3D11_VIEWPORT& viewport)
{
	m_currentState.m_RSViewport = viewport;
}

// IA
void DxDeviceContext::SetInputLayout(IdVertexLayout vertexLayout)
{
	m_currentState.m_IAVertexLayout = vertexLayout;
}

void DxDeviceContext::SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY topology)
{
	m_currentState.m_IAManualPrimitiveTopology = topology;
}


void DxDeviceContext::SetIndexBuffer(IdMeshBuffer indexBuffer)
{
	m_currentState.m_IAIndexBuffer = indexBuffer;
}

void DxDeviceContext::SetVertexBuffer(IdMeshBuffer vertexBuffer, uint32_t slot)
{
	m_currentState.m_IAVertexBuffers[slot] = vertexBuffer;
}

void DxDeviceContext::SetMeshBuffers(std::initializer_list<IdMeshBuffer> vertexBuffers, IdMeshBuffer indexBuffer, bool discardLeft)
{
	SetArrayOfElements(vertexBuffers, m_currentState.m_IAVertexBuffers, discardLeft);
	SetIndexBuffer(indexBuffer);
}

void DxDeviceContext::SetBlendState(IdBlendState blendState)
{
	m_currentState.m_OMBlendState = blendState;
}

void DxDeviceContext::SetDepthStencilState(IdDepthStencilState depthStencilState)
{
	m_currentState.m_OMDepthStencilState = depthStencilState;
}

void DxDeviceContext::SetRasterizerState(IdRasterizerState rasterizerState)
{
	m_currentState.m_RSRasterState = rasterizerState;
}

void DxDeviceContext::SetSamplerState(std::initializer_list<IdSamplerState> samplerStates, bool discardLeft )
{
	SetArrayOfElements(samplerStates, m_currentState.m_PSSamplerStates, discardLeft);
}

void DxDeviceContext::SetSamplerState(IdSamplerState samplerState, uint32_t slot)
{
	m_currentState.m_PSSamplerStates[slot] = samplerState;
}

void DxDeviceContext::SetShader(eDxShaderStage shaderType, IdShader shader)
{
	m_currentState.m_shaders[shaderType] = shader;
}


void DxDeviceContext::Apply()
{

}