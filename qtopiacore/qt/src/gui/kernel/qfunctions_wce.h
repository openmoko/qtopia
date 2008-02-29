/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QFUNCTIONS_WCE_H
#define QFUNCTIONS_WCE_H

#ifdef Q_OS_TEMP

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winuser.h>
#include <winbase.h>
#include <objbase.h>
#include <kfuncs.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

#define POCKET_PC                // POCKETPC
//#undef POCKETPC                // HPCPRO

#define SetWindowLongA                SetWindowLong
#define GetWindowLongA                GetWindowLong
#define SendMessageA                SendMessage
#define calloc                        _calloc

#if !defined(NO_ERRNO_H) && defined(POCKET_PC)
#define NO_ERRNO_H
#endif

// Environment ------------------------------------------------------
char *qgetenv(const char *env);


// Time -------------------------------------------------------------
#ifndef _TM_DEFINED
#define _TM_DEFINED
struct tm {
    int tm_sec;     /* seconds after the minute - [0,59] */
    int tm_min;     /* minutes after the hour - [0,59] */
    int tm_hour;    /* hours since midnight - [0,23] */
    int tm_mday;    /* day of the month - [1,31] */
    int tm_mon;     /* months since January - [0,11] */
    int tm_year;    /* years since 1900 */
    int tm_wday;    /* days since Qt::Sunday - [0,6] */
    int tm_yday;    /* days since January 1 - [0,365] */
    int tm_isdst;   /* daylight savings time flag */
};
#endif // _TM_DEFINED

size_t            strftime(char *strDest, size_t maxsize, const char *format, const struct tm *timeptr);
struct tm  *gmtime(const time_t *timer);
struct tm  *localtime(const time_t *);
time_t            mktime(struct tm *timeptr);

// Time conversion functions
time_t         ftToTime_t(const FILETIME ft);
FILETIME time_tToFt(time_t tt);


// File I/O ---------------------------------------------------------
#define PATH_MAX                  1024
#define _O_RDONLY                0x0001
#define _O_RDWR                        0x0002
#define _O_WRONLY                0x0004
#define _O_CREAT                0x0008
#define _O_TRUNC                0x0010
#define _O_APPEND                0x0020

#define _S_IFMT                        0x0600
#define _S_IFDIR                0x0200
#define _S_IFREG                0x0400

// Regular Berkeley error constants
#define EMFILE                        ERROR_TOO_MANY_OPEN_FILES   // was 0x0800
#define ENOSPC                        ERROR_DISK_FULL                    // was 0x1000
#define EACCES                        ERROR_ACCESS_DENIED            // was 13
#define ENOENT                        ERROR_FILE_NOT_FOUND            // was 2

struct _stat
{
    int st_mode;
    int st_size;
    int st_nlink;
    time_t st_mtime;
    time_t st_atime;
    time_t st_ctime;
};

typedef int mode_t;
extern int errno;

DWORD         GetLogicalDrives(VOID);
int        _getdrive(void);
WCHAR  *_wgetcwd(WCHAR *buffer, int maxlen);
WCHAR  *_wgetdcwd(int drive, WCHAR *buffer, int maxlen);
int        _wmkdir(const WCHAR *dirname);
int        _wrmdir(const WCHAR *dirname);
int        _waccess(const WCHAR *path, int pmode);
int        _wrename(const WCHAR *oldname, const WCHAR *newname);
int        _wremove(const WCHAR *name);
int         open(const char *filename, int oflag, int pmode);
int        _wopen(const WCHAR *filename, int oflag, int pmode);
int        _wstat(const WCHAR *path, struct _stat *buffer);
long        _lseek(int handle, long offset, int origin);
int        _read(int handle, void *buffer, unsigned int count);
int        _write(int handle, const void *buffer, unsigned int count);
int        _close(int handle);
FILE   *_fdopen(int handle, const char *mode);
FILE   *fdopen(int handle, const char *mode);
void        rewind(FILE *stream);
FILE   *tmpfile(void);


// Clipboard --------------------------------------------------------
#define WM_CHANGECBCHAIN        1
#define WM_DRAWCLIPBOARD        2

BOOL ChangeClipboardChain(
    HWND hWndRemove,  // handle to window to remove
    HWND hWndNewNext  // handle to next window
);

HWND SetClipboardViewer(
    HWND hWndNewViewer   // handle to clipboard viewer window
);


// Printer ----------------------------------------------------------
#define ETO_GLYPH_INDEX                0x0010

typedef struct tagENUMLOGFONTEX {
    LOGFONT  elfLogFont;
    TCHAR  elfFullName[LF_FULLFACESIZE];
    TCHAR  elfStyle[LF_FACESIZE];
    TCHAR  elfScript[LF_FACESIZE];
} ENUMLOGFONTEX, *LPENUMLOGFONTEX;


