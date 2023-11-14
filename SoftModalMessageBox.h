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

#pragma once
#ifdef __cplusplus
extern "C" {
#endif

/*
 * The C/C++ headers in the Windows SDK assume the platform's default alignment is used.
 * Don't change the setting from the default when you include the Windows SDK headers,
 * either by using /Zp on the command line or by using #pragma pack. Otherwise,
 * your application may cause memory corruption at runtime.
 */

typedef struct _MSGBOXDATA {
    MSGBOXPARAMSW;
    HWND    pwndOwner;          // Internal use only
    DWORD   dwPadding;          // Note: Windows XP has no this field
    WORD    wLanguageId;
    INT*    pidButton;          // Array of button IDs
    LPWSTR* ppszButtonText;     // Array of button text strings
    DWORD   cButtons;           // Number of buttons
    UINT    DefButton;          // Default button ID
    UINT    CancelId;           // Button ID corresponding to Cancel action
    DWORD   dwTimeout;          // Message box timeout
    HWND*   phwndList;          // Internal use only
    DWORD   dwReserved[20];     // Reserved for future use
} MSGBOXDATA, *PMSGBOXDATA;

typedef struct {
    int     ButtonID;           // Button Control ID, range: IDOK to IDCONTINUE, aka 1 to 11
    PCWSTR  ButtonText;         // Button Control Text, if NULL, will load from user32.dll with wLangId parameter in SoftModalMessageBox
} MSGBOXBUTTONW;

typedef struct {
    int     ButtonID;           // Button Control ID, range: IDOK to IDCONTINUE, aka 1 to 11
    PCSTR   ButtonText;         // Button Control Text, if NULL, will load from user32.dll with wLangId parameter in SoftModalMessageBox
} MSGBOXBUTTONA;

typedef struct {
    HINSTANCE hInstance;        // Handle to the module that contains the icon resource identified by the lpszIcon member, and the string resource identified by the lpszText or lpszCaption member.
    LPCTSTR   lpszIcon;         // Identifies an icon resource. This parameter can be either a null-terminated string or an integer resource identifier passed to the MAKEINTRESOURCE macro.
    DWORD     dwTimeout;        // Time-out value to close the msgdlg automatically, in milliseconds. 0 means INFINITE
    DWORD     dwLanguageId;     // Specifies the language in which to display the text contained in the predefined push buttons.
} PARAM_MSGBOX;

#define MB_DEFBUTTON5               0x00000400L
#define MB_DEFBUTTON6               0x00000500L
#define MB_DEFBUTTON7               0x00000600L
#define MB_DEFBUTTON8               0x00000700L
#define MB_DEFBUTTON9               0x00000800L
#define MB_DEFBUTTON10              0x00000900L
#define MB_DEFBUTTON11              0x00000A00L

// Parameters:
// hWnd      - Handle to the owner window of the message box to be created. If this parameter is NULL, the message box has no owner window.
// Text      - Pointer to a null-terminated string that contains the message to be displayed.
// Caption   - Pointer to a null-terminated string that contains the dialog box title. If this parameter is NULL, the default title Error is used.
// Type      - Specifies the contents and behavior of the dialog box.
// cButtons  - Specifies the count of pButtons
// dwTimeout - Specifies the time-out value to close the msgdlg automatically, in milliseconds. 0 means INFINITE
// hInstance - Handle to the module that contains the icon resource identified by the lpszIcon member, and the string resource identified by the lpszText or lpszCaption member.
// lpszIcon  - Identifies an icon resource. This parameter can be either a null-terminated string or an integer resource identifier passed to the MAKEINTRESOURCE macro.
// wLangId   - Specifies the language of default button text if not specify one in MSGBUTTON->ButtonText
int SoftModalMessageBoxExW(
    HWND hWnd,
    PCWSTR Text,
    PCWSTR Caption,
    UINT Type,
    MSGBOXBUTTONW* pButtons,
    UINT cButtons,
    UINT dwTimeout,
    HINSTANCE hInstance,
    PCWSTR pszIcon,
    LANGID wLangId);

int SoftModalMessageBoxW(
    HWND hWnd,
    PCWSTR Text,
    PCWSTR Caption,
    UINT Type,
    MSGBOXBUTTONW* pButtons,
    UINT cButtons);

int SoftModalMessageBoxExA(
    HWND hWnd,
    PCSTR Text,
    PCSTR Caption,
    UINT Type,
    MSGBOXBUTTONA* pButtons,
    UINT cButtons,
    UINT dwTimeout,
    HINSTANCE hInstance,
    PCSTR pszIcon,
    LANGID wLangId);

int SoftModalMessageBoxA(
    HWND hWnd,
    PCSTR Text,
    PCSTR Caption,
    UINT Type,
    MSGBOXBUTTONA* pButtons,
    UINT cButtons);

#ifdef UNICODE
#define SoftModalMessageBoxEx SoftModalMessageBoxExW
#define SoftModalMessageBox SoftModalMessageBoxW
#else
#define SoftModalMessageBoxEx SoftModalMessageBoxExA
#define SoftModalMessageBox SoftModalMessageBoxA
#endif // !UNICODE

#ifdef __cplusplus
}
#endif