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
