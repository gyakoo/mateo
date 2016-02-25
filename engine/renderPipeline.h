#pragma once

namespace Engine
{
  class StepTimer;

  class RenderPipeline
  {
  public:
    RenderPipeline();
    virtual ~RenderPipeline();

    virtual void Update(const StepTimer& timer);
    virtual void Render();

    virtual void createResources();
    virtual void releaseResources();
    virtual void ReloadWindowSizeResources();
  };

};