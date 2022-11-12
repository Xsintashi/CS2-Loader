#pragma once

#include <string>

const char* getSteamPath();
void startTheGame();

void prepareConfig();

void getConfigFileWithArgs(int appID);

bool isSteamRunning();

char* wcharToChar(const wchar_t* pwchar);
