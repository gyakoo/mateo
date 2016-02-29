#include <Pch.h>
#include <engine/TestRenderPipeline.h>
#include <engine/DxHelper.h>
#include <engine/DxDevice.h>
#include <engine/StepTimer.h>

using namespace Engine;
using namespace Microsoft::WRL;

// Initializes D2D resources used for text rendering.
TestRenderPipeline::TestRenderPipeline()
{
}

// Updates the text to be displayed.
void TestRenderPipeline::Update(const StepTimer& timer)
{
    auto DxDevice = DxDevice::GetInstance();

}

// Renders a frame to the screen.
void TestRenderPipeline::Render()
{
    auto dxDev = DxDevice::GetInstance();
    auto& context = dxDev->GetDefaultContext();
    auto& factory = dxDev->GetFactory();

    // clear render target
    {
        //auto context = DxDevice->GetD3DDeviceContext();

        // Reset the viewport to target the whole screen.

        // Reset render targets to the screen.
        //ID3D11RenderTargetView *const targets[1] = { DxDevice->GetBackBufferRenderTargetView() };
        //context->OMSetRenderTargets(1, targets, DxDevice->GetDepthStencilView());

        // Clear the back buffer and depth stencil view.
        //context->ClearRenderTargetView(DxDevice->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
        //context->ClearDepthStencilView(DxDevice->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }
    
    context.SetViewport(dxDev->GetScreenViewport());
    context.SetRenderTargetDefault();
    context.ClearRenderTargetDefault(DirectX::Colors::CornflowerBlue);

    context.Apply();
}

void TestRenderPipeline::createResources()
{
    auto& factory = DxDevice::GetInstance()->GetFactory();
    factory.createTexture(L"Content\\stones.jpg")
        .then([this](IdTexture texId) 
    {
        m_mytex = texId; 
    });

    factory.createShaderByteCode(L"SamplePixelShader.cso")
        .then([&factory, this](IdByteCode bcId)
    {
        m_ps = factory.createShader(bcId, SHADER_PIXEL);
        m_psCb = factory.createConstantBuffer(bcId, SHADER_PIXEL);
    } );
}
void TestRenderPipeline::releaseResources()
{
    auto factory = DxDevice::GetInstance()->GetFactory();
    factory.releaseResource(m_ps);
    factory.releaseResource(m_psCb);
    factory.releaseResource(m_psByteCode);
    factory.releaseResource(m_mytex);
}

void TestRenderPipeline::ReloadWindowSizeResources()
{

}