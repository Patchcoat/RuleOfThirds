#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <math.h>
#include <iostream>
#include <sstream>
#define PI 3.141592654
#define RGB_(r, g, b) (DWORD)((r << 16) | (g << 8) | b)
#define DBOUT(s) \
{ \
	std::wostringstream os_; \
	os_ << s;\
	OutputDebugString(os_.str().c_str());\
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowOverlayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

LRESULT DestroyWindow();
void PaintControls(HWND hwnd);
void PaintOverlay(HWND hwnd);
void ChangeLineAlpha(HWND hwnd);
COLORREF ColorFromPoint(FLOAT x, FLOAT y);
void ClickHandler(POINT pt, HWND hwnd);

bool windowClosed = false;

HWND hwnd_overlay;

LONG overlayWidth;
LONG overlayHeight;

float lineAlpha = 0.3;

POINT center = {200, 200};
const LONG radius = 150;
const LONG radiusSquared = 150 * 150;
const LONG DIB_DEPTH = 32;
const LONG DIB_WIDTH = radius * 2;
const LONG DIB_HEIGHT = radius * 2;
LPBITMAPINFO lpBmi;
DWORD* pSurface;
DWORD lineColor = 0xFFFF00;
BYTE colorValue = 0xFF;

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

		CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,

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
	hwnd_overlay = CreateWindowEx(
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

	// Allocate memory for the color wheel
	lpBmi = (LPBITMAPINFO)malloc(sizeof(BITMAPINFO) + sizeof(DWORD) * 4);
	lpBmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	lpBmi->bmiHeader.biWidth = DIB_WIDTH;
	lpBmi->bmiHeader.biHeight = -(signed)DIB_HEIGHT;
	lpBmi->bmiHeader.biPlanes = 1;
	lpBmi->bmiHeader.biCompression = BI_RGB | BI_BITFIELDS;
	lpBmi->bmiHeader.biBitCount = 32;
	lpBmi->bmiHeader.biSizeImage = 0;
	lpBmi->bmiHeader.biXPelsPerMeter = 0;
	lpBmi->bmiHeader.biYPelsPerMeter = 0;
	lpBmi->bmiHeader.biClrUsed = 0;
	lpBmi->bmiHeader.biClrImportant = 0;

	DWORD* pBmi = (DWORD*)lpBmi->bmiColors;
	pBmi[0] = 0x00FF0000;
	pBmi[1] = 0x0000FF00;
	pBmi[2] = 0x000000FF;
	pBmi[3] = 0x00000000;

	pSurface = (DWORD*)malloc(sizeof(DWORD) * DIB_HEIGHT * DIB_WIDTH);

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

	// Cleanup
	free(pSurface);
	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	switch (uMsg)
	{
	case WM_DESTROY:
		return DestroyWindow();
	case WM_PAINT:
	{
		PaintControls(hwnd);
	}
	return 0;
	case WM_LBUTTONDOWN:
	{
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		ClickHandler(pt, hwnd);
	}
	return 0;
	case WM_MOUSEMOVE:
	{
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);
		if (wParam & MK_LBUTTON) {
			ClickHandler(pt, hwnd);
		}
	}
	return 0;
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

DWORD HSVtoRGB(LONG h, LONG s, LONG v) {
	if (s == 0) {
		return RGB_(v, v, v);
	}

	unsigned char region, remainder, p, q, t;

	region = h / 43;
	remainder = (h - (region * 43)) * 6;

	p = (v * (255 - s)) >> 8;
	q = (v * (255 - ((s * remainder) >> 8))) >> 8;
	t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

	switch (region) {
	case 0:
		return RGB_(v,t,p);
	case 1:
		return RGB_(q,v,p);
	case 2:
		return RGB_(p,v,t);
	case 3:
		return RGB_(p,q,v);
	case 4:
		return RGB_(t,p,v);
	default:
		return RGB_(v,p,q);
	}

	return RGB_(0,0,0);
}

DWORD ColorFromPoint(FLOAT x, FLOAT y) {
	float distance = (x * x + y * y) / radiusSquared * 255;
	if (distance > 255) {
		return (DWORD)0x00FFFFFF;
	}
	float angle = ((atan(x/y) + PI/2)/PI) * 255;
	return HSVtoRGB(angle, distance, colorValue);
}

void PaintControls(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	FillRect(hdc, &ps.rcPaint, (HBRUSH) CreateSolidBrush(RGB(255,255,255)));

	// Draw Color Wheel
	FLOAT distance = 0;
	FLOAT angle = 0;
	for (int i = -radius; i < radius; i++) {
		for (int j = -radius; j < radius; j++) {
			distance = (i * i + j * j) / radiusSquared;
			int iOffset = lpBmi->bmiHeader.biWidth * (i + radius) + (j + radius);
			if (distance > 1) {
				continue;
			}
			pSurface[iOffset] = ColorFromPoint(i, j);
		}
	}

	StretchDIBits(hdc, center.x - radius, center.y - radius, radius*2, radius*2, 0, 0, DIB_WIDTH, DIB_HEIGHT, (BYTE*)pSurface, lpBmi, DIB_RGB_COLORS, SRCCOPY);

	// draw alpha bar
	RECT alpha_bar;
	alpha_bar.left = 50;
	alpha_bar.top = 380;
	alpha_bar.right = 350;
	alpha_bar.bottom = 430;
	FillRect(hdc, &alpha_bar, (HBRUSH) CreateSolidBrush(RGB(0,0,0)));

	// draw color value bar
	RECT value_bar;
	value_bar.left = 380;
	value_bar.top = 50;
	value_bar.right = 430;
	value_bar.bottom = 350;
	FillRect(hdc, &value_bar, (HBRUSH) CreateSolidBrush(RGB(0,0,0)));

	EndPaint(hwnd, &ps);
}

float trueX = 75;
float trueY = -125;
void ClickHandler(POINT pt, HWND hwnd) {
	// Clicks in the color selector
	float X = pt.x - center.x;
	float Y = pt.y - center.y;
	float distance = (X * X + Y * Y) / radiusSquared;
	if (distance < 1) {
		trueX = X;
		trueY = Y;
		DBOUT("X: " << X << " Y: " << Y << "\n");
		lineColor = ColorFromPoint(Y, X);
		RedrawWindow(hwnd_overlay, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_INTERNALPAINT | RDW_VALIDATE);
		UpdateWindow(hwnd_overlay);
		InvalidateRect(hwnd_overlay, NULL, true);
	}

	// Clicks in the Alpha selector
	if (pt.x > 50 && pt.x < 350 &&
		pt.y > 380 && pt.y < 430) {
		lineAlpha = (pt.x - 50.0f) / 300.0f;
		ChangeLineAlpha(hwnd_overlay);
	}

	// Clicks in the Value selector
	if (pt.x > 380 && pt.x < 430 &&
		pt.y > 50 && pt.y < 350) {
		colorValue = (1 - (pt.y - 50.0f) / 300.0f) * 255;
		lineColor = ColorFromPoint(trueY, trueX);
		RedrawWindow(hwnd_overlay, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_INTERNALPAINT | RDW_VALIDATE);
		UpdateWindow(hwnd_overlay);
		InvalidateRect(hwnd_overlay, NULL, true);
	}

#if 0
	RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_INTERNALPAINT | RDW_VALIDATE);
	UpdateWindow(hwnd);
	InvalidateRect(hwnd, NULL, true);
#endif
}

void line(HDC hdc, HBRUSH hbr, LONG width, LONG height, LONG startx, LONG starty, LONG endx, LONG endy) {
	const RECT line = {startx - width/2, starty - height/2, endx + width/2, endy + height/2};
	FillRect(hdc, &line, hbr);
}

void PaintOverlay(HWND hwnd) {
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	FillRect(hdc, &ps.rcPaint, (HBRUSH)CreateSolidBrush(RGB(255, 0, 255)));
	BYTE red = (lineColor & 0xFF0000) >> 16;
	BYTE green = (lineColor & 0x00FF00) >> 8;
	BYTE blue = lineColor & 0x0000FF;
	HBRUSH hbr = CreateSolidBrush(RGB(red, green, blue));
	LONG widthThird = overlayWidth / 3;
	LONG heightThird = overlayHeight / 3;
	line(hdc, hbr, 4, 0,    widthThird,   0, widthThird,   overlayHeight);
	line(hdc, hbr, 4, 0,    2*widthThird, 0, 2*widthThird, overlayHeight);
	line(hdc, hbr, 0, 4, 0, heightThird,     overlayWidth, heightThird);
	line(hdc, hbr, 0, 4, 0, 2*heightThird,   overlayWidth, 2*heightThird);

	EndPaint(hwnd, &ps);
}

void ChangeLineAlpha(HWND hwnd) {
	SetLayeredWindowAttributes(hwnd, RGB(255, 0, 255), 255 * lineAlpha, LWA_COLORKEY | LWA_ALPHA);
}