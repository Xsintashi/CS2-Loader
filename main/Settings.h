#pragma once

#include <string>
#include <optional>

class Settings {
public:
	struct General{
	std::string
		path = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive\\csgo.exe",
		language = "",
		execConfig = "";

	int
		priority = 0,
		tickrate = 0,
		refresh = 60,
		threads = 4;
	;

	bool
		insecure = false,
		logConsole = false,
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

	struct Display {
		int
			displayMode = 1,
			aspectRatio = 0,
			resolution = 0;
	} display;

	} general;
private:
};

inline std::optional<Settings> cfg;