// Graphics ---------------------------------------------------------
#ifdef POCKET_PC
#   define SM_CXCURSOR                13
#   define SM_CYCURSOR                14
#else
// ###
#define GHND                        GMEM_MOVEABLE | GMEM_ZEROINIT
#endif // POCKET_PC

BOOL ResizePalette(HPALETTE hpal, UINT nEntries);
COLORREF PALETTEINDEX(WORD wPaletteIndex);

BOOL SetWindowOrgEx(HDC hdc, int X, int Y, LPPOINT lpPoint);
BOOL TextOut(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString);
BOOL GetViewportOrgEx(HDC hdc, LPPOINT lpPoint);
BOOL GetViewportExtEx(HDC hdc, LPSIZE lpSize);
BOOL GetWindowOrgEx(HDC hdc, LPPOINT lpPoint);
BOOL GetWindowExtEx(HDC hdc, LPSIZE lpSize);

UINT qt_GetDIBColorTable(HDC hdc, DIBSECTION *ds, UINT uStartIndex, UINT cEntries, RGBQUAD *pColors);


// Other stuff ------------------------------------------------------
// ### not the real values
#define STARTF_USESTDHANDLES        1
#define CREATE_NO_WINDOW        2
#define DETACHED_PROCESS        3
#define CF_HDROP                15

void abort();
void *_expand(void* pvMemBlock, size_t iSize);
void *calloc(size_t num, size_t size);

unsigned long _beginthreadex(void *security,
                              unsigned stack_size,
                              unsigned (__stdcall *start_address)(void *),
                              void *arglist,
                              unsigned initflag,
                              unsigned *thrdaddr);
void _endthreadex(unsigned nExitCode);

#ifndef POCKET_PC
    int isprint(int c);
    int isdigit(int c);
    int isxdigit(int c);
    int isspace(int c);
    int isgraph(int c);
    double atof(const char *string);
    char *strrchr(const char *string, int c);
    double strtod(const char *nptr, char **endptr);
    long strtol(const char *nptr, char **endptr, int base);
#endif // POCKET_PC

void *bsearch(const void *key,
               const void *base0,
               size_t nmemb,
               size_t size,
               int (__cdecl *compar) (const void *, const void *));


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
///////////////// MFC compatibility functions ///////////////////
// This code has been copied from the MFC library source code  //
// and will need replacing. Some of this is not used also, and //
// needs removing                                               //
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

// Missing typedefs
#ifndef _TIME_T_DEFINED
typedef unsigned long time_t;
#define _TIME_T_DEFINED
#endif
typedef HANDLE  HDWP;
typedef HANDLE  HDROP;
typedef wchar_t _TUCHAR;
typedef LPVOID  LPPRINTER_DEFAULTS;

#ifndef WS_THICKFRAME
#define WS_THICKFRAME        WS_DLGFRAME
#endif

#if (_WIN32_WCE < 400) // CE 4.0, CE.NET has these
    typedef struct _DROPFILES
    {
        DWORD pFiles;
        POINT pt;
        BOOL fNC;
        BOOL fWide;
    } DROPFILES, FAR *LPDROPFILES;

    typedef LPVOID              LPCHOOSEFONT;
#   define RDW_INVALIDATE       (0x0001)
#   define RDW_INTERNALPAINT    (0x0002)
#   define RDW_ERASE            (0x0004)
#   define RDW_VALIDATE         (0x0008)
#   define RDW_NOERASE          (0x0020)
#   define RDW_NOCHILDREN       (0x0040)
#   define RDW_ALLCHILDREN      (0x0080)
#   define RDW_UPDATENOW        (0x0100)
#   define RDW_ERASENOW         (0x0200)
#endif // _WIN32_WCE < 400

#if (_WIN32_WCE < 210)
typedef LPVOID  LPPAGESETUPDLG;
#endif // _WIN32_WCE < 210
typedef UINT    UWORD;

