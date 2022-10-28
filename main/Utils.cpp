#include "Utils.h"
#include "Global.h"
#include "Settings.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <TlHelp32.h>

const char* getSteamPath() {
	DWORD dwType = REG_SZ;
	HKEY hKey = 0;
	char value[64];
	DWORD valueLength = 64;
	const char* subkey = "SOFTWARE\\Valve\\Steam";
	RegOpenKey(HKEY_CURRENT_USER, subkey, &hKey);
	RegQueryValueEx(hKey, "SteamPath", NULL, &dwType, (LPBYTE)&value, &valueLength);

	sprintf(value, std::string(value).c_str());

	return value;
}

void startTheGame() {
	std::string run = std::string("\"").append(global->steamPath).append("\\steam.exe\" ").append(global->gameArgs);

	WinExec(run.c_str(), SW_NORMAL);
}

void prepareConfig() {

	std::string options = "-applaunch 730 ";

	switch (cfg->res.displayMode) {
	default:
		options += std::string("");
		break;
	case 1:
		options += std::string("-windowed ");
		break;
	case 2:
		options += std::string("-fullscreen ");
		break;
	case 3:
		options += std::string("-windowed -noborder ");
		break;
	}

	if (cfg->insecure)
		options += "-insecure ";

	if (cfg->allowDebug)
		options += "-allowdebuging ";

	if (cfg->logConsole)
		options += "-condebug ";

	if (cfg->consoleOnStartup)
		options += "-console ";

	if (cfg->hijack)
		options += "-hijack ";

	if (cfg->dev)
		options += "-dev ";

	if (cfg->thirdParty)
		options += "-allow_third_party_software ";

	if (cfg->highPriority)
		options += "-high ";

	if (cfg->limitVSConst)
		options += "-limitvsconst ";

	if (cfg->forceNoVSync)
		options += "-forcenovsync ";

	if (cfg->emulateGL)
		options += "-r_emulate_g ";

	if (cfg->disableDX9Ex)
		options += "-disable_d3d9ex ";

	if (cfg->softParticlesDefaultOFF)
		options += "-softparticlesdefaultoff ";

	if (cfg->defaultCfg)
		options += "-autoconfig ";

	if (cfg->noAAFonts)
		options += "-noaafonts ";

	if (cfg->noHLTV)
		options += "-nohltv ";

	if (cfg->noPreload)
		options += "-nopreload ";

	if (cfg->noBrowser)
		options += "-no-browser ";

	if (cfg->noVideo)
		options += "-novid ";

	if (cfg->noJoystick)
		options += "-nojoy ";

	if (cfg->res.width)
		options += std::string("-w ").append(std::to_string(cfg->res.width)).append(" ");

	if (cfg->res.height)
		options += std::string("-h ").append(std::to_string(cfg->res.height)).append(" ");

	if (cfg->threads)
		options += std::string("-threads ").append(std::to_string(cfg->threads)).append(" ");

	if (cfg->tickrate)
		options += std::string("-tickrate ").append(std::to_string(cfg->tickrate)).append(" ");

	if (cfg->refresh)
		options += std::string("-refresh ").append(std::to_string(cfg->refresh)).append(" ");

	if (strlen(cfg->language))
		options += std::string("-language ").append(cfg->language).append(" ");

	if (strlen(cfg->execConfig))
		options += std::string("+exec ").append(cfg->execConfig).append(" ");

	if (strlen(cfg->game))
		options += std::string("-game ").append(cfg->game).append(" ");

	if (strlen(cfg->serverConnect))
		options += std::string("+connect ").append(cfg->serverConnect).append(" ");

	global->gameArgs = options;
}

char* wcharToChar(const wchar_t* pwchar)
{
    // get the number of characters in the string.
    int currentCharIndex = 0;
    char currentChar = pwchar[currentCharIndex];

    while (currentChar != '\0')
    {
        currentCharIndex++;
        currentChar = pwchar[currentCharIndex];
    }

    const int charCount = currentCharIndex + 1;

    // allocate a new block of memory size char (1 byte) instead of wide char (2 bytes)
    char* filePathC = (char*)malloc(sizeof(char) * charCount);

    for (int i = 0; i < charCount; i++)
    {
        // convert to char (1 byte)
        char character = pwchar[i];

        *filePathC = character;

        filePathC += sizeof(char);

    }
    filePathC += '\0';

    filePathC -= (sizeof(char) * charCount);

    return filePathC;
}


/* 
 * Since steam://run/<id>//<args>/ protocol doesn't really want to work with arguments.
 * I used method with calling steam.exe to open app with id 730 and giving him choosed in this program args in normal way.
 * For some reason steam runs app with also agrs given in app launch options causing small mess with them ¯\_(ツ)_/¯
 * 
 * My goal is to get profile steam id and removing given in launch option args for few seconds
 * and recover them back after the game runs at this moment. Don't see any other wait to fix that.
 */

int getActiveUserID() {
	DWORD dwType = REG_DWORD;
	HKEY hKey = 0;
	int value;
	DWORD valueLength = 64;
	const char* subkey = "SOFTWARE\\Valve\\Steam\\ActiveProcess";
	RegOpenKey(HKEY_CURRENT_USER, subkey, &hKey);
	RegQueryValueEx(hKey, "ActiveUser", NULL, &dwType, (LPBYTE)&value, &valueLength);

	return value;
}

void getConfigFileWithArgs(int appID) {
	std::string filePath = std::string(global->steamPath).append("\\userdata\\").append(std::to_string(getActiveUserID())).append("\\config\\localconfig.vdf");
	std::ifstream file(filePath);
}

bool isSteamRunning()
{

	PROCESSENTRY32 pe32 = { 0 };
	HANDLE    hSnap;
	int       iProccessID;
	bool      bProcessFound;

	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnap, &pe32);     // Can throw away, never an actual app

	bProcessFound = false;   //init values
	iProccessID = 1;

	while (iProccessID)    // go until out of Processes
	{
		iProccessID = Process32Next(hSnap, &pe32);
		if (!strcmp(pe32.szExeFile, "steam.exe"))    // Did we find our process?
		{
			bProcessFound = true;
			iProccessID = 0;
		}
	}
	return !bProcessFound;
}