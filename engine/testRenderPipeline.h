﻿#pragma once

#include <engine/RenderPipeline.h>

namespace Engine
{
	// Renders the current FPS value in the bottom right corner of the screen using Direct2D and DirectWrite.
	class TestRenderPipeline : public RenderPipeline
	{
	public:
        TestRenderPipeline();		

        virtual void Update(const StepTimer& timer);
        virtual void Render();

        virtual void createResources();
        virtual void releaseResources();
        virtual void ReloadWindowSizeResources();

	private:
        IdTexture   m_mytex;
        IdByteCode  m_psByteCode;
        IdShader    m_ps;
        IdConstantBuffer m_psCb;
	};
}