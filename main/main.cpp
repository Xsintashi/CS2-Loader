#include "gui.h"

#include <thread>
#include "Settings.h"
#include "Utils.h"
#include "Global.h"
#include <string>
#include <iostream>
#include <sstream>

int WINAPI wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)
{
	//AllocConsole(); freopen("CONOUT$", "w", stdout);

#ifdef DEBUG
	AllocConsole(); freopen("CONOUT$", "w", stdout);
#endif

	// Try to open the mutex.
	HANDLE hMutex = OpenMutex(
		MUTEX_ALL_ACCESS, 0, "csgoLoaderInstance");

	if (!hMutex)
		// Mutex doesn�t exist. This is
		// the first instance so create
		// the mutex.
		hMutex =
		CreateMutex(0, 0, "csgoLoaderInstance");
	else {
		// The mutex exists so this is the
		// the second instance so return.
		MessageBoxA(nullptr, "You can run only one instance of the Loader", "CS:GO Loader", MB_OK | MB_ICONERROR);
		return 0;
	}

	cfg.emplace(Settings{});
	global.emplace(GlobalVars{});
	
	global->steamPath = getSteamPath();

	std::stringstream args(wcharToChar(arguments));
	std::string argsTemp;

	while (std::getline(args, argsTemp, ' '))
	{
		global->arguments.push_back(argsTemp);
	}

	for (unsigned int i = 0; i < global->arguments.size(); ++i) {
		if (std::string(global->arguments[i]) == "-load") {
			Set::Load(global->arguments[i + 1]);
			i++;
		}
		if (std::string(global->arguments[i]) == "-silent") {
			prepareConfig();
			startTheGame();
			GUI::isRunning = false;
		}
		if (std::string(global->arguments[i]) == "-min") {
			GUI::windowVisibility(VISIBLITY::MINIMIZE);
		}
		if (std::string(global->arguments[i]) == "-hide") {
			GUI::windowVisibility(VISIBLITY::HIDE);
		}
	}

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
