#include <Windows.h>
#include <commdlg.h>
#include "resource.h"

#define EDITID 1
#define UNTITLED TEXT("(UNTITLED)")

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlgProc(HWND, UINT, WPARAM, LPARAM);

//declear all function

//PADFILE.C
void PadFileInitialize(HWND hwnd);
BOOL PadFileOpenDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);
BOOL PadFileSaveDlg(HWND hwnd, PTSTR pstrFileName, PTSTR pstrTitleName);
BOOL PadFileRead(HWND hwndEdit, PTSTR pstrFileName);
BOOL PadFileWrite(HWND hwndEdit, PTSTR pstrFileName);

//PADFIND.C
HWND PadFindFindDlg(HWND hwnd);
HWND PadFindReplaceDlg(HWND hwnd);
BOOL PadFindFindText(HWND hwndEdit, int * piSearchOffset, LPFINDREPLACE pfr);
BOOL PadFindNextText(HWND hwndEdit, int * piSearchOffset);
BOOL PadFindReplaceText(HWND hwndEdit, int * piSearchOffset, LPFINDREPLACE pfr);
BOOL PadFindVaildFind();

//PADFONT.C
BOOL PadFontChooseFont(HWND hwnd);
void PadFontInitialize(HWND hwndEdit);
void PadFontSetFont(HWND hwndEdit);
void PadFontDeinitialialize();

//PADPRINT0.C----------------------------------------------------
BOOL PadPrntPrintFile(HINSTANCE hInst, HWND hwnd, HWND hwndEdit,
	PTSTR pstrTitleName);// <------INVALID

//Global Variable

