# CS:GO Loader

[![C++](https://img.shields.io/badge/language-C%2B%2B-%23f34b7d.svg?style=for-the-badge)](https://en.wikipedia.org/wiki/C%2B%2B) [![CS:GO](https://img.shields.io/badge/game-CS%3AGO-yellow.svg?style=for-the-badge)](https://store.steampowered.com/app/730/CounterStrike_Global_Offensive/) [![License](https://img.shields.io/github/license/Xsintashi/CSGO-Loader?style=for-the-badge)](LICENSE)![menu](https://raw.githubusercontent.com/Xsintashi/CSGO-Loader/main/assets/menu.png)

Free open-source Loader software for Counter-Strike: Global Offensive game. Compatible with the Steam version of the game. Available for Windows.

# Acknowledgments
[Cazz](https://github.com/cazzwastaken) - for ImGui Borderless Window Code Snippet
[ocornut](https://github.com/ocornut) - for ImGui itself
[nlohmann](https://github.com/nlohmann) - for json lib

# Common Issues

 * Since steam://run/<id>//<args>/ protocol doesn't really want to work with arguments. I used method with calling steam.exe to open app with id 730 and giving him choosed in this program args in normal way. For some reason steam runs app with also agrs given in app launch options causing small mess with them ¯\\_(ツ)\_/¯
