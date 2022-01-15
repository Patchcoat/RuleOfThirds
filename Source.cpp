#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowOverlayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT DestroyWindow();
void PaintControls(HWND hwnd);
void PaintOverlay(HWND hwnd);
void changeLineAlpha(HWND hwnd);

bool windowClosed = false;

LONG overlayWidth;
LONG overlayHeight;

float lineAlpha = 0.3;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	bool endProgram = false;

	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Rule Of Thirds Window";

	WNDCLASS wc = {};

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Register the overlay
	const wchar_t CLASS_NAME_OVERLAY[] = L"Rule Of Thirds Overlay";

	WNDCLASS wc_overlay = {};

	wc_overlay.lpfnWndProc = WindowOverlayProc;
	wc_overlay.hInstance = hInstance;
	wc_overlay.lpszClassName = CLASS_NAME_OVERLAY;

	RegisterClass(&wc_overlay);

	// Create the Window.

	HWND hwnd = CreateWindowEx(
		0,
		CLASS_NAME,
		L"Rule Of Thirds Overlay",
		WS_OVERLAPPEDWINDOW,

		CW_USEDEFAULT, CW_USEDEFAULT, 350, 100,

		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, nCmdShow);

	// Create the Overlay
	HMONITOR hmon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = { sizeof(mi) };
	if (!GetMonitorInfo(hmon, &mi))
	{
		return 0;
	}
	HWND hwnd_overlay = CreateWindowEx(
		WS_EX_TOPMOST,
		CLASS_NAME_OVERLAY,
		L"ROTO",
		WS_POPUP | WS_VISIBLE,

		mi.rcMonitor.left, mi.rcMonitor.top,
		mi.rcMonitor.right - mi.rcMonitor.left,
		mi.rcMonitor.bottom - mi.rcMonitor.top,

		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (hwnd_overlay == NULL) 
	{
		return 0;
	}

	SetWindowLong(hwnd_overlay, GWL_EXSTYLE, GetWindowLong(hwnd_overlay, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_SIZEBOX);
	SetLayeredWindowAttributes(hwnd_overlay, RGB(255, 0, 255), 255 * lineAlpha, LWA_COLORKEY | LWA_ALPHA);
	ShowWindow(hwnd_overlay, nCmdShow);

	// Message Handler
	MSG msg = {};
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (windowClosed == true) {
			break;
		}
	}

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		return DestroyWindow();
	case WM_PAINT:
	{
		PaintControls(hwnd);
	}
	return 0;
	// TODO handle WM_LBUTTONDOWN and WM_LBUTTONUP events
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK WindowOverlayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		return DestroyWindow();
	case WM_PAINT:
	{
		PaintOverlay(hwnd);
	}
	return 0;
	case WM_SIZE:
		RECT size;
		GetWindowRect(hwnd, &size);
		overlayWidth = size.right - size.left;
		overlayHeight = size.bottom - size.top;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT DestroyWindow() {
	PostQuitMessage(0);
	windowClosed = true;
	return 0;
}

void PaintControls(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	FillRect(hdc, &ps.rcPaint, (HBRUSH) CreateSolidBrush(RGB(255,255,255)));

	// TODO add draw color controls

	EndPaint(hwnd, &ps);
}

void line(HDC hdc, HBRUSH hbr, LONG width, LONG height, LONG startx, LONG starty, LONG endx, LONG endy) {
	const RECT line = {startx - width/2, starty - height/2, endx + width/2, endy + height/2};
	FillRect(hdc, &line, hbr);
}

void PaintOverlay(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	FillRect(hdc, &ps.rcPaint, (HBRUSH) CreateSolidBrush(RGB(255,0,255)));
	HBRUSH hbr = CreateSolidBrush(RGB(255, 255, 0));
	LONG widthThird = overlayWidth / 3;
	LONG heightThird = overlayHeight / 3;
	line(hdc, hbr, 4, 0,    widthThird,   0, widthThird,   overlayHeight);
	line(hdc, hbr, 4, 0,    2*widthThird, 0, 2*widthThird, overlayHeight);
	line(hdc, hbr, 0, 4, 0, heightThird,     overlayWidth, heightThird);
	line(hdc, hbr, 0, 4, 0, 2*heightThird,   overlayWidth, 2*heightThird);

	EndPaint(hwnd, &ps);
}

void changeLineAlpha(HWND hwnd) {
	SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 255 * lineAlpha, LWA_COLORKEY | LWA_ALPHA);
}