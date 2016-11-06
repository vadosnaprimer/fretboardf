// fretboardf.cpp : Defines the entry point for the application.

// http://www.liutaiomottola.com/formulae/fret.htm

#include "stdafx.h"
#include "resource.h"

using namespace std;

#define MAX_LOADSTRING 100
#define TOTALSTRINGS   6
#define TOTALFRETS     25
#define TOTALNOTES     12
#define POSTOFRET(x)   static_cast<int>((TOTALNOTES * log((float)ScaleLength / (ScaleLength - x))) / (log(2.0f)))
#define FRETTOPOS(x)   static_cast<int>(ScaleLength - ((ScaleLength) / pow((float)2, (float)(x) / TOTALNOTES)))

HINSTANCE hInst;
HBITMAP hBitmap = NULL;
HFONT hFont = NULL;
HWND SpinControlHWNDs[TOTALSTRINGS];
HWND EditControlHWNDs[TOTALSTRINGS];

int WindowX = 0;
int WindowY = 0;
int WindowW = 0;
int WindowH = 0;
int ScaleLength = 0;
int BGwidth = 0;
int BGheight = 0;
int CurrentString = 0;
int CurrentFret = 0;
int ClientOffset = 46;
bool pattern[TOTALSTRINGS][TOTALFRETS] = {};
string tuning[TOTALSTRINGS] = { "E", "A", "D", "G", "B", "E" };
const string defaulttuning[TOTALSTRINGS] = { "E", "A", "D", "G", "B", "E" };
const string notes[TOTALNOTES] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
char configfile[1024];
char configbuf[1024];
string lastnote = "";

enum OFNREASON {
	PATTERNLOAD = 0,
	PATTERNSAVE,
	TUNINGLOAD,
	TUNINGSAVE
};

void DebugPrintPattern()
{
#ifdef _DEBUG
	system("cls");
	freopen("conout$", "w", stdout);
	cout.fill(' ');
	for (int i=TOTALSTRINGS-1; i>=0; i--)
	{
		cout << setw(3) << left << tuning[i];
		for (int j=0; j<TOTALFRETS; j++)
		{
			cout << (pattern[i][j] ? "+" : "-");
		}
		cout << "\n";
	}
#endif
}

void UpdateWindow(HWND hWnd, RECT *cr)
{
	DebugPrintPattern();
	cr->left -= 8;
	GetClientRect(hWnd, cr);
	InvalidateRect(hWnd, cr, 0);
}

void UpdateControlPositions(HWND hWnd)
{
	RECT cr, sr, er;
	int y;
	float strheight;
	GetClientRect(hWnd, &cr);
	for (int i = 0; i < TOTALSTRINGS; i++)
	{
		GetWindowRect(SpinControlHWNDs[i], &sr);
		GetWindowRect(EditControlHWNDs[i], &er);
		POINT sp = { sr.left, sr.top };
		POINT ep = { er.left, er.top };
		ScreenToClient(hWnd, &sp);
		ScreenToClient(hWnd, &ep);
		strheight = (float)cr.bottom / TOTALSTRINGS;
		y = static_cast<int>(strheight * float(TOTALSTRINGS - 1 - i) + (float)strheight / 2 - (sr.bottom - sr.top) / 2);
		SetWindowPos(SpinControlHWNDs[i], 0, sp.x, y + 1, sr.right - sr.left, sr.bottom - sr.top, 0);
		SetWindowPos(EditControlHWNDs[i], 0, ep.x, y    , er.right - er.left, er.bottom - er.top, 0);
	}
}

void InitPattern()
{
	for (int i=TOTALSTRINGS-1; i>=0; i--)
	{
		for (int j=0; j<TOTALFRETS; j++)
		{
			pattern[i][j] = false;
		}
	}
	DebugPrintPattern();
}

