#include "imgui_opengl_hook.h"

static bool open = false;
static bool close = false;
static funcWglSwapBuffers OwglSwapBuffers;
static WNDPROC oWndProc;
static HWND hwnd = nullptr;
static HGLRC newContext;
static HGLRC oContext;
static DWORD KEY;
static SwapBufferCallback sbcallback = nullptr;
static RENDER renderCallback = nullptr;
static void* target;
static CONFIG_CALLBACK config_func = nullptr;

void initHook()
{
	MH_Initialize();
	target = GetProcAddress(GetModuleHandle(TEXT("opengl32.dll")), "wglSwapBuffers");
	MH_CreateHook(target, hookSwapBuffers, (void**) & OwglSwapBuffers);
	MH_EnableHook(target);

}

bool hookSwapBuffers(HDC hdc)
{
    if (hwnd == nullptr) //初始化
    {
        hwnd = WindowFromDC(hdc);
        IMGUI_CHECKVERSION();

        newContext = wglCreateContext(hdc);

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;    // Enable Gamepad Controls

        if (config_func != nullptr)
        {
            config_func(io);
        }

        ImGui_ImplWin32_InitForOpenGL(hwnd);
        ImGui_ImplOpenGL3_Init();

        oWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)WndProc);


    }
    if (open)
    {//渲染
        oContext = wglGetCurrentContext();
        wglMakeCurrent(hdc, newContext);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (renderCallback != nullptr)
        {
            renderCallback();
        }
        


        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        wglMakeCurrent(hdc, oContext);

    }

    if (sbcallback != nullptr)
    {
        sbcallback(hdc);
    }
    
    auto back = OwglSwapBuffers(hdc); //调用源函数
    return back;
}


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (GetAsyncKeyState(KEY) & 1)
    {
        open = !open;
    }
    if (open)
    {
        if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
            return 1L;
    }

    return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam); //调用源窗口过程


}


void SetGui(RENDER callback, DWORD button)
{
    renderCallback = callback;
    KEY = button;
}

void SetSwapHook(SwapBufferCallback callback)
{
    sbcallback = callback;
}


void deleteHook()
{
    if (!close)
    {
        close = true;
        open = false;
        Sleep(10);
        MH_DisableHook(MH_ALL_HOOKS);
        MH_RemoveHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)oWndProc);
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        wglDeleteContext(newContext);
    }
    

}

void SetConfig(CONFIG_CALLBACK callback)
{
    config_func = callback;
}