// Missing definitions: not necessary equal to their Win32 values
// (the goal is to just have a clean compilation of MFC)
#define BS_USERBUTTON             BS_PUSHBUTTON
#define WS_MAXIMIZE               0
#define WS_MINIMIZE               0
#define WS_EX_CONTROLPARENT       0x00010000L
#define WS_EX_LEFTSCROLLBAR       0
#ifndef WS_EX_TOOLWINDOW
#define WS_EX_TOOLWINDOW          0
#endif
#define WS_EX_NOPARENTNOTIFY      0
#define WM_ENTERIDLE              0x0121
#define WM_PRINT                  WM_PAINT
#define WM_NCCREATE               (0x0081)
#define WM_PARENTNOTIFY           0
#define WM_NCDESTROY              (WM_APP-1)
#ifndef SW_RESTORE
#define SW_RESTORE                (SW_SHOWNORMAL)
#endif
#define SW_NORMAL                 (SW_SHOWNORMAL)
#define SW_SHOWMINNOACTIVE              (SW_HIDE)
#define MB_TYPEMASK               (0x0000000FL)
#define MB_ICONMASK               (0x000000F0L)
#define CTLCOLOR_SCROLLBAR        CTLCOLOR_EDIT
#define PSM_CANCELTOCLOSE         (WM_USER + 107)
#define ESB_ENABLE_BOTH           (0x0000)
#define RDW_NOINTERNALPAINT       (0x0010)
#define RDW_FRAME                 (0x0400)
#define RDW_NOFRAME               (0x0800)
#ifndef DCX_CACHE
#define DCX_CACHE                 (0x00000002L)
#endif
#define WAIT_OBJECT_0             0x00000000L
#define PRF_CHILDREN              0x00000010L
#define PRF_CLIENT                0x00000004L
#define HELP_HELPFILE             (0x0000L)
#define MSGF_MENU                 2
#define pshHelp                   0x040E
#define SM_DBCSENABLED            42
#define MF_BITMAP                 0x00000004L
#define MF_DISABLED               0
#define FW_REGULAR                FW_NORMAL
#define MB_TASKMODAL              0
#define MB_SYSTEMMODAL            MB_APPLMODAL
#define PDERR_DNDMMISMATCH              0x1009
#define PDERR_DEFAULTDIFFERENT    0x100C
#define IDB_HIST_SMALL_COLOR      8
#define IDB_HIST_LARGE_COLOR      9
#define DEFAULT_GUI_FONT          SYSTEM_FONT
#define SFGAO_LINK                0x00010000L
#ifndef _MAX_FNAME
#define _MAX_FNAME                 64
#endif
#ifndef SWP_NOREDRAW
#define SWP_NOREDRAW               0
#endif
#ifndef SBS_SIZEBOX
#define SBS_SIZEBOX               0
#endif
#ifndef SBS_SIZEGRIP
#define SBS_SIZEGRIP              0
#endif
#define SC_SIZE                   (0xF000)
#define WSAGETSELECTEVENT(lParam) LOWORD(lParam)
#define WSAGETSELECTERROR(lParam) HIWORD(lParam)
#define HWND_TOPMOST              ((HWND)-1)
#define HWND_NOTOPMOST                   ((HWND)-2)
#define HCBT_CREATEWND            (3)
#define CC_SHOWHELP               0
#define PS_DOT                    2
#define PD_ALLPAGES               0
#define PD_USEDEVMODECOPIES       0
#define PD_NOSELECTION            0
#define PD_HIDEPRINTTOFILE        0
#define PD_NOPAGENUMS             0
#define CF_METAFILEPICT           3
#define CWP_ALL                   0x0000
#define CWP_SKIPINVISIBLE         0x0001
#define CWP_SKIPDISABLED          0x0002
#define CWP_SKIPTRANSPARENT       0x0004
#define MM_LOMETRIC               2
#define MM_HIMETRIC               3
#define MM_LOENGLISH              4
#define MM_HIENGLISH              5
#define MM_TWIPS                  6
#define MM_ISOTROPIC              7
#define MM_ANISOTROPIC            8
#define OLEUI_FALSE               0
#define OLEUI_SUCCESS             1
#define OLEUI_OK                  1
#define OLEUI_CANCEL              2
#define KF_EXTENDED               0x0100
#define KF_DLGMODE                0x0800
#define KF_MENUMODE               0x1000
#define KF_ALTDOWN                0x2000
#define KF_REPEAT                 0x4000
#define KF_UP                     0x8000
#define IDB_STD_SMALL_MONO        2
#define IDB_STD_LARGE_MONO        3
#define IDB_VIEW_SMALL_MONO       6
#define IDB_VIEW_LARGE_MONO       7
#define SPI_GETWORKAREA           48
#define LBSELCHSTRING             TEXT("commdlg_LBSelChangedNotify")
#define SHAREVISTRING             TEXT("commdlg_ShareViolation")
#define FILEOKSTRING              TEXT("commdlg_FileNameOK")
#define COLOROKSTRING             TEXT("commdlg_ColorOK")
#define SETRGBSTRING              TEXT("commdlg_SetRGBColor")
#define HELPMSGSTRING             TEXT("commdlg_help")
#define FINDMSGSTRING             TEXT("commdlg_FindReplace")
#define DRAGLISTMSGSTRING         TEXT("commctrl_DragListMsg")

#define OFN_ENABLESIZING 0