void PrepareFile(char *filename, OPENFILENAME *ofn, int size, int reason)
{
	ZeroMemory(filename, MAX_PATH);
	ZeroMemory(ofn, size);
	GetModuleFileName(NULL, filename, MAX_PATH);
	PathRemoveFileSpec(filename);

	switch (reason)
	{
	case PATTERNLOAD:
		strcat(filename, "\\pattern");
		ofn->lpstrTitle = "Load Pattern From...";
		break;
	case PATTERNSAVE:
		strcat(filename, "\\pattern");
		ofn->lpstrTitle = "Save Pattern As...";
		break;
	case TUNINGLOAD:
		strcat(filename, "\\tuning");
		ofn->lpstrTitle = "Load Tuning From...";
		break;
	case TUNINGSAVE:
		strcat(filename, "\\tuning");
		ofn->lpstrTitle = "Save Tuning As...";
		break;
	}

	ofn->lStructSize     = size;
	ofn->hwndOwner       = NULL;
	ofn->lpstrInitialDir = filename;
	ofn->lpstrFile       = filename;
	ofn->lpstrFilter     = "Text Files\0*.txt\0Any File\0*.*\0";
	ofn->nMaxFile        = MAX_PATH;
	ofn->Flags           = OFN_FILEMUSTEXIST;
	ofn->lpstrDefExt     = (LPCSTR)"txt";
}

void Exit()
{
	sprintf(configbuf, "%d", WindowX);
	WritePrivateProfileString("Position", "x", configbuf, configfile);
	sprintf(configbuf, "%d", WindowY);
	WritePrivateProfileString("Position", "y", configbuf, configfile);
	sprintf(configbuf, "%d", WindowW);
	WritePrivateProfileString("Position", "w", configbuf, configfile);
	sprintf(configbuf, "%d", WindowH);
	WritePrivateProfileString("Position", "h", configbuf, configfile);

	DeleteObject(hBitmap);
	DeleteObject(hFont);
	hFont = NULL;
	PostQuitMessage(0);
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hWndEdit = GetDlgItem(hDlg, IDC_EDIT1);
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowText(hWndEdit, _T("https://github.com/vadosnaprimer/fretboardf"));
		return 1;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return 1;
		}
		break;
	}
	return 0;
}

