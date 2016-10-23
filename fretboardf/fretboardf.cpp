// fretboardf.cpp : Defines the entry point for the application.

// http://www.liutaiomottola.com/formulae/fret.htm

#include "stdafx.h"
#include "fretboardf.h"

using namespace std;

#define MAX_LOADSTRING 100

// global variables:
HINSTANCE hInst;						// current instance
HBITMAP hBitmap = NULL;					// BG bitmap
HFONT hFont = NULL;
TCHAR szTitle[MAX_LOADSTRING];			// title bar text
TCHAR szWindowClass[MAX_LOADSTRING];	// main window class name
int ScaleLength = 825;
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
bool pattern[TOTALSTRINGS][TOTALFRETS] = {};
string lastnote = "";
int currentstring = 0;
int currentfret = 0;

void InitPattern()
{
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
		//	pattern[i][j] = 1; // for debug
			cout << (pattern[i][j] ? "+" : "-");
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

	hFont = CreateFont(
		14, 6,							// height, width
		0, 0, 600,						// escapement, orientation, weight
		FALSE, FALSE, FALSE,			// italic, underline, strikeout
		ANSI_CHARSET, OUT_DEVICE_PRECIS,// charset, precision
		CLIP_MASK, CLEARTYPE_NATURAL_QUALITY,		// clipping, quality
		DEFAULT_PITCH, "Arial");		// pitch, name

	return RegisterClassEx(&wcex);
}

// save the instance handle in a global variable, create and display the main program window
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;	
	hInst = hInstance; // store instance handle in our global variable	
	hWnd = CreateWindow(
		szWindowClass,			// lpClassName
		szTitle,				// lpWindowName
		WS_OVERLAPPEDWINDOW,	// dwStyle
		400,					// x
		400,					// y
		923,					// nWidth
		149,					// nHeight
		NULL,					// hWndParent
		NULL,					// hMenu
		hInstance,				// hInstance
		NULL					// lpParam
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
	RECT cr;

	switch (message)
	{
	case WM_CREATE:
		GetClientRect(hWnd, &cr);
    	hBitmap = (HBITMAP)LoadImage(hInst, "fretboardbg.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
		InitPattern();
		break;

	case WM_PAINT:
	{
    	PAINTSTRUCT 	ps;
    	BITMAP 			bitmap;
		HDC 			hdc;
    	HDC 			hdcMem;
        HGDIOBJ 		oldBitmap;

		GetClientRect(hWnd, &cr);
    	hdc = BeginPaint(hWnd, &ps);
    	hdcMem = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hBitmap);
        GetObject(hBitmap, sizeof(bitmap), &bitmap);
		BGwidth = bitmap.bmWidth;
		BGheight = bitmap.bmHeight;
		
		SetStretchBltMode(hdc, HALFTONE);
        StretchBlt(hdc, cr.top, cr.left, cr.right, cr.bottom, hdcMem, 0, 0, BGwidth, BGheight, SRCCOPY);		
		SelectObject(hdc, hFont);
		SetBkColor  (hdc, 0x000000ff);
		SetTextColor(hdc, 0x00ffffff);
		ScaleLength = cr.right * 1.305f;

		for (int str=0; str<TOTALSTRINGS; str++)
		{
			for (int fret=0; fret<TOTALFRETS; fret++)
			{
				if (pattern[str][fret])
				{
					string root = tuning[str];
					int offset;
					for (offset=0; offset<TOTALNOTES; offset++)
					{
						if (notes[offset] == root)
							break;
					}
					string note = notes[(offset + fret) % TOTALNOTES];
					int cur = ScaleLength - ((ScaleLength) / pow((float)2, (float)fret / TOTALNOTES));
					int prev = fret == 0 ? 0 : ScaleLength - ((ScaleLength) / pow((float)2, (float)(fret - 1) / TOTALNOTES));
					int x = float(prev + (cur - prev) / 2);
					float strheight = (float)cr.bottom / TOTALSTRINGS;
					int y = strheight * float(TOTALSTRINGS - 1 - str) + (float)strheight / 2 - 7;
					if (note.length() == 1)
						x += 3;
					if (fret == 0)
						x = 0;
					else
						x += 2;
					MoveToEx(hdc, 0, 0, NULL);
					TextOut(hdc, x, y, note.c_str(), note.length());
				}
			}
		}
        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);

    	EndPaint(hWnd, &ps);
    	break;
	}

	case WM_MOUSEMOVE:
	{
		char header[100];
		POINT Mouse;
		POINTSTOPOINT(Mouse, MAKEPOINTS(lParam));
		Mouse.x -= 11;
		GetClientRect(hWnd, &cr);
		currentstring = Mouse.y / (cr.bottom / 6);
		currentfret   = (TOTALNOTES * log((float)ScaleLength / (ScaleLength - Mouse.x))) / (log(2.0f)) + 1;
		if (currentfret > TOTALFRETS - 1)
			currentfret = TOTALFRETS - 1;
		if (currentstring > TOTALSTRINGS - 1)
			currentstring = TOTALSTRINGS - 1;
		string root = tuning[currentstring];
		int offset;
		for (offset=0; offset<TOTALNOTES; offset++)
		{
			if (notes[offset] == root)
				break;
		}
		string note = notes[(offset + currentfret) % TOTALNOTES];
		sprintf(header, "%s    %d : %d", note.c_str(), currentstring + 1, currentfret);
		if (note != lastnote)
			SetWindowText(hWnd, header);
		break;
	}

	case WM_LBUTTONDOWN:
		GetClientRect(hWnd, &cr);
		pattern[TOTALSTRINGS - currentstring - 1][currentfret] ^= 1;
		DebugPrintPattern();
		InvalidateRect(hWnd, &cr, 0);

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_FILE_LOAD:

		case ID_FILE_SAVEAS:
		{
			char filename[MAX_PATH];
			OPENFILENAME ofn;
			ZeroMemory(&filename, sizeof(filename));
			ZeroMemory(&ofn,      sizeof(ofn));
			ofn.lStructSize  = sizeof(ofn);
			ofn.hwndOwner    = NULL;
			ofn.lpstrFilter  = "Text Files\0*.txt\0Any File\0*.*\0";
			ofn.lpstrFile    = filename;
			ofn.nMaxFile     = MAX_PATH;
			ofn.lpstrTitle   = "Select a File";
			ofn.Flags        = OFN_FILEMUSTEXIST;
			ofn.lpstrDefExt  = (LPCSTR)"txt";
  
			GetModuleFileName(NULL, filename, MAX_PATH);
			PathRemoveFileSpec(filename);
			strcat(filename, "\\pattern");
			if (GetSaveFileName(&ofn))
			{
				std::ofstream out(filename);
				for (int i=TOTALSTRINGS-1; i>=0; i--)
				{
					out << tuning[i] << " ";
					for (int j=0; j<TOTALFRETS; j++)
					{
						out << (pattern[i][j] ? "+" : "-");
					}
					out << "\n";
				}
				out.close();
			}

			break;
		}
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
		DeleteObject(hFont);
		hFont = NULL;
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
