#include "gui.h"
#include "Settings.h"

#include <Windows.h>
#include <WinUser.h>

#include "Utils.h"
#include "Global.h"

#include <thread> // for cpu threads counter

#include "../imgui/imgui.h"
#include "../imgui/imgui_stdlib.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#define sameLine ImGui::SameLine();
#define push(val) ImGui::PushID(val);
#define pop() ImGui::PopID();
#define title "Counter-Strike: Global Offensive Loader"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (GUI::device && wideParameter != SIZE_MINIMIZED)
		{
			GUI::presentParameters.BackBufferWidth = LOWORD(longParameter);
			GUI::presentParameters.BackBufferHeight = HIWORD(longParameter);
			GUI::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		GUI::position = MAKEPOINTS(longParameter); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(GUI::window, &rect);

			rect.left += points.x - GUI::position.x;
			rect.top += points.y - GUI::position.y;

			if (GUI::position.x >= 0 &&
				GUI::position.x <= GUI::width &&
				GUI::position.y >= 0 && GUI::position.y <= 19)
				SetWindowPos(
					GUI::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void GUI::Minimalize() noexcept {
	ShowWindow(window, SW_MINIMIZE);
}

void GUI::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP, //WS_SYSMENU with win10 title bar
		100,
		100,
		width,
		height,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void GUI::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool GUI::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void GUI::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void GUI::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void GUI::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsVGUI();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void GUI::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void GUI::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void GUI::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

void GUI::Render() noexcept
{
	constexpr int spacing = 112;
	int processorCount = std::thread::hardware_concurrency();
	
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ width, height });
	static int flags = (ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
	ImGui::Begin(title, nullptr, flags);
	ImGui::Text(title); sameLine ImGui::SetCursorPosX(width  - 48);
		if (ImGui::Button("_", { 16.f, 16.f })) Minimalize();  sameLine
		if (ImGui::Button("X", { 16.f, 16.f })) isRunning = false;
	ImGui::Columns(2, nullptr, false);
	ImGui::PushItemWidth(48);
	ImGui::Text("Display");
	ImGui::BeginChild("Display", { 208.f, 80.f }, true, ImGuiWindowFlags_NoScrollbar);
	ImGui::Text("Width"); ImGui::SameLine(spacing - 4);
	ImGui::SetNextItemWidth(48);
	ImGui::InputInt("##width", &cfg->res.width, 0, 0);
	ImGui::Text("Height"); ImGui::SameLine(spacing - 4);
	ImGui::SetNextItemWidth(48);
	ImGui::InputInt("##height", &cfg->res.height, 0, 0);
	ImGui::Text("Mode"); ImGui::SameLine(spacing - 4);
	ImGui::SetNextItemWidth(96);
	ImGui::Combo("##display", &cfg->res.displayMode, "Default\0Windowed\0Fullscreen\0Windowed Fullscreen\0");
	ImGui::EndChild();

	ImGui::Text("Debug");
	ImGui::BeginChild("Debug", { 208.f, 176.f }, true, ImGuiWindowFlags_NoScrollbar);

	push("Insecure");
	ImGui::Checkbox("Insecure", &cfg->insecure);

	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Your game files signatures will not be validated\nand you will be not allowed to join this VAC secure server.");
	pop()

	push("Allow Debuging");
	ImGui::Checkbox("Allow Debuging", &cfg->allowDebug);
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Allows to debug the game.");
	pop()

	push("Log Console");
	ImGui::Checkbox("Log Console", &cfg->logConsole);
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Game will log console in ../csgo/console.log.");
	pop()

	push("Show Console");
	ImGui::Checkbox("Console on Startup", &cfg->consoleOnStartup);

	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Makes the game start with the console opened.");
	pop()

	push("Hijack");
	ImGui::Checkbox("Hijack", &cfg->hijack);
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Hijacks the game.");
	pop()

	push("Dev");
	ImGui::Checkbox("Dev", &cfg->dev);

	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("dev.");
	pop()

	push("Game");
	ImGui::Text("Game"); ImGui::SameLine(spacing - 8);
	ImGui::SetNextItemWidth(64);
	ImGui::InputText("##game", cfg->game, sizeof(cfg->game));
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Folder of the game.");
	pop()

	ImGui::EndChild();

	push("Threads");
	ImGui::Text("Threads"); ImGui::SameLine(spacing);
	ImGui::InputInt("##threads", &cfg->threads, 0, 0);
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sets the amount of processor threads that CS:GO will use.");
	cfg->threads = std::clamp(cfg->threads, 0, processorCount);
	pop()

	push("Tickrate");
	ImGui::Text("Tickrate"); ImGui::SameLine(spacing);
	ImGui::InputInt("##tickrate", &cfg->tickrate, 0, 0);
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sets the tick rate of any \"Offline With Bots\" games,\nor any servers that you host via your client.");
	pop()

	push("Refresh");
	ImGui::Text("Refresh"); ImGui::SameLine(spacing);
	ImGui::InputInt("##refresh", &cfg->refresh, 0, 0);
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sets the refresh rate of your client. The refresh rate (in hz) is\nhow many times per second your monitor will update - at 60hz,\nyour monitor is effectively showing 60 frames per second. For 144hz monitors,\nset this to 144 so that your game refreshes 144 times a second.");
	pop()

	if (ImGui::Button("Save")) {
		Set::Save("test.cfg");
	}
	sameLine
	if (ImGui::Button("Load")) {
		Set::Load("test.cfg");
	}

	ImGui::NextColumn();				/*					NEXT COLUMN					*/

	ImGui::Dummy({ 0.f, 16.f });
	push("Config");
	ImGui::Text("Execute Config"); ImGui::SameLine(spacing);
	ImGui::SetNextItemWidth(120);
	ImGui::InputText("##exec", cfg->execConfig, sizeof(cfg->execConfig));
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("This launch option will execute all commands a specified file.");
	pop()

	push("Language");
	ImGui::Text("Language"); ImGui::SameLine(spacing);
	ImGui::SetNextItemWidth(120);
	ImGui::InputText("##lang", cfg->language, sizeof(cfg->language));
	sameLine
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Makes CS:GO start in the language that you specified.");
	pop()

	push("Server");
	ImGui::Text("Server"); ImGui::SameLine(spacing);
	ImGui::SetNextItemWidth(120);
	ImGui::InputText("##server", cfg->serverConnect, sizeof(cfg->serverConnect));
	sameLine
		ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Connects with the server automatically.");
	pop()

	ImGui::Checkbox("Allow Third Party Software", &cfg->thirdParty);
	ImGui::Checkbox("High Priority", &cfg->highPriority);
	ImGui::Checkbox("Limit Vertex Shaders", &cfg->limitVSConst);
	ImGui::Checkbox("Force NoVSync", &cfg->forceNoVSync);
	ImGui::Checkbox("Emulate GL", &cfg->emulateGL);
	ImGui::Checkbox("Disable DX9Ex", &cfg->disableDX9Ex);
	ImGui::Checkbox("Disable Soft Particles on Default", &cfg->softParticlesDefaultOFF);
	ImGui::Checkbox("Default Config on Startup", &cfg->defaultCfg);
	ImGui::Checkbox("Disable Anti-Aliasing Fonts", &cfg->noAAFonts);
	ImGui::Checkbox("Disable HLTV", &cfg->noHLTV);
	ImGui::Checkbox("Disable Preload", &cfg->noPreload);
	ImGui::Checkbox("Disable Browser", &cfg->noBrowser);
	ImGui::Checkbox("Disable Intro", &cfg->noVideo);
	ImGui::Checkbox("Disable Joystick Support", &cfg->noJoystick);

	ImGui::PopItemWidth();

	ImGui::Columns(1);

	//ImGui::Dummy({ 0.f, 16.f });
	ImGui::Separator();
	prepareConfig();
	ImGui::SetNextItemWidth(width - static_cast<int>(ImGui::CalcTextSize("Start").x) - 32);
	ImGui::InputText("##output", &global->gameArgs, ImGuiInputTextFlags_ReadOnly);
	sameLine
	if (ImGui::Button("Start")) {
		startTheGame();
	}
	ImGui::End();
}
