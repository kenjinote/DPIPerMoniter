#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include <windows.h>
#include "resource.h"

TCHAR szClassName[] = TEXT("Window");
#define DEFAULT_DPI 96

BOOL GetScaling(HWND hWnd, UINT* pnX, UINT* pnY)
{
	BOOL bSetScaling = FALSE;
	const HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
	if (hMonitor)
	{
		HMODULE hShcore = LoadLibrary(TEXT("SHCORE"));
		if (hShcore)
		{
			typedef HRESULT __stdcall GetDpiForMonitor(HMONITOR, int, UINT*, UINT*);
			GetDpiForMonitor* fnGetDpiForMonitor = reinterpret_cast<GetDpiForMonitor*>(GetProcAddress(hShcore, "GetDpiForMonitor"));
			if (fnGetDpiForMonitor)
			{
				UINT uDpiX, uDpiY;
				HRESULT hr = fnGetDpiForMonitor(hMonitor, 0, &uDpiX, &uDpiY);
				if (SUCCEEDED(hr) && uDpiX > 0 && uDpiY > 0)
				{
					*pnX = uDpiX;
					*pnY = uDpiY;
					bSetScaling = TRUE;
				}
			}
			FreeLibrary(hShcore);
		}
	}
	return bSetScaling;
}

#define SCALEX(X) MulDiv(X, uDpiX, DEFAULT_DPI)
#define SCALEY(Y) MulDiv(Y, uDpiY, DEFAULT_DPI)
#define POINT2PIXEL(PT) MulDiv(PT, uDpiY, 72)

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit, hButton, hStatic;
	static HBITMAP hBitmap;
	static UINT uDpiX = DEFAULT_DPI, uDpiY = DEFAULT_DPI;
	static HFONT hFont;
	switch (msg)
	{
	case WM_CREATE:
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), TEXT("あいうえお"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("ボタン"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hStatic = CreateWindow(TEXT("STATIC"), TEXT("あいうえお"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hBitmap = LoadBitmap(((LPCREATESTRUCT)lParam)->hInstance, MAKEINTRESOURCE(IDB_BITMAP1));
		SendMessage(hWnd, WM_DPICHANGED, 0, 0);
		break;
	case WM_NCCREATE:
		{
			typedef BOOL(WINAPI*fnTypeEnableNCScaling)(HWND);
			const HMODULE hModUser32 = GetModuleHandle(TEXT("user32.dll"));
			if (hModUser32)
			{
				const fnTypeEnableNCScaling fnEnableNCScaling = (fnTypeEnableNCScaling)GetProcAddress(hModUser32, "EnableNonClientDpiScaling");
				if (fnEnableNCScaling)
				{
					fnEnableNCScaling(hWnd);
				}
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			const HDC hdc = BeginPaint(hWnd, &ps);
			const HDC hdcMem = CreateCompatibleDC(hdc);
			SelectObject(hdcMem, hBitmap);
			BITMAP bm;
			GetObject(hBitmap, sizeof(BITMAP), &bm);
			StretchBlt(hdc, SCALEX(10), SCALEY(130), SCALEX(bm.bmWidth), SCALEY(bm.bmHeight), hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
			DeleteDC(hdcMem);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_DPICHANGED:
		if (!GetScaling(hWnd, &uDpiX, &uDpiY))
		{
			HDC hdc = GetDC(NULL);
			if (hdc)
			{
				uDpiX = GetDeviceCaps(hdc, LOGPIXELSX);
				uDpiY = GetDeviceCaps(hdc, LOGPIXELSY);
				ReleaseDC(NULL, hdc);
			}
		}
		DeleteObject(hFont);
		hFont = CreateFont(-POINT2PIXEL(10), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, TEXT("メイリオ"));
		SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, 0);
		SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, 0);
		SendMessage(hStatic, WM_SETFONT, (WPARAM)hFont, 0);
		MoveWindow(hEdit, SCALEX(10), SCALEY(10), SCALEX(256), SCALEY(32), FALSE);
		MoveWindow(hButton, SCALEX(10), SCALEY(50), SCALEX(256), SCALEY(32), FALSE);
		MoveWindow(hStatic, SCALEX(10), SCALEY(90), SCALEX(256), SCALEY(32), FALSE);
		InvalidateRect(hWnd, 0, TRUE);
		break;
	case WM_DESTROY:
		DeleteObject(hBitmap);
		DeleteObject(hFont);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	const WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	const HWND hWnd = CreateWindow(
		szClassName,
		TEXT("Window"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