BOOL CALLBACK DialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT cr, r;

	switch (message)
	{
	case WM_INITDIALOG:
		GetModuleFileName(NULL, configfile, MAX_PATH);
		PathRemoveFileSpec(configfile);
		strcat(configfile, "\\config.txt");
#ifdef _DEBUG
		AllocConsole();
#endif
		for (int i = 0; i < TOTALSTRINGS; i++)
		{
			SpinControlHWNDs[i] = GetDlgItem(hWnd, IDC_SPIN1 + i);
			EditControlHWNDs[i] = GetDlgItem(hWnd, IDC_EDIT1 + i);
			SendDlgItemMessage(hWnd, IDC_EDIT1 + i, EM_LIMITTEXT, 2, 0);
			SetDlgItemText    (hWnd, IDC_EDIT1 + i, tuning[i].c_str());
		}
    	hBitmap = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(IDB_BITMAP1), IMAGE_BITMAP, 0, 0, LR_COPYFROMRESOURCE);
		InitPattern();

		WindowX = GetPrivateProfileInt("Position", "x", 200, configfile);
		WindowY = GetPrivateProfileInt("Position", "y", 200, configfile);
		WindowW = GetPrivateProfileInt("Position", "w", 960, configfile);
		WindowH = GetPrivateProfileInt("Position", "h", 174, configfile);
		MoveWindow(hWnd, WindowX, WindowY, WindowW, WindowH, 0);

		UpdateControlPositions(hWnd);
		SetFocus(SpinControlHWNDs[0]); // hack, to prevent autoselectiin of edit control text
		return 0; // same reason

	case WM_NOTIFY:
	{
		LPNMUPDOWN lpnmud = (LPNMUPDOWN)lParam;
		if (lpnmud->hdr.code == UDN_DELTAPOS)
		{
			int offset;
			int str = lpnmud->hdr.idFrom - IDC_SPIN1;
			for (offset = 0; offset < TOTALNOTES; offset++)
			{
				if (notes[offset] == tuning[str])
					break;
			}
			if (offset == 0)
				offset = TOTALNOTES;
			if (lpnmud->iDelta > 0)
				tuning[str] = notes[(offset + 1) % TOTALNOTES];
			else
				tuning[str] = notes[offset - 1];
			SetDlgItemText(hWnd, IDC_EDIT1 + str, tuning[str].c_str());
			UpdateWindow(hWnd, &cr);
		}
    	return 1;
	}
	
	case WM_PAINT:
	{
    	PAINTSTRUCT 	ps;
    	BITMAP 			bitmap;
		HDC 			hdc;
    	HDC 			hdcMem;
        HGDIOBJ 		oldBitmap;

        GetObject(hBitmap, sizeof(bitmap), &bitmap);
		GetClientRect(hWnd, &cr);
    	hdc       = BeginPaint(hWnd, &ps);
    	hdcMem    = CreateCompatibleDC(hdc);
        oldBitmap = SelectObject(hdcMem, hBitmap);
		BGwidth   = bitmap.bmWidth;
		BGheight  = bitmap.bmHeight;
		cr.left  += ClientOffset;
		cr.right -= ClientOffset;
		
		SetStretchBltMode(hdc, HALFTONE);
        StretchBlt(hdc, cr.left, cr.top, cr.right, cr.bottom, hdcMem, 0, 5, BGwidth, BGheight-10, SRCCOPY);		
		SelectObject(hdc, hFont);
		SetBkColor(hdc, 0x000000ff);
		SetTextColor(hdc, 0x00ffffff);
		ScaleLength = static_cast<int>((cr.right - 12) * 1.32f);

		for (int str = 0; str < TOTALSTRINGS; str++)
		{
			for (int fret = 0; fret < TOTALFRETS; fret++)
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
					int cur = FRETTOPOS(fret);
					int prev = fret == 0 ? 0 : FRETTOPOS(fret - 1);
					int x = static_cast<int>(float(prev + (cur - prev) / 2) + ClientOffset);
					float strheight = (float)cr.bottom / TOTALSTRINGS;
					int y = static_cast<int>(strheight * float(TOTALSTRINGS - 1 - str) + (float)strheight / 2 - 7);
					if (note.length() == 1)
						x += 3;
					if (fret == 0)
						x = ClientOffset + 1;
					else
						x += 4;
					MoveToEx(hdc, 0, 0, NULL);
					TextOut(hdc, x, y, note.c_str(), note.length());
				}
			}
		}
        SelectObject(hdcMem, oldBitmap);
        DeleteDC(hdcMem);
    	EndPaint(hWnd, &ps);
    	return 1;
	}

	case WM_MOUSEMOVE:
	{
		char header[100];
		POINT Mouse;
		POINTSTOPOINT(Mouse, MAKEPOINTS(lParam));
		Mouse.x -= 58;
		GetClientRect(hWnd, &cr);
		CurrentString = Mouse.y / (cr.bottom / 6);
		CurrentFret   = POSTOFRET(Mouse.x) + 1;
		if (CurrentFret > TOTALFRETS - 1)
			CurrentFret = TOTALFRETS - 1;
		if (CurrentString > TOTALSTRINGS - 1)
			CurrentString = TOTALSTRINGS - 1;
		string root = tuning[TOTALSTRINGS - CurrentString - 1];
		int offset;
		for (offset = 0; offset < TOTALNOTES; offset++)
		{
			if (notes[offset] == root)
				break;
		}
		string note = notes[(offset + CurrentFret) % TOTALNOTES];
		if (note.length() == 1)
			note += "  ";
		sprintf(header, "FretboardF :: %s : %d : %d", note.c_str(), CurrentString + 1, CurrentFret);
		if (note != lastnote)
			SetWindowText(hWnd, header);
		return 1;
	}

	case WM_LBUTTONDOWN:
		pattern[TOTALSTRINGS - CurrentString - 1][CurrentFret] ^= 1;
		UpdateWindow(hWnd, &cr);
		return 1;

	case WM_SIZE:
		if (!IsIconic(hWnd))
		{
			GetWindowRect(hWnd, &r);
			WindowW = r.right - r.left;
			WindowH = r.bottom - r.top;
		}
		UpdateControlPositions(hWnd);
		UpdateWindow(hWnd, &cr);
		return 1;

	case WM_MOVE:
		GetWindowRect(hWnd, &r);
		WindowX = r.left;
		WindowY = r.top;
		return 1;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_NEWPATTERN:
		{
			InitPattern();
			UpdateWindow(hWnd, &cr);
			return 1;
		}

		case ID_FILE_LOADPATTERN:
		{
			char filename[MAX_PATH];
			OPENFILENAME ofn;
			PrepareFile(filename, &ofn, sizeof(ofn), PATTERNLOAD);

			if (GetOpenFileName(&ofn))
			{
				ifstream in(filename);
				string line;
				for (int i = TOTALSTRINGS - 1; i >= 0; i--)
				{
					in >> line;
					for (int j = 0; j < TOTALFRETS; j++)
					{
						pattern[i][j] = (line[j] == '+');
					}
					in.ignore();
				}
				in.close();
			}
			UpdateWindow(hWnd, &cr);
			return 1;
		}

		case ID_FILE_SAVEPATTERNAS:
		{
			char filename[MAX_PATH];
			OPENFILENAME ofn;
			PrepareFile(filename, &ofn, sizeof(ofn), PATTERNSAVE);

			if (GetSaveFileName(&ofn))
			{
				ofstream out(filename);
				for (int i = TOTALSTRINGS - 1; i >= 0; i--)
				{
					for (int j = 0; j < TOTALFRETS; j++)
					{
						out << (pattern[i][j] ? "+" : "-");
					}
					out << "\n";
				}
				out.close();
			}
			return 1;
		}

		case ID_FILE_LOADDEFAULTTUNING:
			for (int i = 0; i < TOTALSTRINGS; i++)
			{
				tuning[i] = defaulttuning[i];
				SetDlgItemText(hWnd, IDC_EDIT1 + i, tuning[i].c_str());
			}
			UpdateWindow(hWnd, &cr);
			return 1;

		case ID_FILE_LOADTUNING:
		{
			char filename[MAX_PATH];
			OPENFILENAME ofn;
			PrepareFile(filename, &ofn, sizeof(ofn), TUNINGLOAD);
			
			if (GetOpenFileName(&ofn))
			{
				ifstream in(filename);
				string line;
				for (int i = TOTALSTRINGS - 1; i >= 0; i--)
				{
					in >> line;
					tuning[i] = line;
					SetDlgItemText(hWnd, IDC_EDIT1 + i, tuning[i].c_str());
				}
				in.close();
			}
			UpdateWindow(hWnd, &cr);
			return 1;
		}

		case ID_FILE_SAVETUNINGAS:
		{
			char filename[MAX_PATH];
			OPENFILENAME ofn;
			PrepareFile(filename, &ofn, sizeof(ofn), TUNINGSAVE);

			if (GetSaveFileName(&ofn))
			{
				ofstream out(filename);
				for (int i = TOTALSTRINGS - 1; i >= 0; i--)
				{
					out << tuning[i];
					out << "\n";
				}
				out.close();
			}
			return 1;
		}

		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			return 1;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			Exit();
			return 1;
		}
		return 1;

	case WM_CLOSE:
		Exit();
		return 1;
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	HWND hDialog = 0;
	MSG msg;
	int status;
	hInst = hInstance;

	hFont = CreateFont(
		14, 6,									// height, width
		0, 0, 600,								// escapement, orientation, weight
		FALSE, FALSE, FALSE,					// italic, underline, strikeout
		ANSI_CHARSET, OUT_DEVICE_PRECIS,		// charset, precision
		CLIP_MASK, CLEARTYPE_NATURAL_QUALITY,	// clipping, quality
		DEFAULT_PITCH, "Arial");				// pitch, name

	hDialog = CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN), 0, DialogProc);

	if (!hDialog)
    {
        return 1;
    }

	while ((status = GetMessage(&msg, 0, 0, 0)) != 0)
	{
		if (status == -1)
            return -1;
        if (!IsDialogMessage(hDialog, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
	}

	return (int) msg.wParam;
}
