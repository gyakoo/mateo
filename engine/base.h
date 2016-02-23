#pragma once
#include <engine/stepTimer.h>

namespace engine
{
    class dxDevice;
    class renderPipeline;

    [Windows::Foundation::Metadata::WebHostHidden]
    public ref class base sealed
    {
    public:
      base(Windows::ApplicationModel::Core::CoreApplicationView^ appView, bool setEventHandlers);

      void update();
      void render();
      void setWindow(Windows::UI::Core::CoreWindow^ window, bool setEventHandlers);
      void onAppLoad(Platform::String^ entryPoint);
      void deviceLost();
      void deviceRestored();
      void reloadWindowSizeResources();

      int registerRenderPipeline(Platform::String^ pipelineName);
      void unregisterRenderPipeline(int index);

    private:
      // Application lifecycle event handlers.
      void onActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
      void onSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
      void onResuming(Platform::Object^ sender, Platform::Object^ args);

      // Window event handlers.
      void onWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
      void onVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
      void onWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);

      // DisplayInformation event handlers.
      void onDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
      void onOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);
      void onDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args);

    private:
      Platform::Agile<Windows::UI::Core::CoreWindow> m_window;
      std::unique_ptr<dxDevice> m_dxDevice;
      stepTimer m_timer;
      std::vector< std::shared_ptr<renderPipeline> > m_renderPipelines;
    };
}
