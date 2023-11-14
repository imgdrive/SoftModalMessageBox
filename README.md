# SoftModalMessageBox - Change text on Messagebox buttons
When developing ImgDrive, we need to change text on MessageBox buttons, but MessageBox doesn't export the related interface to the outside world. We spent some time to reverse analyze user32.dll and successfully achieved our goal.

![image](https://github.com/dvdforge/SoftModalMessageBox/assets/19568093/8eee67ba-210f-4550-8884-fcfb079ee570)

## Usage
To customize the button text, you need to call the undocumented SoftModalMessageBox interface in user32.dll.
``` c++
#include "SoftModalMessageBox.h"

int foo() {
    MSGBOXBUTTONW Buttons[] = {
        {IDOK}, 
        {IDCANCEL, L"Annuler"}, 
        {IDABORT, L"中止"}, 
        {IDRETRY, L"重試"}, 
        {IDIGNORE, L"Ignorieren"}, 
        {IDYES, L"Sì"}, 
        {IDNO, L"N?o"}, 
        {IDCLOSE, L"Cerrar"}, 
        {IDHELP, L"Справка"}, 
        {IDTRYAGAIN, L"Pr?v igjen"}, 
        {IDCONTINUE, L"Voortzetten"}};

    int ret = SoftModalMessageBoxExW(
        NULL, 
        L"SoftModalMessageBox with 11 buttons", L"SoftModalMessageBox - yubsoft.com",
        MB_ICONINFORMATION | MB_DEFBUTTON6,
        Buttons, ARRAYSIZE(Buttons),
        0, 
        NULL, IDI_SHIELD, 
        MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT));
    return ret;
}
```

## Call graph
![image](https://github.com/dvdforge/SoftModalMessageBox/assets/19568093/ebddc40a-d9db-4ae6-b989-0b8162d279cf)

There are 9 MessageBox related functions exported from Microsoft's user32.dll:

MessageBoxA
MessageBoxW
MessageBoxExA
MessageBoxExW
MessageBoxIndirectA
MessageBoxIndirectW
MessageBoxTimeoutA
MessageBoxTimeoutW
SoftModalMessageBox

Among them, MessageBoxTimeoutA, MessageBoxTimeoutW, SoftModalMessageBox are undocumented API interfaces and do not appear in the Windows SDK header file.

## Function prototype
``` c++
int MessageBox(
    HWND hWnd,          // handle to owner window
    LPCTSTR lpText,     // text in message box
    LPCTSTR lpCaption,  // message box title
    UINT uType          // message box style
);

int MessageBoxEx(
    HWND hWnd,          // handle to owner window
    LPCTSTR lpText,     // text in message box
    LPCTSTR lpCaption,  // message box title
    UINT uType,         // message box style
    WORD wLanguageId    // language identifier
);

int MessageBoxTimeout(
    HWND hwndOwner,
    LPCTSTR lpszText,
    LPCTSTR lpszCaption,
    UINT wStyle,
    WORD wLanguageId,
    DWORD dwTimeout
);

typedef struct {
    UINT      cbSize; 
    HWND      hwndOwner; 
    HINSTANCE hInstance; 
    LPCTSTR   lpszText; 
    LPCTSTR   lpszCaption; 
    DWORD     dwStyle; 
    LPCTSTR   lpszIcon; 
    DWORD_PTR dwContextHelpId; 
    MSGBOXCALLBACK lpfnMsgBoxCallback; 
    DWORD     dwLanguageId; 
} MSGBOXPARAMS, *PMSGBOXPARAMS;
int MessageBoxIndirect(CONST LPMSGBOXPARAMS lpMsgBoxParams);

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
int SoftModalMessageBox(PMSGBOXDATA lpmb);

```
## Special feature
<table>
	<tbody>
		<tr>
			<th>Feature</th>
			<th>Functions</th>
			<th>Description</th>
		</tr>
		<tr>
			<td>Modifying Icons</td>
			<td>MessageBoxIndirect</td>
			<td>The hInstance and lpszIcon in MSGBOXPARAMS specify the icon.<br>
			Note that dwStyle must contain MB_USERICON, and cannot contain any other MessageBox built-in icons (such as MB_ICONERROR), otherwise no icon will be displayed.</td>
		</tr>
		<tr>
			<td>Load button text in the specified language</td>
			<td>MessageBoxEx</td>
			<td>wLanguageId is used to specify the language of the button text. For example:<br>
			MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT).<br>Windows will load the button text from user32.dll in the corresponding language, if it fails, it will load it again in the current system language.</td>
		</tr>
		<tr>
			<td>Dialogs disappear automatically after a few seconds</td>
			<td>MessageBoxTimeout</td>
			<td>This function is undocumented.</td>
		</tr>
		<tr>
			<td>Supports up to 11 buttons</td>
			<td>SoftModalMessageBox</td>
			<td>This function is undocumented.<br>
			Theoretically, it can support more than 11 buttons, but since MessageBox's window handler only handles button IDs from IDOK (1) to IDCONTINUE (11), clicking on a button with an ID of more than 11 will have no effect.</td>
		</tr>
	</tbody>
</table>

## Notes
- SoftModalMessageBox internally constructs a DLGTEMPLATE structure, and then calls InternalDialogBox to create a dialog box. The FontHeight of DLTEMPLATE is assigned to 0x7FFF, which indicates that this dialog box uses MessageBox special font. Call SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &ncm, 0) ncm.lfMessageFont return the LOGFONT.

- Dialog size: https://devblogs.microsoft.com/oldnewthing/20110624-00/?p=10343
