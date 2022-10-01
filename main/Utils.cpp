#include "Utils.h"
#include <Windows.h>

const char* getSteamPath() {
	DWORD dwType = REG_SZ;
	HKEY hKey = 0;
	char value[64];
	DWORD value_length = 64;
	const char* subkey = "SOFTWARE\\Valve\\Steam";
	RegOpenKey(HKEY_CURRENT_USER, subkey, &hKey);
	RegQueryValueEx(hKey, "SteamPath", NULL, &dwType, (LPBYTE)&value, &value_length);

	sprintf(value, std::string(value).append("/steamapps/common/Counter - Strike Global Offensive/csgo.exe").c_str());

	return value;
}