static HWND hDlgModeless;
static TCHAR * szAppName = TEXT("RayPad");

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;
	HACCEL hAccel;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = RayPad;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("这个程序需要在 Windows NT 才能执行！"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,
		szAppName,
		WS_OVERLAPPEDWINDOW,
		400,
		300,
		300,
		300,
		NULL,
		NULL,
		hInstance,
		szCmdLine);

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	hAccel = LoadAccelerators(hInstance, szAppName);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (hDlgModeless == NULL || !IsDialogMessage(hDlgModeless, &msg))
		{
			if (!TranslateAccelerator(hwnd, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return msg.wParam;
}

void DoCaption(HWND hwnd, TCHAR * szTitleName)
{
	TCHAR szCaption[64 + MAX_PATH];
	wsprintf(szCaption, TEXT("%s - %s"),
		szAppName, szTitleName[0] ? szTitleName : UNTITLED);
	SetWindowText(hwnd, szCaption);
}


void OkMessage(HWND hwnd, TCHAR * szMessage, TCHAR * szTitleName)
{
	TCHAR szBuffer[64 + MAX_PATH];
	wsprintf(szBuffer, szMessage, szTitleName[0] ? szTitleName : UNTITLED);
	MessageBox(hwnd, szBuffer, szAppName, MB_OK | MB_ICONEXCLAMATION);
}

short AskAboutSave(HWND hwnd, TCHAR * szTitleName)
{
	TCHAR szBuffer[64 + MAX_PATH];
	int iReturn;
	wsprintf(szBuffer, TEXT("Save current changes in %s?"),
		szTitleName[0] ? szTitleName : UNTITLED);
	iReturn = MessageBox(hwnd, szBuffer, szAppName
		, MB_YESNOCANCEL | MB_ICONEXCLAMATION);
	if (iReturn == IDYES)
	{
		if (!SendMessage(hwnd, WM_COMMAND, ID_FILE_SAVE, 0))
		{
			iReturn = IDCANCEL;
		}
	}
	return iReturn;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bNeedSave = FALSE;
	static HINSTANCE hInst;
	static HWND hwndEdit;
	static int iOffset;
	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH];
	static UINT messageFindReplace;
	int iSelBeg, iSelEnd, iEnable;
	LPFINDREPLACE pfr;

	switch (message)
	{
	case WM_CREATE:
		hInst = ((LPCREATESTRUCT)lParam)->hInstance;

		//create edit control child window

		hwndEdit = CreateWindow(TEXT("edit"), NULL, WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL | WS_BORDER
			| ES_LEFT | ES_MULTILINE | ES_NOHIDESEL | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0,
			hwnd, (HMENU)EDITID, hInst, NULL);

		SendMessage(hwndEdit, EM_LIMITTEXT, 32000, 0);
		//Initialize common dialog box stuff
		PadFileInitialize(hwnd);
		PadFontInitialize(hwndEdit);
		// custom a new message
		messageFindReplace = RegisterWindowMessage(FINDMSGSTRING);
		DoCaption(hwnd, szTitleName);
		return 0;
	case WM_SETFOCUS:
		SetFocus(hwndEdit);
		return 0;
	case WM_SIZE:
		MoveWindow(hwndEdit, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
		return 0;
	case WM_INITMENUPOPUP:			//what about WM_INITMENU?
		switch (lParam)
		{
		case 1:	//EditMenu
			//Enable Undo if Edit control can do it
			EnableMenuItem((HMENU)wParam, ID_EDIT_UNDO,
				SendMessage(hwndEdit, EM_CANUNDO, 0, 0) ? MF_ENABLED : MF_GRAYED);
			//Enable Paste if can do it
			EnableMenuItem((HMENU)wParam, ID_EDIT_PASTE,
				IsClipboardFormatAvailable(CF_TEXT) ? MF_ENABLED : MF_GRAYED);
			//When text is selected
			SendMessage(hwndEdit, EM_GETSEL, (WPARAM)&iSelBeg, (LPARAM)&iSelEnd);
			iEnable = iSelBeg != iSelEnd ? MF_ENABLED : MF_GRAYED;

			EnableMenuItem((HMENU)wParam, ID_EDIT_UNDO, iEnable);
			EnableMenuItem((HMENU)wParam, ID_EDIT_COPY, iEnable);
			EnableMenuItem((HMENU)wParam, ID_EDIT_DELETE, iEnable);
			break;
		case 2: //Search menu
			//Enable Find,Next,and Replace if modeless DLG ae not already active
			iEnable = hDlgModeless == NULL ? MF_ENABLED : MF_GRAYED;
			EnableMenuItem((HMENU)wParam, ID_SEARCH_FIND, iEnable);
			EnableMenuItem((HMENU)wParam, ID_SEARCH_FINDNEXT, iEnable);
			EnableMenuItem((HMENU)wParam, ID_SEARCH_REPLACE, iEnable);
			break;
		}
		return 0;
	case WM_COMMAND:
		//Message from edit control

		if (lParam && LOWORD(wParam) == EDITID)
		{
			switch (HIWORD(wParam))

			{
			case EN_UPDATE://edit ichanged not display.
				//EN_CHANGED is after displaying
				bNeedSave = TRUE;
				return 0;
			case EN_ERRSPACE:
			case EN_MAXTEXT:
				MessageBox(hwnd, TEXT("Edit Control out of space."), szAppName, MB_OK | MB_ICONSTOP);
				return 0;
			}
			break;
		}

		switch (LOWORD(wParam))
		{
			//Message from File Menu:
		case ID_FILE_NEW:
			if (bNeedSave && IDCANCEL == AskAboutSave(hwnd, szTitleName))
				return 0;
			SetWindowText(hwndEdit, TEXT("\0"));
			szFileName[0] = '\0';
			szTitleName[0] = '\0';
			DoCaption(hwnd, szTitleName);
			bNeedSave = FALSE;
			return 0;
		case ID_FILE_OPEN:
			if (bNeedSave && IDCANCEL == AskAboutSave(hwnd, szTitleName))
				return 0;
			if (PadFileOpenDlg(hwnd, szFileName, szTitleName))
			{
				if (!PadFileRead(hwndEdit, szFileName))
				{
					OkMessage(hwnd, TEXT("Could not read file %s!"), szTitleName);
					szFileName[0] = '\0'; 
					szTitleName[0] = '\0';
				}
			}
			DoCaption(hwnd, szTitleName);
			bNeedSave = FALSE;
			return 0;
		case ID_FILE_SAVE:
			if (szFileName[0])
			{
				if (PadFileWrite(hwndEdit, szFileName))
				{
					bNeedSave = FALSE;
					return 1;// <----???
				}
				else
				{
					OkMessage(hwnd, TEXT("Could not read file %s!"),
						szTitleName);
					return 0;
				}
			}//fall through
		case ID_FILE_SAVEAS:
			if (PadFileSaveDlg(hwnd, szFileName, szTitleName))
			{
				DoCaption(hwnd, szFileName);
				if (PadFileWrite(hwndEdit, szFileName))
				{
					bNeedSave = FALSE;
					return 1;
				}
				else
				{
					OkMessage(hwnd, TEXT("Could not write file %s"), szTitleName);
					return 0;
				}
			}
			return 0;
		case ID_FILE_PAINT:
			if (!PadPrntPrintFile(hInst, hwnd, hwndEdit, szTitleName))
			{
				OkMessage(hwnd, TEXT("Could not print files %s"), szTitleName);
			}

			return 0;

		case ID_FILE_EXIT:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			return 0;
		case ID_EDIT_UNDO:
			SendMessage(hwndEdit, WM_UNDO, 0, 0);
			return 0;
		case ID_EDIT_CUT:
			SendMessage(hwndEdit, WM_CUT, 0, 0);
			return 0;
		case ID_EDIT_COPY:
			SendMessage(hwndEdit, WM_COPY, 0, 0);
			return 0;
		case ID_EDIT_PASTE:
			SendMessage(hwndEdit, WM_PASTE, 0, 0);
			return 0;
		case ID_EDIT_DELETE:
			SendMessage(hwndEdit, WM_CLEAR, 0, 0);
			return 0;
			//MSG from Search MENU

		case ID_SEARCH_FIND:
			SendMessage(hwndEdit, EM_GETSEL, 0, (LPARAM)&iOffset);
			hDlgModeless = PadFindFindDlg(hwnd);
			return 0;
		case ID_SEARCH_FINDNEXT:
			SendMessage(hwndEdit, EM_GETSEL, 0, (LPARAM)&iOffset);
			if (PadFindVaildFind())
				PadFindNextText(hwndEdit, &iOffset);
			else
				hDlgModeless = PadFindFindDlg(hwnd);
			return 0;
		case ID_SEARCH_REPLACE:
			SendMessage(hwndEdit, EM_GETSEL, 0, (LPARAM)&iOffset);
			hDlgModeless = PadFindReplaceDlg(hwnd);
			return 0;
		case ID_FORMAT_FONT:
			if (PadFontChooseFont(hwnd))
				PadFontSetFont(hwndEdit);
			return 0;

			//Message from HELP

		case ID_HELP_HELP:
			OkMessage(hwnd, TEXT("HELP not yet implemented!"), TEXT("\0"));
			return 0;
		case ID_HELP_ABOUTRAYPAD:
			DialogBox(hInst,AboutBox, hwnd, AboutDlgProc);
			return 0;
		}
		break;
	case WM_CLOSE:
		if (!bNeedSave || IDCANCEL != AskAboutSave(hwnd, szTitleName))
			DestroyWindow(hwnd);
		return 0;
	case WM_QUERYENDSESSION:
		if (!bNeedSave || IDCANCEL != AskAboutSave(hwnd, szTitleName))
			return 1;
		return 0;
	case WM_DESTROY:
		PadFontDeinitialialize();
		PostQuitMessage(0);
		return 0;
	default:
		//Process *Find-Replace*messages
		if (message == messageFindReplace) // Custom message
		{
			pfr = (LPFINDREPLACE)lParam;

			if (pfr->Flags & FR_DIALOGTERM)
				hDlgModeless = NULL;
			if (pfr->Flags & FR_FINDNEXT)
				if (!PadFindFindText(hwndEdit, &iOffset, pfr))
					OkMessage(hwnd, TEXT("TEXT not found!"), TEXT("\0"));
			if (pfr->Flags & FR_REPLACEALL)
				while (PadFindReplaceText(hwndEdit, &iOffset, pfr));
			return 0;
		}
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}


BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_OK:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}