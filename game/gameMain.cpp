#include "pch.h"
#include <game/gameMain.h>

using namespace game;
using namespace Windows::Foundation;
using namespace Windows::System::Threading;
using namespace Concurrency;

gameMain::gameMain(Windows::ApplicationModel::Core::CoreApplicationView^ appView)
{
  m_engineBase = ref new engine::base(appView, true);

  // test
  m_engineBase->registerRenderPipeline("testRenderPipeline");
}

gameMain::~gameMain()
{
}

void gameMain::setWindow(Windows::UI::Core::CoreWindow^ window)
{
  m_engineBase->setWindow(window, true);
}

bool gameMain::run()
{
  m_engineBase->update();
  m_engineBase->render();
  return true;
}

void gameMain::load(Platform::String^ entryPoint)
{
  m_engineBase->onAppLoad(entryPoint);
}