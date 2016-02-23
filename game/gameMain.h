#pragma once

// Renders Direct2D and 3D content on the screen.
namespace game
{
	class gameMain
	{
	public:
		gameMain(Windows::ApplicationModel::Core::CoreApplicationView^ appView);
		~gameMain();

        void load(Platform::String^ entryPoint);
        void setWindow(Windows::UI::Core::CoreWindow^ window);
        bool run();

	private:
        Platform::Agile<engine::base> m_engineBase;		
	};
}