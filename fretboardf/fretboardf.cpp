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
int BGwidth = 0;
int BGheight = 0;

// forward declarations of functions included in this code module
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

//==================== CUSTOM CODE ========================//

#define TOTALFRETS  25
#define TOTALNOTES  12
#define SCALELENGTH 819

void DebugPrintPattern(void);
void InitPattern(void);

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

string tuning[TOTALSTRINGS] = { "E", "A", "D", "G", "B", "E" };
string notes[TOTALNOTES] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
string pattern[TOTALSTRINGS][TOTALFRETS] = {};
string lastnote = "";

void InitPattern()
{
	for (int i=0; i<TOTALSTRINGS; i++)
	{
		for (int j=0; j<TOTALFRETS; j++)
		{
			pattern[i][j] = (string)"-";
		}
	}
#ifdef _DEBUG
		AllocConsole();
#endif
		DebugPrintPattern();
}

void DebugPrintPattern()
{
#ifdef _DEBUG
	freopen("CONOUT$", "w", stdout);
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
}

//================= END OF CUSTOM CODE ====================//

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
    	hBitmap = (HBITMAP)LoadImage(hInst, "freatboardbg.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		InitPattern();
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
		BGwidth = bitmap.bmWidth;
		BGheight = bitmap.bmHeight;
        BitBlt(hdc, 1, 1, BGwidth, BGheight, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

    	EndPaint(hWnd, &ps);
    	break;

	case WM_MOUSEMOVE:
	{
		POINT Mouse;
		POINTSTOPOINT(Mouse, MAKEPOINTS(lParam));
		RECT cr;
		GetClientRect(hWnd, &cr);
		Mouse.x -= 8;
		int str     = Mouse.y / (cr.bottom / 6);
		// Mouse.x  = SCALELENGTH � (SCALELENGTH / (2 ^ (fret / TOTALNOTES))) // origninal formula
		int fret    = (TOTALNOTES * log((float)SCALELENGTH / (SCALELENGTH - Mouse.x))) / (log(2.0)) + 1;
		if (fret > TOTALFRETS - 1)
			fret = TOTALFRETS - 1;
		string root = tuning[str];
		int offset;
		for (offset=0; offset<TOTALNOTES; offset++)
		{
			if (notes[offset] == root)
				break;
		}
		string note = notes[(offset + fret) % TOTALNOTES];
		char header[100];
		sprintf(header, "%3s    %d : %d", note.c_str(), str+1, fret);
		if (note != lastnote)
			SetWindowText(hWnd, header);
		break;
	}

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
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
