#pragma once

#include <string>
#include <optional>
#include <vector>

class Settings {
public:
	char
		path[256] = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo.exe",
		language[256] = "",
		execConfig[256] = "";

	int
		tickrate = 64,
		refresh = 60,
		threads = 4;

	struct Resolution {
		int
			width = 1024,
			height = 768,
			displayMode = 1;
	}res;

	bool
		insecure = false,
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
		noBrowser = false;
	;
};

inline std::optional<Settings> cfg;