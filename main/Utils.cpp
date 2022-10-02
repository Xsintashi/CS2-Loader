#include "Utils.h"
#include <Windows.h>
#include "Global.h"

const char* getSteamPath() {
	DWORD dwType = REG_SZ;
	HKEY hKey = 0;
	char value[64];
	DWORD value_length = 64;
	const char* subkey = "SOFTWARE\\Valve\\Steam";
	RegOpenKey(HKEY_CURRENT_USER, subkey, &hKey);
	RegQueryValueEx(hKey, "SteamExe", NULL, &dwType, (LPBYTE)&value, &value_length);

	sprintf(value, std::string(value).c_str());

	return value;
}

void startTheGame() {
	std::string run = std::string("\"").append(getSteamPath()).append("\" ").append(global->output);

	while (run.find("/") != std::string::npos) // replace #p with name of killed player
		run.replace(run.find("/"), 1, "\\");

	WinExec(run.c_str(), SW_NORMAL);
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