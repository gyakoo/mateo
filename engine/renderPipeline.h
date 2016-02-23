#pragma once

namespace engine
{
  class stepTimer;

  class renderPipeline
  {
  public:
    renderPipeline();
    virtual ~renderPipeline();

    virtual void update(const stepTimer& timer);
    virtual void render();

    virtual void createResources();
    virtual void releaseResources();
    virtual void reloadWindowSizeResources();
  };

};