#include "Settings.h"
#include <fstream>
#include <Windows.h>
#include <ShlObj.h>

using json = nlohmann::json;

#define w(name, valueName) js[name] = valueName;
#define l(name, valueName) valueName = js[name];
#define ll(name, name2, valueName) valueName = js[name][name2];

void Settings::Load(std::string load) noexcept {

	json js;

	if (std::ifstream in{ cfg->path / load }; in.good()) {
		js = json::parse(in, nullptr, false, true);
		if (js.is_discarded())
			return;
	} else {
		return;
	}

	l("Tickrate", cfg->tickrate)
	l("Refresh", cfg->refresh)
	l("Threads", cfg->threads)
	l("Heapsize", cfg->heapsize)

	ll("Display", "Width", cfg->res.width)
	ll("Display", "Height", cfg->res.height)
	ll("Display", "Mode", cfg->res.displayMode)

	sprintf(cfg->language, std::string(js["Language"]).c_str());
	sprintf(cfg->game, std::string(js["Game"]).c_str());
	sprintf(cfg->execConfig, std::string(js["Exec Config"]).c_str());
	sprintf(cfg->serverConnect, std::string(js["Server"]).c_str());

	l("Third Party Software", cfg->thirdParty)
	l("Insecure", cfg->insecure)
	l("Allow Debug", cfg->allowDebug)
	l("Hijack", cfg->hijack)
	l("Dev", cfg->dev)
	l("Log Console", cfg->logConsole)
	l("High Priority", cfg->highPriority)
	l("No Intro", cfg->noVideo)
	l("No Joystick Support", cfg->noJoystick)
	l("Console on Startup", cfg->consoleOnStartup)
	l("Limit Vertex Shader", cfg->limitVSConst)
	l("Force No VSync", cfg->forceNoVSync)
	l("Emulate GL", cfg->emulateGL)
	l("Disable DX9Ex", cfg->disableDX9Ex)
	l("OFF soft Particles", cfg->softParticlesDefaultOFF)
	l("Default Config", cfg->defaultCfg)
	l("No HLTV", cfg->noHLTV)
	l("NO Preload", cfg->noPreload)
	l("NO AA Fonts", cfg->noAAFonts)
	l("NO Browser", cfg->noBrowser)
	l("To Console", cfg->toConsole)
	l("No Crash Dialog", cfg->noCrashDialog)
}

void Settings::Save(std::string save) noexcept {

	json js;

	w("Tickrate",  cfg->tickrate)
	w("Refresh",  cfg->refresh)
	w("Threads",  cfg->threads)
	w("Heapsize",  cfg->heapsize)

	js["Display"] = {
		{ "Width" , cfg->res.width },
		{ "Height" , cfg->res.height },
		{ "Mode" , cfg->res.displayMode }
	};

	w("Language",  cfg->language)
	w("Game",  cfg->game)
	w("Exec Config",  cfg->execConfig)
	w("Server",  cfg->serverConnect)

	w("Third Party Software",  cfg->thirdParty)
	w("Insecure",  cfg->insecure)
	w("Allow Debug",  cfg->allowDebug)
	w("Hijack",  cfg->hijack)
	w("Dev",  cfg->dev)
	w("Log Console",  cfg->logConsole)
	w("High Priority",  cfg->highPriority)
	w("No Intro",  cfg->noVideo)
	w("No Joystick Support",  cfg->noJoystick)
	w("Console on Startup",  cfg->consoleOnStartup)
	w("Limit Vertex Shader",  cfg->limitVSConst)
	w("Force No VSync",  cfg->forceNoVSync)
	w("Emulate GL",  cfg->emulateGL)
	w("Disable DX9Ex",  cfg->disableDX9Ex)
	w("OFF soft Particles",  cfg->softParticlesDefaultOFF)
	w("Default Config",  cfg->defaultCfg)
	w("No HLTV",  cfg->noHLTV)
	w("NO Preload",  cfg->noPreload)
	w("NO AA Fonts",  cfg->noAAFonts)
	w("NO Browser",  cfg->noBrowser)
	w("To Console", cfg->toConsole)
	w("No Crash Dialog", cfg->noCrashDialog)

	createConfigDir();
	if (std::ofstream out{ path / save }; out.good())
		out << std::setw(2) << js;
}

[[nodiscard]] static std::filesystem::path buildConfigsFolderPath() noexcept
{
	std::filesystem::path path;

	if (PWSTR pathToAppData; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathToAppData))) {
		path = pathToAppData;
		CoTaskMemFree(pathToAppData);
	}

	path /= "CSGO Loader";
	return path;
}

void Settings::createConfigDir() const noexcept {
	std::error_code ec; std::filesystem::create_directory(path, ec);
}

void Settings::openConfigDir() const noexcept {
	createConfigDir();
	ShellExecuteW(nullptr, L"open", path.wstring().c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

Settings::Settings() noexcept : path{ buildConfigsFolderPath() } {
	createConfigDir();
}