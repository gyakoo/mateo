#include "pch.h"
#include <engine/testRenderPipeline.h>
#include <engine/dxHelper.h>
#include <engine/dxDevice.h>
#include <engine/stepTimer.h>

using namespace engine;
using namespace Microsoft::WRL;

// Initializes D2D resources used for text rendering.
testRenderPipeline::testRenderPipeline()
{
	ZeroMemory(&m_textMetrics, sizeof(DWRITE_TEXT_METRICS));

	// Create device independent resources
	ComPtr<IDWriteTextFormat> textFormat;
	engine::ThrowIfFailed(
		dxDevice::getInstance()->GetDWriteFactory()->CreateTextFormat(
			L"Segoe UI",
			nullptr,
			DWRITE_FONT_WEIGHT_LIGHT,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			32.0f,
			L"en-US",
			&textFormat
			)
		);

    engine::ThrowIfFailed(
		textFormat.As(&m_textFormat)
		);

    engine::ThrowIfFailed(
		m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR)
		);

    engine::ThrowIfFailed(
		dxDevice::getInstance()->GetD2DFactory()->CreateDrawingStateBlock(&m_stateBlock)
		);
}

// Updates the text to be displayed.
void testRenderPipeline::update(const stepTimer& timer)
{
    auto dxDevice = dxDevice::getInstance();

    // Update display text.
	uint32 fps = timer.GetFramesPerSecond();

	m_text = (fps > 0) ? std::to_wstring(fps) + L" FPS" : L" - FPS";

	ComPtr<IDWriteTextLayout> textLayout;
    engine::ThrowIfFailed(
        dxDevice->GetDWriteFactory()->CreateTextLayout(
			m_text.c_str(),
			(uint32) m_text.length(),
			m_textFormat.Get(),
			240.0f, // Max width of the input text.
			50.0f, // Max height of the input text.
			&textLayout
			)
		);

    engine::ThrowIfFailed(
		textLayout.As(&m_textLayout)
		);

    engine::ThrowIfFailed(
		m_textLayout->GetMetrics(&m_textMetrics)
		);
}

// Renders a frame to the screen.
void testRenderPipeline::render()
{
    auto dxDevice = dxDevice::getInstance();

    // clear render target
    {
        auto context = dxDevice->GetD3DDeviceContext();

        // Reset the viewport to target the whole screen.
        auto viewport = dxDevice->GetScreenViewport();
        context->RSSetViewports(1, &viewport);

        // Reset render targets to the screen.
        ID3D11RenderTargetView *const targets[1] = { dxDevice->GetBackBufferRenderTargetView() };
        context->OMSetRenderTargets(1, targets, dxDevice->GetDepthStencilView());

        // Clear the back buffer and depth stencil view.
        context->ClearRenderTargetView(dxDevice->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
        context->ClearDepthStencilView(dxDevice->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    }

	ID2D1DeviceContext* context = dxDevice::getInstance()->GetD2DDeviceContext();
	Windows::Foundation::Size logicalSize = dxDevice::getInstance()->GetLogicalSize();

	context->SaveDrawingState(m_stateBlock.Get());
	context->BeginDraw();

	// Position on the bottom right corner
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		logicalSize.Width - m_textMetrics.layoutWidth,
		logicalSize.Height - m_textMetrics.height
		);

	context->SetTransform(screenTranslation * dxDevice::getInstance()->GetOrientationTransform2D());

    engine::ThrowIfFailed(
		m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
		);

	context->DrawTextLayout(
		D2D1::Point2F(0.f, 0.f),
		m_textLayout.Get(),
		m_whiteBrush.Get()
		);

	// Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
        engine::ThrowIfFailed(hr);
	}

	context->RestoreDrawingState(m_stateBlock.Get());
}

void testRenderPipeline::createResources()
{
    engine::ThrowIfFailed(
        dxDevice::getInstance()->GetD2DDeviceContext()->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &m_whiteBrush)
		);

    dxDevice::getInstance()->getFactory()->createTexture(L"Content\\stones.jpg");
}
void testRenderPipeline::releaseResources()
{
	m_whiteBrush.Reset();
}

void testRenderPipeline::reloadWindowSizeResources()
{

}