// fretboardf.cpp : Defines the entry point for the application.

#include "stdafx.h"
#include "fretboardf.h"

using namespace std;

#define MAX_LOADSTRING 100

// global variables:
HINSTANCE hInst;						// current instance
HBITMAP hBitmap = NULL;					// BG bitmap
TCHAR szTitle[MAX_LOADSTRING];			// title bar text
TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name



//==================== CUSTOM VARS ========================//

#define TOTALFRETS 25

// can't start from index 1, so give the strings names
// in reverse order, since that's how notes are going
enum stringnames {
	SIX,
	FIVE,
	FOUR,
	THREE,
	TWO,
	ONE,
	TOTALSTRINGS,
};

char tuning[TOTALSTRINGS][3] = { "E", "A", "D", "G", "B", "E" };
char notes[12][3] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
char *pattern[TOTALSTRINGS][TOTALFRETS] = {};

//================= END OF CUSTOM VARS ====================//



// forward declarations of functions included in this code module
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: place code here.
	MSG msg;
	HACCEL hAccelTable;

	// initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_FRETBOARDF, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FRETBOARDF));

	// main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FRETBOARDF));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_FRETBOARDF);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

// save the instance handle in a global variable, create and display the main program window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;	
	hInst = hInstance; // store instance handle in our global variable	
	hWnd = CreateWindow(
		szWindowClass,							// lpClassName
		szTitle,								// lpWindowName
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME,	// dwStyle
		400,									// x
		400,									// y
		632,									// nWidth
		138,									// nHeight
		NULL,									// hWndParent
		NULL,									// hMenu
		hInstance,								// hInstance
		NULL									// lpParam
	);
	
	if (!hWnd)
	{
	   return FALSE;
	}
	
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	return TRUE;
}

// process messages for the main window
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_CREATE:
		for (int i=0; i<TOTALSTRINGS; i++)
		{
			for (int j=0; j<TOTALFRETS; j++)
			{
				pattern[i][j] = "-";
			}
		}

#ifdef _DEBUG
		AllocConsole();
		freopen("CONIN$", "r",stdin);
		freopen("CONOUT$", "w",stdout);
		freopen("CONOUT$", "w",stderr);

    	hBitmap = (HBITMAP)LoadImage(hInst, "freatboardbg.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		for (int i=TOTALSTRINGS-1; i>=0; i--)
		{
			cout << tuning[i] << " ";
			for (int j=0; j<TOTALFRETS; j++)
			{
				cout << pattern[i][j];
			}
			cout << "\n";
		}
		cout << "\n";
#endif
    	break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
    	PAINTSTRUCT 	ps;
    	HDC 			hdc;
    	BITMAP 			bitmap;
    	HDC 			hdcMem;
        HGDIOBJ 		oldBitmap;

    	hdc = BeginPaint(hWnd, &ps);
    	hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hBitmap);

        GetObject(hBitmap, sizeof(bitmap), &bitmap);
        BitBlt(hdc, 1, 1, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

    	EndPaint(hWnd, &ps);
    	break;

	case WM_DESTROY:
		DeleteObject(hBitmap);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hWndEdit = GetDlgItem(hDlg, IDC_EDIT1);
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(hWndEdit, _T("https://github.com/vadosnaprimer/fretboardf"));
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
