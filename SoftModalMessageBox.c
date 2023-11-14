/*
Copyright (c) 2018-2023, Yubsoft. All rights reserved.
support@yubsoft.com

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <assert.h>
#include "SoftModalMessageBox.h"

static int LoadStringEx(HMODULE hModule, UINT wID, PWSTR pBuffer, int cchBufferMax, WORD wLangId)
{
    HRSRC hRsrc;
    int cch = 0;
    UINT GroupId = (wID >> 4) + 1;

    if (pBuffer == NULL) {
        assert(0);
        return 0;
    }

    hRsrc = FindResourceEx(hModule, RT_STRING, (PWSTR)GroupId, wLangId);
    if (hRsrc) {
        HGLOBAL hStringSeg = LoadResource(hModule, hRsrc);
        PWSTR psz = (PWSTR)LockResource(hStringSeg);
        if (psz) {
            wID &= 0x0F;
            while (TRUE) {
                cch = *psz++;
                if (wID-- == 0) {
                    break;
                }
                psz += cch;
            }

            if (cchBufferMax == 0) {
                *(PWSTR*)pBuffer = psz;
            } else {
                cchBufferMax--;
                if (cch > cchBufferMax) {
                    cch = cchBufferMax;
                }

                RtlCopyMemory(pBuffer, psz, cch * sizeof(WCHAR));
            }
            UnlockResource(hStringSeg);
        }
    }

    if (cchBufferMax != 0) {
        pBuffer[cch] = 0;
    }
    return cch;
}

#define MAX_MSGBUTTONS  11
#define MAX_MSGTEXT     32
typedef int (WINAPI* PROC_SOFTMODALMESSAGEBOX)(PMSGBOXDATA lpmb);
typedef int (WINAPI* PROC_LOADSTRINGBASEEXW)(HINSTANCE hInstance, UINT uID, PWSTR lpBuffer, int nBufferMax, int LangId);
int SoftModalMessageBoxExW(HWND hWnd, PCWSTR Text, PCWSTR Caption, UINT Type, MSGBOXBUTTONW* pButtons, UINT cButtons, UINT dwTimeout, HINSTANCE hInstance, PCWSTR pszIcon, LANGID wLangId)
{
    HMODULE hUser32 = LoadLibrary(L"user32");
    PROC_SOFTMODALMESSAGEBOX pSoftModalMessageBox = (PROC_SOFTMODALMESSAGEBOX)GetProcAddress(hUser32, "SoftModalMessageBox");
    PROC_LOADSTRINGBASEEXW LoadStringBaseExW = (PROC_LOADSTRINGBASEEXW)GetProcAddress(LoadLibrary(L"kernel32"), "LoadStringBaseExW");
    MSGBOXDATA mbd = {0};
    MSGBOXDATA* pmbd = &mbd;
    int ButtonIds[MAX_MSGBUTTONS] = {0};
    WCHAR* ButtonTexts[MAX_MSGBUTTONS] = {0};
    WCHAR TextBuffer[MAX_MSGBUTTONS][MAX_MSGTEXT] = {0};
    BOOL fCancel = FALSE;
    UINT i;
    MSGBOXBUTTONW MsgBoxButtonNull = {IDOK};

    if (pSoftModalMessageBox == 0) {
        assert(0);
        return 0;
    }

    if (pButtons == NULL) {
        pButtons = &MsgBoxButtonNull;
        cButtons = 1;
    }

    if (cButtons == 0) {
        cButtons = 1;
    }

    // Max support 11 buttons
    if (cButtons > MAX_MSGBUTTONS) {
        cButtons = MAX_MSGBUTTONS;
    }

    for (i = 0; i < cButtons; i++) {
        ButtonIds[i] = pButtons[i].ButtonID;
        ButtonTexts[i] = (WCHAR*)pButtons[i].ButtonText;
        if (ButtonIds[i] == IDCANCEL) {
            fCancel = TRUE;
        }

        // If user doesn't specify button text, try to load one from user32.dll resource
        if (ButtonTexts[i] == 0) {
#if 0
            int n = LoadStringEx(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT, wLangId);
            if (n == 0) {
                n = LoadStringEx(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT, 0);
            }
#else
            // Also can use the LoadStringBaseExW (available in Windows Vista or later) to load
            int n = (LoadStringBaseExW != NULL) ? LoadStringBaseExW(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT, wLangId) :
                    LoadStringEx(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT, wLangId);
            if (n == 0) {
                n = LoadString(hUser32, ButtonIds[i] + 800 - 1, TextBuffer[i], MAX_MSGTEXT);
            }
#endif
            ButtonTexts[i] = TextBuffer[i];
        }
    }

    mbd.cbSize = sizeof(MSGBOXPARAMSW);
    mbd.hwndOwner = hWnd;
    mbd.hInstance = NULL;
    mbd.lpszText = Text;
    mbd.lpszCaption = Caption;
    mbd.dwStyle = Type;
    if (pszIcon != NULL) {
        mbd.lpszIcon = pszIcon;
        mbd.dwStyle &= (~MB_ICONMASK);
        mbd.dwStyle |= MB_USERICON;
    }

    //++ Workaround begin: Windows XP doesn't has dwPadding
    if (LoadStringBaseExW == NULL) {
        pmbd = (MSGBOXDATA*)((UCHAR*)&mbd - sizeof(DWORD));
    }
    //-- Workaround end

    pmbd->wLanguageId = wLangId;
    pmbd->dwTimeout = (dwTimeout == 0) ? INFINITE : dwTimeout;
    pmbd->pidButton = ButtonIds;
    pmbd->ppszButtonText = ButtonTexts;
    pmbd->cButtons = cButtons;
    pmbd->DefButton = (mbd.dwStyle & MB_DEFMASK) >> 8;
    if (cButtons == 1 && pButtons[0].ButtonID == IDOK) {
        pmbd->CancelId = IDOK;
    } else if (fCancel) {
        pmbd->CancelId = IDCANCEL;
        mbd.dwStyle |= MB_OKCANCEL;  // If MB_OK SoftModalMessageBox will return 1 always
    } else {
        mbd.dwStyle |= MB_OKCANCEL;  // If MB_OK SoftModalMessageBox will return 1 always
    }

    return pSoftModalMessageBox(&mbd);
}

int SoftModalMessageBoxExA(HWND hWnd, PCSTR Text, PCSTR Caption, UINT Type, MSGBOXBUTTONA* pButtons, UINT cButtons, UINT dwTimeout, HINSTANCE hInstance, PCSTR pszIcon, LANGID wLangId)
{
    int ret;
    WCHAR* wText;
    WCHAR* wCaption;
    WCHAR TextBuffer[MAX_MSGBUTTONS][MAX_MSGTEXT] = {0};
    MSGBOXBUTTONW wpButtons[MAX_MSGBUTTONS] = {0};
    UINT i;

    if (Text != NULL) {
        size_t TextLen = strlen(Text);
        wText = LocalAlloc(LPTR, (TextLen + 1) * sizeof(WCHAR));
        MultiByteToWideChar(CP_ACP, 0, Text, -1, wText, (int)TextLen);
    } else {
        wText = L"";
    }

    if (Caption != NULL) {
        size_t CaptionLen = strlen(Caption);
        wCaption = LocalAlloc(LPTR, (CaptionLen + 1) * sizeof(WCHAR));
        MultiByteToWideChar(CP_ACP, 0, Caption, -1, wCaption, (int)CaptionLen);
    } else {
        wCaption = L"";
    }

    for (i = 0; i < min(cButtons, MAX_MSGBUTTONS); i++) {
        wpButtons[i].ButtonID = pButtons[i].ButtonID;
        if (pButtons[i].ButtonText != NULL) {
            wpButtons[i].ButtonText = TextBuffer[i];
            MultiByteToWideChar(CP_ACP, 0, pButtons[i].ButtonText, -1,  TextBuffer[i], MAX_MSGTEXT - 1);
        }
    }
    ret = SoftModalMessageBoxExW(hWnd, wText, wCaption, Type, wpButtons, cButtons, dwTimeout, hInstance, (PCWSTR)pszIcon, wLangId);

    if (Text != NULL) {
        LocalFree(wText);
    }

    if (Caption != NULL) {
        LocalFree(wCaption);
    }
    return ret;
}

int SoftModalMessageBoxW(HWND hWnd, PCWSTR Text, PCWSTR Caption, UINT Type, MSGBOXBUTTONW* pButtons, UINT cButtons)
{
    return SoftModalMessageBoxExW(hWnd, Text, Caption, Type, pButtons, cButtons, 0, NULL, NULL, 0);
}

int SoftModalMessageBoxA(HWND hWnd, PCSTR Text, PCSTR Caption, UINT Type, MSGBOXBUTTONA* pButtons, UINT cButtons)
{
    return SoftModalMessageBoxExA(hWnd, Text, Caption, Type, pButtons, cButtons, 0, NULL, NULL, 0);
}

#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "comctl32.lib")
int wmain(int argc, wchar_t* argv[])
{
    InitCommonControls();

    {
        MSGBOXBUTTONW Buttons[] = {{IDOK}, {IDCANCEL, L"Annuler"}, {IDABORT, L"中止"}, {IDRETRY, L"重試"}, {IDIGNORE, L"Ignorieren"}, {IDYES, L"Sì"}, {IDNO, L"Não"}, {IDCLOSE, L"Cerrar"}, {IDHELP, L"Справка"}, {IDTRYAGAIN, L"Prøv igjen"}, {IDCONTINUE, L"Voortzetten"}};
        int ret = SoftModalMessageBoxExW(
            NULL, L"SoftModalMessageBox with 11 buttons", L"SoftModalMessageBox - yubsoft.com",
            MB_ICONINFORMATION | MB_DEFBUTTON6,
            Buttons, ARRAYSIZE(Buttons),
            0, NULL, IDI_SHIELD,
            MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT));
        printf("SoftModalMessageBox return %d\n", ret);
    }

    {
        MSGBOXBUTTONW Buttons[] = {{IDOK}, {IDYES}};
        int ret = SoftModalMessageBoxW(NULL, L"SoftModalMessageBox with Close button is disabled", L"SoftModalMessageBox - yubsoft.com", MB_ICONINFORMATION, Buttons, ARRAYSIZE(Buttons));
        printf("SoftModalMessageBox return %d\n", ret);
    }

    {
        MSGBOXBUTTONA Buttons[] = {{IDOK}, {IDCANCEL}};
        int ret = SoftModalMessageBoxExA(NULL, 
            "SoftModalMessageBox - default button text language", "SoftModalMessageBox - yubsoft.com",
            MB_ICONINFORMATION,
            Buttons, ARRAYSIZE(Buttons),
            0, NULL, NULL,
            MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
        printf("SoftModalMessageBox return %d\n", ret);
    }

    return 0;
}
