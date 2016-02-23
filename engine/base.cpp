#include "pch.h"
#include "base.h"
#include <engine/testRenderPipeline.h>
#include <engine/dxDevice.h>
#include <engine/renderPipeline.h>

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

namespace engine
{

  base::base(CoreApplicationView^ appView, bool setEventHandlers)
  {
    if (setEventHandlers)
    {
      // lifecycle handlers
      appView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &base::onActivated);
      CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &base::onSuspending);
      CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &base::onResuming);
    }

    m_dxDevice = std::make_unique<dxDevice>(this);

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */

  }

  void base::onAppLoad(Platform::String^ entryPoint)
  {

  }

  void base::update()
  {
    m_timer.Tick([&]()
    {
      // TODO: Replace this with your app's content update functions.      
      for (auto rpip : m_renderPipelines)
      {
        rpip->update(m_timer);
      }
    });

  }

  void base::render()
  {
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
      return ;

    for (auto rpip : m_renderPipelines)
    {
      rpip->render();
    }

    // Render the scene objects.
    m_dxDevice->Present();
  }

  void base::setWindow(CoreWindow^ window, bool setEventHandlers)
  {
    m_window = window;

    if (setEventHandlers)
    {
      // window handlers
      window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &base::onWindowSizeChanged);
      window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &base::onVisibilityChanged);
      window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &base::onWindowClosed);

      // display info handlers
      DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
      currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &base::onDpiChanged);
      currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &base::onOrientationChanged);
      DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &base::onDisplayContentsInvalidated);
    }

    m_dxDevice->SetWindow(window);
  }


  // Application lifecycle event handlers.
  void base::onActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args)
  {

  }

  void base::onSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args)
  {
    // Save app state asynchronously after requesting a deferral. Holding a deferral
    // indicates that the application is busy performing suspending operations. Be
    // aware that a deferral may not be held indefinitely. After about five seconds,
    // the app will be forced to exit.
    SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

    create_task([this, deferral]()
    {
      m_dxDevice->Trim();
      // Insert your code here.
      deferral->Complete();
    });
  }

  void base::onResuming(Platform::Object^ sender, Platform::Object^ args)
  {

  }

  // Window event handlers.
  void base::onWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args)
  {
    m_dxDevice->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
  }

  void base::onVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args)
  {

  }

  void base::onWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args)
  {

  }

  // DisplayInformation event handlers.
  void base::onDpiChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args)
  {
    // Note: The value for LogicalDpi retrieved here may not match the effective DPI of the app
    // if it is being scaled for high resolution devices. Once the DPI is set on DeviceResources,
    // you should always retrieve it using the GetDpi method.
    // See DeviceResources.cpp for more details.
    m_dxDevice->SetDpi(sender->LogicalDpi);
  }

  void base::onOrientationChanged(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args)
  {
    m_dxDevice->SetCurrentOrientation(sender->CurrentOrientation);
  }

  void base::onDisplayContentsInvalidated(Windows::Graphics::Display::DisplayInformation^ sender, Platform::Object^ args)
  {
    m_dxDevice->ValidateDevice();
  }

  void base::deviceLost()
  {
      for (auto rpip : m_renderPipelines)
      {
          rpip->releaseResources();
      }
  }

  void base::deviceRestored()
  {
      for (auto rpip : m_renderPipelines)
      {
          rpip->createResources();
      }
  }

  void base::reloadWindowSizeResources()
  {
      for (auto rpip : m_renderPipelines)
      {
          rpip->reloadWindowSizeResources();
      }
  }

  int base::registerRenderPipeline(Platform::String^ pipelineName)
  {
      int ret = -1;
      if (pipelineName == "testRenderPipeline")
      {
          ret = (int)m_renderPipelines.size();
          auto rpip = std::make_shared<testRenderPipeline>();
          m_renderPipelines.push_back(rpip);
          rpip->createResources();
          rpip->reloadWindowSizeResources();
      }
      return ret;
  }

  void base::unregisterRenderPipeline(int index)
  {
      if (index >= 0 && index < (int)m_renderPipelines.size())
      {
          m_renderPipelines.erase(m_renderPipelines.begin() + index);
      }
  }

}; // namespace