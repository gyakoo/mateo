#pragma once

#include <string>
#include <engine/renderPipeline.h>

namespace engine
{
	// Renders the current FPS value in the bottom right corner of the screen using Direct2D and DirectWrite.
	class testRenderPipeline : public renderPipeline
	{
	public:
        testRenderPipeline();		

        virtual void update(const stepTimer& timer);
        virtual void render();

        virtual void createResources();
        virtual void releaseResources();
        virtual void reloadWindowSizeResources();

	private:
		// Resources related to text rendering.
		std::wstring                                    m_text;
		DWRITE_TEXT_METRICS	                            m_textMetrics;
		Microsoft::WRL::ComPtr<ID2D1SolidColorBrush>    m_whiteBrush;
		Microsoft::WRL::ComPtr<ID2D1DrawingStateBlock1> m_stateBlock;
		Microsoft::WRL::ComPtr<IDWriteTextLayout3>      m_textLayout;
		Microsoft::WRL::ComPtr<IDWriteTextFormat2>      m_textFormat;
	};
}