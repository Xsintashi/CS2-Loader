#include "gui.h"

#include <thread>
#include "Settings.h"
#include "Utils.h"

int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)
{

	cfg.emplace(Settings{});

	strcpy(cfg->path, getSteamPath());

	// create gui
	GUI::CreateHWindow("Counter-Strike: Global Offensive Loader");
	GUI::CreateDevice();
	GUI::CreateImGui();

	while (GUI::isRunning)
	{
		GUI::BeginRender();
		GUI::Render();
		GUI::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	// destroy gui
	GUI::DestroyImGui();
	GUI::DestroyDevice();
	GUI::DestroyHWindow();

	return EXIT_SUCCESS;
}
