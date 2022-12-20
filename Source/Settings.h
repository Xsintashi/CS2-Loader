#pragma once

#include <string>
#include <optional>
#include <vector>
#include <filesystem>

#include "../Lib/nlohmann/json.hpp"

class Settings {
public:
	Settings() noexcept;
	void openConfigDir() const noexcept;
	void Save(std::string save = "empty") noexcept;
	void Load(std::string load = "empty") noexcept;

	char
		language[32] = "", //e.g english
		game[32] = "", //csgo
		serverConnect[32] = "", //connect 127.0.0.1
		execConfig[32] = "";

	int
		tickrate = 0, //64 || 128
		refresh = 0, // 60 || 144
		threads = 0,
		heapsize = 0;

	struct Resolution {
		int
			width = 0,
			height = 0,
			displayMode = 0;
	}res;

	bool
		guiStyle = true, // false = GoldSrc Style | true = Source Style
		thirdParty = false,
		insecure = false,
		allowDebug = false,
		hijack = false,
		dev = false,
		logConsole = false,
		highPriority = false,
		noVideo = false,
		noJoystick = false,
		consoleOnStartup = false,
		limitVSConst = false,
		forceNoVSync = false,
		emulateGL = false,
		disableDX9Ex = false,
		softParticlesDefaultOFF = false,
		defaultCfg = false,
		noHLTV = false,
		noPreload = false,
		noAAFonts = false,
		noBrowser = false,
		toConsole = false,
		noCrashDialog = false;
private:
	std::filesystem::path path;
	void createConfigDir() const noexcept;
};

inline std::optional<Settings> cfg;