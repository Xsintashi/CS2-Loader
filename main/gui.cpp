#include "gui.h"
#include "Settings.h"

#include <Windows.h>
#include <WinUser.h>

#include "Utils.h"

#include <thread> // for cpu threads counter

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

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

	std::string options = "-steam ";
	constexpr int spacing = 112;
	int processorCount = std::thread::hardware_concurrency();
	
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ width, height });
	static int flags = (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
	ImGui::Begin("Counter-Strike: Global Offensive", &isRunning, flags);
	ImGui::Text("Path"); ImGui::SameLine();
	ImGui::SetNextItemWidth(width - static_cast<int>(ImGui::CalcTextSize("Path").x) - 4);
	ImGui::InputText("##path", cfg->path, sizeof(cfg->path));

	ImGui::Separator();

	ImGui::Columns(2, nullptr, false);
	ImGui::PushItemWidth(48);
	ImGui::Text("Display");
	ImGui::BeginChild("Display", { 208.f, 80.f }, true, ImGuiWindowFlags_NoScrollbar);
	ImGui::Text("Width"); ImGui::SameLine(spacing - 4);
	ImGui::InputInt("##width", &cfg->res.width, 0, 0);
	ImGui::Text("Height"); ImGui::SameLine(spacing - 4);
	ImGui::InputInt("##height", &cfg->res.height, 0, 0);
	ImGui::Text("Mode"); ImGui::SameLine(spacing - 4);
	ImGui::SetNextItemWidth(96);
	ImGui::Combo("##display", &cfg->res.displayMode, "Windowed\0Fullscreen\0Windowed Fullscreen\0");
	ImGui::EndChild();

	ImGui::Text("Debug");
	ImGui::BeginChild("Debug", { 208.f, 176.f }, true, ImGuiWindowFlags_NoScrollbar);

	ImGui::PushID("Insecure");
	if(ImGui::Checkbox("Insecure", &cfg->insecure))
		options += "-insecure ";

	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Your game files signatures will not be validated\nand you will be not allowed to join this VAC secure server.");
	ImGui::PopID();

	ImGui::PushID("Allow Debuging");
	if(ImGui::Checkbox("Allow Debuging", &cfg->allowDebug))
		options += "-allowdebuging ";
	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Allows to debug the game.");
	ImGui::PopID();

	ImGui::PushID("Log Console");
	if (ImGui::Checkbox("Log Console", &cfg->logConsole))
		options += "-condebug ";

	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Game will log console in ../csgo/console.log.");
	ImGui::PopID();

	ImGui::PushID("Show Console");
	if (ImGui::Checkbox("Console on Startup", &cfg->consoleOnStartup))
		options += "-console ";

	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Makes the game start with the console opened.");
	ImGui::PopID();

	ImGui::PushID("Hijack");
	if (ImGui::Checkbox("Hijack", &cfg->hijack))
		options += "-hijack ";

	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Hijacks the game.");
	ImGui::PopID();

	ImGui::PushID("Dev");
	if (ImGui::Checkbox("Dev", &cfg->dev))
		options += "-dev ";

	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("dev.");
	ImGui::PopID();

	ImGui::PushID("Game");
	ImGui::Text("Game"); ImGui::SameLine(spacing - 4);
	ImGui::InputText("##game", cfg->game, sizeof(cfg->game));
	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Uses this folder of the game.");
	ImGui::PopID();

	ImGui::EndChild();

	ImGui::PushID("Threads");
	ImGui::Text("Threads"); ImGui::SameLine(spacing);
	ImGui::InputInt("##threads", &cfg->threads, 0, 0);
	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sets the amount of processor threads that CS:GO will use.");
	cfg->threads = std::clamp(cfg->threads, 0, processorCount);
	ImGui::PopID();

	ImGui::PushID("Tickrate");
	ImGui::Text("Tickrate"); ImGui::SameLine(spacing);
	ImGui::InputInt("##tickrate", &cfg->tickrate, 0, 0);
	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sets the tick rate of any \"Offline With Bots\" games,\nor any servers that you host via your client.");
	ImGui::PopID();

	ImGui::PushID("Refresh");
	ImGui::Text("Refresh"); ImGui::SameLine(spacing);
	ImGui::InputInt("##refresh", &cfg->refresh, 0, 0);
	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Sets the refresh rate of your client. The refresh rate (in hz) is\nhow many times per second your monitor will update - at 60hz,\nyour monitor is effectively showing 60 frames per second. For 144hz monitors,\nset this to 144 so that your game refreshes 144 times a second.");
	ImGui::PopID();

	ImGui::NextColumn();

	ImGui::PushID("Config");
	ImGui::Text("Execute Config"); ImGui::SameLine(spacing);
	ImGui::SetNextItemWidth(120);
	ImGui::InputText("##exec", cfg->execConfig, sizeof(cfg->execConfig));
	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("This launch option will execute all commands a specified file.");
	ImGui::PopID();

	ImGui::PushID("Language");
	ImGui::Text("Language"); ImGui::SameLine(spacing);
	ImGui::SetNextItemWidth(120);
	ImGui::InputText("##lang", cfg->language, sizeof(cfg->language));
	ImGui::SameLine();
	ImGui::TextDisabled("?");
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Makes CS:GO start in the language that you specified.");
	ImGui::PopID();


	if (ImGui::Checkbox("Allow Third Party Software", &cfg->thirdParty))
		options += "-allow_third_party_software ";

	if (ImGui::Checkbox("High Priority", &cfg->highPriority))
		options += "-high ";

	if (ImGui::Checkbox("Limit Vector Shaders", &cfg->limitVSConst))
		options += "-limitvsconst ";

	if (ImGui::Checkbox("Force NoVSync", &cfg->forceNoVSync))
		options += "-forcenovsync ";

	if (ImGui::Checkbox("Emulate GL", &cfg->emulateGL))
		options += "-r_emulate_g ";

	if (ImGui::Checkbox("Disable DX9Ex", &cfg->disableDX9Ex))
		options += "-disable_d3d9ex ";

	if (ImGui::Checkbox("Disable Soft Particles on Default", &cfg->softParticlesDefaultOFF))
		options += "-softparticlesdefaultoff ";

	if (ImGui::Checkbox("Default Config on Startup", &cfg->defaultCfg))
		options += "-autoconfig ";

	if (ImGui::Checkbox("Disable Anti-Aliasing Fonts", &cfg->noAAFonts))
		options += "-noaafonts ";

	if (ImGui::Checkbox("Disable HLTV", &cfg->noHLTV))
		options += "-nohltv ";

	if (ImGui::Checkbox("Disable Preload", &cfg->noPreload))
		options += "nopreload ";

	if (ImGui::Checkbox("Disable Browser", &cfg->noBrowser))
		options += "-no-browser ";

	if (ImGui::Checkbox("Disable Intro", &cfg->noVideo))
		options += "-novid ";

	if (ImGui::Checkbox("Disable Joystick Support", &cfg->noJoystick))
		options += "-nojoy ";

	if (cfg->res.width)
	options += std::string("-w ").append(std::to_string(cfg->res.width )).append(" ");

	if (cfg->res.height)
	options += std::string("-h ").append(std::to_string(cfg->res.height)).append(" ");

	if (cfg->threads)
		options += std::string("-threads ").append(std::to_string(cfg->threads)).append(" ");

	if (cfg->tickrate)
		options += std::string("-tickrate ").append(std::to_string(cfg->tickrate)).append(" ");

	if (cfg->refresh)
		options += std::string("-refresh ").append(std::to_string(cfg->refresh)).append(" ");

	if(strlen(cfg->language))
		options += std::string("-language ").append(cfg->language).append(" ");

	if (strlen(cfg->execConfig))
		options += std::string("+exec ").append(cfg->execConfig).append(" ");

	if (strlen(cfg->game))
		options += std::string("-game ").append(cfg->game).append(" ");

	switch (cfg->res.displayMode) {
		default:	
		case 1:
			options += std::string("-fullscreen ");
			break;
		case 0:
			options += std::string("-windowed ");
			break;
		case 2:
			options += std::string("-noborder ");
			break;
	}

	ImGui::PopItemWidth();

	ImGui::Columns(1);

	ImGui::Separator();

	char* output = &options[0];
	strcpy(output, options.c_str());

	ImGui::SetNextItemWidth(width - static_cast<int>(ImGui::CalcTextSize("Start").x) - 32);
	ImGui::InputText("##output", output, ImGuiInputTextFlags_ReadOnly );
	ImGui::SameLine();
	if (ImGui::Button("Start")) {
		std::string run = std::string("\"").append(cfg->path).append("\" ").append(output);

		while (run.find("/") != std::string::npos) // replace #p with name of killed player
			run.replace(run.find("/"), 1, "\\");

		WinExec(run.c_str(), SW_NORMAL);
	}
	ImGui::End();
}