#ifndef WM_SETCURSOR
        #define WM_SETCURSOR 0x0020
        #define IDC_ARROW           MAKEINTRESOURCE(32512)
        #define IDC_IBEAM           MAKEINTRESOURCE(32513)
        #define IDC_WAIT            MAKEINTRESOURCE(32514)
        #define IDC_CROSS           MAKEINTRESOURCE(32515)
        #define IDC_UPARROW         MAKEINTRESOURCE(32516)
        #define IDC_SIZE            MAKEINTRESOURCE(32646)
        #define IDC_ICON            MAKEINTRESOURCE(32512)
        #define IDC_SIZENWSE        MAKEINTRESOURCE(32642)
        #define IDC_SIZENESW        MAKEINTRESOURCE(32643)
        #define IDC_SIZEWE          MAKEINTRESOURCE(32644)
        #define IDC_SIZENS          MAKEINTRESOURCE(32645)
        #define IDC_SIZEALL         MAKEINTRESOURCE(32646)
        #define IDC_NO              MAKEINTRESOURCE(32648)
        #define IDC_APPSTARTING     MAKEINTRESOURCE(32650)
        #define IDC_HELP            MAKEINTRESOURCE(32651)
        #define IDC_HAND            MAKEINTRESOURCE(32649)
#endif

#if defined(_MIPS_)
extern "C" void _asm(char *, ...);
#endif

#define GMEM_MOVEABLE             LMEM_MOVEABLE
#define GMEM_FIXED                LMEM_FIXED
#define GMEM_ZEROINIT             LMEM_ZEROINIT
#define GMEM_INVALID_HANDLE       LMEM_INVALID_HANDLE
#define GMEM_LOCKCOUNT            LMEM_LOCKCOUNT
#define GPTR                      LPTR
#if (_WIN32_WCE < 300)
#define GMEM_SHARE                0
#endif // _WIN32_WCE

// WinCE: CESYSGEN prunes the following FRP defines,
// and INTERNET_TRANSFER_TYPE_ASCII breaks in wininet.h
#undef FTP_TRANSFER_TYPE_ASCII
#define FTP_TRANSFER_TYPE_ASCII 0x00000001
#undef FTP_TRANSFER_TYPE_BINARY
#define FTP_TRANSFER_TYPE_BINARY 0x00000002

#define MM_TEXT 1
typedef DWORD OLE_COLOR;
#define WS_OVERLAPPEDWINDOW 0

#ifndef MF_BITMAP
#define MF_BITMAP 0x00000004L
#endif

#ifndef WS_EX_CAPTIONOKBTN
#define WS_EX_CAPTIONOKBTN 0x80000000L
#endif

#ifndef WS_EX_NODRAG
#define WS_EX_NODRAG       0x40000000L
#endif

#define FR_DOWN                         0x00000001
#define FR_WHOLEWORD                    0x00000002
#define FR_MATCHCASE                    0x00000004
#define FR_FINDNEXT                     0x00000008
#define FR_REPLACE                      0x00000010
#define FR_REPLACEALL                   0x00000020
#define FR_DIALOGTERM                   0x00000040
#define FR_SHOWHELP                     0x00000080
#define FR_ENABLEHOOK                   0x00000100
#define FR_ENABLETEMPLATE               0x00000200
#define FR_NOUPDOWN                     0x00000400
#define FR_NOMATCHCASE                  0x00000800
#define FR_NOWHOLEWORD                  0x00001000
#define FR_ENABLETEMPLATEHANDLE         0x00002000
#define FR_HIDEUPDOWN                   0x00004000
#define FR_HIDEMATCHCASE                0x00008000
#define FR_HIDEWHOLEWORD                0x00010000
typedef UINT (APIENTRY *LPFRHOOKPROC) (HWND, UINT, WPARAM, LPARAM);

#ifndef POCKET_PC
HGLOBAL GlobalAlloc(UINT uFlags, DWORD dwBytes);
HGLOBAL GlobalFree(HGLOBAL hMem);
HGLOBAL GlobalReAlloc(HGLOBAL hMem, DWORD dwBytes, UINT uFlags);
DWORD   GlobalSize(HGLOBAL hMem);
LPVOID  GlobalLock(HGLOBAL hMem);
BOOL    GlobalUnlock(HGLOBAL hMem);
HGLOBAL GlobalHandle(LPCVOID pMem);
UINT    GlobalFlags(HGLOBAL hMem);
#endif

#ifdef __cplusplus
}        // Extern C.
#endif

#endif // Q_OS_TEMP

// silence syncqt
QT_BEGIN_HEADER
QT_MODULE(Gui)
QT_END_HEADER

#endif // QFUNCTIONS_WCE_H
