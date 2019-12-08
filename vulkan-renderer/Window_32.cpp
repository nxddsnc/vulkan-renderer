#include "BUILD_OPTIONS.h"
#include "Platform.h"
#include "Window.h"
#include <assert.h>
#include "Renderer.h"
#include "Shared.h"
#ifdef VK_USE_PLATFORM_WIN32_KHR

// Microsoft Windows specific versions of window functions
LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Window * window = reinterpret_cast<Window*>(
		GetWindowLongPtrW(hWnd, GWLP_USERDATA));

	switch (uMsg) {
	case WM_CLOSE:
		window->Close();
		return 0;
	case WM_SIZE:
		// we get here if the window has changed size, we should rebuild most
		// of our window resources before rendering to this window again.
		// ( no need for this because our window sizing by hand is disabled )
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

uint64_t	Window::_win32ClassIdCounter = 0;

void Window::_initOSWindow()
{
	WNDCLASSEX win_class{};
	assert(_width > 0);
	assert(_height > 0);

	_win32Instance = GetModuleHandle(nullptr);
	_win32ClassName = _windowName + "_" + std::to_string(_win32ClassIdCounter);
	_win32ClassIdCounter++;

	// Initialize the window class structure:
	win_class.cbSize = sizeof(WNDCLASSEX);
	win_class.style = CS_HREDRAW | CS_VREDRAW;
	win_class.lpfnWndProc = WindowsEventHandler;
	win_class.cbClsExtra = 0;
	win_class.cbWndExtra = 0;
	win_class.hInstance = _win32Instance; // hInstance
	win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
	win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	win_class.lpszMenuName = NULL;
	win_class.lpszClassName = _win32ClassName.c_str();
	win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	// Register window class:
	if (!RegisterClassEx(&win_class)) {
		// It didn't work, so try to give a useful error:
		assert(0 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	// Create window with the registered class:
	RECT wr = { 0, 0, LONG(_width), LONG(_height) };
	AdjustWindowRectEx(&wr, style, FALSE, ex_style);
	_win32Window = CreateWindowEx(0,
		_win32ClassName.c_str(),		// class name
		_windowName.c_str(),			// app name
		style,							// window style
		CW_USEDEFAULT, CW_USEDEFAULT,	// x/y coords
		wr.right - wr.left,				// width
		wr.bottom - wr.top,				// height
		NULL,							// handle to parent
		NULL,							// handle to menu
		_win32Instance,				// hInstance
		NULL);							// no extra parameters
	if (!_win32Window) {
		// It didn't work, so try to give a useful error:
		assert(1 && "Cannot create a window in which to draw!\n");
		fflush(stdout);
		std::exit(-1);
	}
	SetWindowLongPtr(_win32Window, GWLP_USERDATA, (LONG_PTR)this);

	ShowWindow(_win32Window, SW_SHOW);
	SetForegroundWindow(_win32Window);
	SetFocus(_win32Window);
}

void Window::_deInitOSWindow()
{
	DestroyWindow(_win32Window);
	UnregisterClass(_win32ClassName.c_str(), _win32Instance);
}

void Window::_updateOSWindow()
{
	MSG msg;
	if (PeekMessage(&msg, _win32Window, 0, 0, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::InitOSSurface(VkInstance instance, VkSurfaceKHR *surface)
{
	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfo = {};
	win32SurfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfo.hinstance = _win32Instance;
	win32SurfaceCreateInfo.hwnd = _win32Window;
	vkCreateWin32SurfaceKHR(instance, &win32SurfaceCreateInfo, nullptr, surface);
}

#endif