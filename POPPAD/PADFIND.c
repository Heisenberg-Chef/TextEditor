#include <Windows.h>	
#include <commdlg.h>
#include <tchar.h> // for _tcsstr(strstr for Unicode & non-Unicode)
#define MAX_STRING_LEN 256
static TCHAR szFindText[MAX_STRING_LEN];
static TCHAR szReplText[MAX_STRING_LEN];

HWND PadFindFindDlg(HWND hwnd)
{
	static FINDREPLACE fr;
	fr.lStructSize = sizeof(FINDREPLACE);
	fr.hwndOwner = hwnd;
	fr.hInstance = NULL;
	fr.Flags = FR_HIDEUPDOWN | FR_HIDEMATCHCASE | FR_HIDEWHOLEWORD;
	fr.lpstrFindWhat = szFindText;
	fr.lpstrReplaceWith = NULL;
	fr.wFindWhatLen = MAX_STRING_LEN;
	fr.wReplaceWithLen = 0;
	fr.lCustData = 0;
	fr.lpfnHook = NULL;
	fr.lpTemplateName = NULL;
	return FindText(&fr);
}

HWND PadFindReplaceDlg(HWND hwnd)
{
	static FINDREPLACE fr;
	fr.lStructSize = sizeof(FINDREPLACE);
	fr.hwndOwner = hwnd;
	fr.hInstance = NULL;
	fr.Flags = FR_HIDEUPDOWN | FR_HIDEMATCHCASE | FR_HIDEWHOLEWORD;
	fr.lpstrFindWhat = szFindText;
	fr.lpstrReplaceWith = szReplText;
	fr.wFindWhatLen = MAX_STRING_LEN;
	fr.wReplaceWithLen = MAX_STRING_LEN;
	fr.lCustData = 0;
	fr.lpfnHook = NULL;
	fr.lpTemplateName = NULL;

	return ReplaceText(&fr);
}

BOOL PadFindFindText(HWND hwndEdit, int * piSearchOffset, LPFINDREPLACE pfr)
{
	int iLength, iPos;
	PTSTR	pstrDoc, pstrPos;
	//readd in the edit document

	iLength = GetWindowTextLength(hwndEdit);

	if (NULL == (pstrDoc =(PTSTR)malloc((iLength + 1) * sizeof(TCHAR))))
		return FALSE;
	GetWindowText(hwndEdit, pstrDoc, iLength + 1);
	//Search the document for the find string

	pstrPos = _tcsstr(pstrDoc + *piSearchOffset, pfr->lpstrFindWhat);

	free(pstrDoc);
	if (pstrPos == NULL)
	{
		return FALSE;
	}
	iPos = pstrPos - pstrDoc;
	*piSearchOffset = iPos + lstrlen(pfr->lpstrFindWhat);

	//Select the found text

	SendMessage(hwndEdit, EM_SETSEL, iPos, *piSearchOffset);
	SendMessage(hwndEdit, EM_SCROLLCARET, 0, 0);
	return TRUE;
}

BOOL PadFindNextText(HWND hwndEdit, int * piSearchOffset)
{
	FINDREPLACE fr;
	fr.lpstrFindWhat = szFindText;
	return PadFindFindText(hwndEdit, piSearchOffset, &fr);
}

BOOL PadFindReplaceText(HWND hwndEdit, int * piSearchOffset,LPFINDREPLACE pfr)
{
	//Find the text
	if ((!PadFindFindText(hwndEdit, piSearchOffset, pfr)))
	{
		return FALSE;
	}
	SendMessage(hwndEdit, EM_REPLACESEL, 0, (LPARAM)pfr->lpstrReplaceWith);
	return TRUE;
}

BOOL PadFindVaildFind()
{
	return *szFindText != '\0';
}
