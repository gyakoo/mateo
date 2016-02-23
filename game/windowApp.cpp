#include "pch.h"
#include "windowApp.h"

#include <ppltasks.h>

using namespace game;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto appSourceCreator = ref new appSource();
	CoreApplication::Run(appSourceCreator);
	return 0;
}

IFrameworkView^ appSource::CreateView()
{
	return ref new windowApp();
}

windowApp::windowApp() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

// The first method called when the IFrameworkView is being created.
void windowApp::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. 
	applicationView->Activated += ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &windowApp::OnActivated);
	CoreApplication::Suspending += ref new EventHandler<SuspendingEventArgs^>(this, &windowApp::OnSuspending);
  CoreApplication::Resuming += ref new EventHandler<Platform::Object^>(this, &windowApp::OnResuming);

	// At this point we have access to the device. We can create the device-dependent resources.  
  m_gameMain = std::make_unique<gameMain>(applicationView);
}

// Called when the CoreWindow object is created (or re-created).
void windowApp::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &windowApp::OnWindowSizeChanged);
	window->VisibilityChanged += ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &windowApp::OnVisibilityChanged);
	window->Closed += ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &windowApp::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
	currentDisplayInformation->DpiChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &windowApp::OnDpiChanged);
	currentDisplayInformation->OrientationChanged += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &windowApp::OnOrientationChanged);
	DisplayInformation::DisplayContentsInvalidated += ref new TypedEventHandler<DisplayInformation^, Object^>(this, &windowApp::OnDisplayContentsInvalidated);

  m_gameMain->setWindow(window);
}

// Initializes scene resources, or loads a previously saved app state.
void windowApp::Load(Platform::String^ entryPoint)
{
  m_gameMain->load(entryPoint);
}

// This method is called after the window becomes active.
void windowApp::Run()
{
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

      m_gameMain->run();
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void windowApp::Uninitialize()
{
}

void windowApp::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void windowApp::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
}

void windowApp::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
}

void windowApp::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
}

void windowApp::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void windowApp::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void windowApp::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
}

void windowApp::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
}

void windowApp::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
}