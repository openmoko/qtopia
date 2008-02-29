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
#ifdef Q_OS_TEMP

#include <windows.h>
#include <winbase.h>
#include <kfuncs.h>
#include <stdio.h>

#include "qfunctions_wce.h"
#include "qplatformdefs.h"
#include "qstring.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "uuid.lib")

#ifdef __cplusplus
extern "C" {
#endif


// WCE Debug --------------------------------------------------------
// Alter the following to increase debug output
# define QCE_DEBUG_LEVEL 0

#if (QCE_DEBUG_LEVEL > 0)
#   define QCE_DEBUG(y, x) if (y <= QCE_DEBUG_LEVEL) { x; }
#else
#   define QCE_DEBUG(y, x)
#endif


// Environment ------------------------------------------------------
// Windows CE was no concept of environment settings, therefore we
// just return 'root folder'.
char *qgetenv(char const *) { return "\\"; }


// Time -------------------------------------------------------------
size_t strftime(char *strDest, size_t maxsize, const char *format, const struct tm *timeptr) {
    return 0;
}

struct tm *gmtime(const time_t *timer) {
    return NULL;
}

struct tm *localtime(const time_t *timer) {
    return NULL;
}

time_t mktime(struct tm *timeptr) {
    return 0;
}

time_t ftToTime_t(const FILETIME ft)
{
    ULARGE_INTEGER li;
    li.LowPart  = ft.dwLowDateTime;
    li.HighPart = ft.dwHighDateTime;

    // 100-ns to seconds
    li.QuadPart /= 10000000;

    // FILETIME is from 1601-01-01 T 00:00:00
    // time_t   is from 1970-01-01 T 00:00:00
    // 1970 - 1601 = 369 year (89 leap years)
    //
    // ((369y*365d) + 89d) *24h *60min *60sec
    // = 11644473600 seconds
    li.QuadPart -= 11644473600;
    return li.LowPart;
}

FILETIME time_tToFt(time_t tt)
{
    ULARGE_INTEGER li;
    li.QuadPart  = tt;
    li.QuadPart += 11644473600;
    li.QuadPart *= 10000000;

    FILETIME ft;
    ft.dwLowDateTime = li.LowPart;
    ft.dwHighDateTime = li.HighPart;
    return ft;
}


// File I/O ---------------------------------------------------------
int errno = 0;

DWORD GetLogicalDrives(VOID)
{
    return 1;
}

int _getdrive(void)
{
    return 1;
}
WCHAR *_wgetcwd(WCHAR *buffer, int maxlen)
{
    return wcscpy(buffer, L"\\");
}

WCHAR *_wgetdcwd(int drive, WCHAR *buffer, int maxlen)
{
    return wcscpy(buffer, L"\\");
}

int _wmkdir(const WCHAR *dirname)
{
    return CreateDirectory(dirname, 0) ? 0 : -1;
}

int _wrmdir(const WCHAR *dirname)
{
    return RemoveDirectory(dirname) ? 0 : -1;
}

int _waccess(const WCHAR *path, int pmode)
{
    DWORD res = GetFileAttributes(path);
    if (0xFFFFFFFF == res)
        return -1;

    if ((pmode & W_OK) && (res & FILE_ATTRIBUTE_READONLY))
        return -1;

    if ((pmode & X_OK) && !(res & FILE_ATTRIBUTE_DIRECTORY)) {
        QString file = QString::fromUcs2(path);
        if (!(file.endsWith(".exe") ||
               file.endsWith(".com")))
            return -1;
    }

    return 0;
}

int _wrename(const WCHAR *oldname, const WCHAR *newname)
{
    return !MoveFile(oldname, newname);
}

int _wremove(const WCHAR *name)
{
    return !DeleteFile(name);
}

int open(const char *filename, int oflag, int pmode)
{
    QString fn(filename);
    return _wopen((WCHAR*)fn.ucs2(), oflag, pmode);
}

int _wopen(const WCHAR *filename, int oflag, int pmode)
{
    WCHAR *flag;

    if (oflag & _O_APPEND) {
        if (oflag & _O_WRONLY) {
            flag = L"a";
        } else if (oflag & _O_RDWR) {
            flag = L"a+";
        }
    } else if (oflag & _O_WRONLY) {
        flag = L"w";
    } else if (oflag & _O_RDWR) {
        flag = L"w+"; // slightly different from "r+" where the file must exist
    } else if (oflag & _O_RDONLY) {
        flag = L"r";
    } else {
        flag = L"";
    }

    int retval = (int)_wfopen(filename, flag);
    return (retval == NULL) ? -1 : retval;
}

int _wstat(const WCHAR *path, struct _stat *buffer)
{
    WIN32_FIND_DATA finfo;
    HANDLE ff = FindFirstFile(path, &finfo);

    if (ff == INVALID_HANDLE_VALUE)
        return -1;

    buffer->st_ctime = ftToTime_t(finfo.ftCreationTime);
    buffer->st_atime = ftToTime_t(finfo.ftLastAccessTime);
    buffer->st_mtime = ftToTime_t(finfo.ftLastWriteTime);
    buffer->st_nlink = 0;
    buffer->st_size  = finfo.nFileSizeLow; // ### missing high!
    buffer->st_mode  = (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? _S_IFDIR : _S_IFREG;
    buffer->st_mode |= (finfo.dwFileAttributes & FILE_ATTRIBUTE_READONLY) ? _O_RDONLY : _O_RDWR;

    return (FindClose(ff) == 0);
}

long _lseek(int handle, long offset, int origin)
{
    return fseek((FILE*)handle, offset, origin);
}

int _read(int handle, void *buffer, unsigned int count)
{
    return fread(buffer, 1, count, (FILE*)handle);
}

int _write(int handle, const void *buffer, unsigned int count)
{
    return fwrite(buffer, 1, count, (FILE*)handle);
}

int _close(int handle)
{
    if (!handle)
        return 0;
    return fclose((FILE*)handle);
}

FILE *_fdopen(int handle, const char *mode)
{
    return (FILE*)handle;
}

FILE *fdopen(int handle, const char *mode)
{
    return (FILE*)handle;
}

void rewind(FILE *stream)
{
    fseek(stream, 0L, SEEK_SET);
}

FILE *tmpfile(void)
{
    static long i = 0;
    char name[16];
    sprintf(name, "tmp%i", i++);
    return fopen(name, "r+");
}


// Clipboard --------------------------------------------------------
BOOL ChangeClipboardChain(HWND hWndRemove, HWND hWndNewNext)
{
    return false;
}

HWND SetClipboardViewer(HWND hWndNewViewer)
{
    return NULL;
}


// Printer ----------------------------------------------------------


// Graphics ---------------------------------------------------------
BOOL ResizePalette(HPALETTE hpal, UINT nEntries) {
    return false;
}

COLORREF PALETTEINDEX(WORD wPaletteIndex) {
    return 0;
}

BOOL SetWindowOrgEx(HDC hdc, int X, int Y, LPPOINT lpPoint) {
    // SetViewportOrgEx(hdc, -X, -Y, lpPoint);
    // return SetViewportOrgEx(hdc, X - 30, Y - 30, lpPoint);
    // return ::SetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
    return true;
}

BOOL TextOut(HDC hdc, int nXStart, int nYStart, LPCTSTR lpString, int cbString) {
    return ExtTextOut(hdc, nXStart, nYStart - 16, 0, NULL, lpString, cbString, NULL);
}

BOOL GetViewportOrgEx(HDC hdc, LPPOINT lpPoint)
{
    if (hdc == NULL)
        return false;
    lpPoint->x = 0;                // origin is always (0,0)
    lpPoint->y = 0;
    return true;
}

BOOL GetViewportExtEx(HDC hdc, LPSIZE lpSize)
{
    if (hdc == NULL)
        return false;
    lpSize->cx = 1;                // extent is always 1,1
    lpSize->cy = 1;
    return true;
}

BOOL GetWindowOrgEx(HDC hdc, LPPOINT lpPoint)
{
    if (hdc == NULL)
        return false;
    lpPoint->x = 0;                // origin is always (0,0)
    lpPoint->y = 0;
    return true;
}

BOOL GetWindowExtEx(HDC hdc, LPSIZE lpSize)
{
    if (hdc == NULL)
        return false;
    lpSize->cx = 1;                // extent is always 1,1
    lpSize->cy = 1;
    return true;
}

UINT qt_GetDIBColorTable(HDC hdc, DIBSECTION *ds, UINT uStartIndex, UINT cEntries, RGBQUAD *pColors)
{
    if (pColors == NULL ||            // No place for palette
         ds->dsBmih.biBitCount > 8) // Not Palettized
        return 0;

    LPBYTE pBits = (LPBYTE)ds->dsBm.bmBits;
    BYTE OldPalIndex = *pBits;

    // Create a mask for the palette index bits for 1, 2, 4, and 8 bpp
    WORD wIndexMask = (0xFF << (8 - ds->dsBmih.biBitCount)) & 0x00FF;

    UINT TestPixelY = 0;                   // We always use Top-down DIBs
    // ...and the following check fails...
   // TestPixelY = ds->dsBmih.biHeight <= 0 ? 0 :        //  Top-down DIB
                 //ds->dsBm.bmHeight-1;                // Bottom-up DIB

    // Debug level 1 only output
    QCE_DEBUG(1, {
        qDebug("WCE - GetDIBColorTable()");
        qDebug("     wIndexMask               = 0x%X", wIndexMask);
        qDebug("     TestPixelY               = %d", TestPixelY);
        qDebug("   DIBSECTION:");
        qDebug("     dsBmih.biWidth       (W) = %d", ds->dsBmih.biWidth);
        qDebug("     dsBmih.biHeight      (H) = %d", ds->dsBmih.biHeight);
        qDebug("     dsBmih.biBitCount    (D) = %d", ds->dsBmih.biBitCount);
        qDebug("     dsBmih.biClrUsed         = %d", ds->dsBmih.biClrUsed);
        qDebug("     dsBmih.biClrImportant    = %d", ds->dsBmih.biClrImportant);
        qDebug("     dsBmih.biXPelsPerMeter   = %d", ds->dsBmih.biXPelsPerMeter);
        qDebug("     dsBmih.biYPelsPerMeter   = %d", ds->dsBmih.biYPelsPerMeter);
        qDebug("     dsBmih.biCompression     = %d", ds->dsBmih.biCompression);
        qDebug("     dsBmih.biPlanes          = %d", ds->dsBmih.biPlanes);
        qDebug("     dsBmih.biSize   (struct) = %d", ds->dsBmih.biSize);
        qDebug("     dsBmih.biSizeImage (img) = %d", ds->dsBmih.biSizeImage);
        qDebug("     dsBm.bmType              = %d", ds->dsBm.bmType);
        qDebug("     dsBm.bmBits       (data) = 0x%p", ds->dsBm.bmBits );
    })

    UINT cColors = ds->dsBmih.biClrUsed ? ds->dsBmih.biClrUsed :
                   1 << (ds->dsBmih.biBitCount*ds->dsBmih.biPlanes);
    cColors = qMin(cColors, cEntries);

    QCE_DEBUG(2, qDebug("   ColorTable:"));
    for (UINT iColor = uStartIndex; iColor < cColors; iColor++) {
        *pBits = (iColor << (8 - ds->dsBmih.biBitCount)) |
                 (*pBits & ~wIndexMask);

        COLORREF rgbColor = GetPixel(hdc, 0, TestPixelY);
        pColors[iColor - uStartIndex].rgbReserved = 0;
        pColors[iColor - uStartIndex].rgbBlue     = GetBValue(rgbColor);
        pColors[iColor - uStartIndex].rgbRed      = GetRValue(rgbColor);
        pColors[iColor - uStartIndex].rgbGreen    = GetGValue(rgbColor);

        // Debug level 2 only output
        QCE_DEBUG(2, qDebug("     %2X - ColorRef: 0x%x", *pBits, rgbColor))
    }

    *pBits = OldPalIndex;
    return cColors;
}


// Other stuff ------------------------------------------------------
void abort()
{
    exit(3);
}

void *_expand(void* pvMemBlock, size_t iSize)
{
    return realloc(pvMemBlock, iSize);
}

void *calloc(size_t num, size_t size)
{
    void *ptr = malloc(num * size);
    if(ptr)
        memset(ptr, 0, num * size);
    return ptr;
}

unsigned long _beginthreadex(void *security,
                              unsigned stack_size,
                              unsigned (__stdcall *start_address)(void *),
                              void *arglist,
                              unsigned initflag,
                              unsigned *thrdaddr) {
    return (unsigned long)
        CreateThread((LPSECURITY_ATTRIBUTES)security,
                      (DWORD)stack_size,
                      (LPTHREAD_START_ROUTINE)start_address,
                      (LPVOID)arglist,
                      (DWORD)initflag | CREATE_SUSPENDED,
                      (LPDWORD)thrdaddr);
}

void _endthreadex(unsigned nExitCode) {
    ExitThread((DWORD)nExitCode);
}

#ifndef POCKET_PC
// ### very rough approximation of these sets
// A proper implementation uses tables for these sets, but this will do for now
int isprint (int c) { return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || isdigit(c) || isspace(c)) ? 1 : 0; }
int isdigit (int c) { return (((c >= '1') && (c <= '9')) || (c == '0')) ? 1 : 0; }
int isxdigit(int c) { return (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')) || isdigit(c)) ? 1 : 0; }
int isspace (int c) { return ((c == ' ') || (c == '\t')) ? 1 : 0; }

const int _U = 0x01;        // Upper Case
const int _L = 0x02;        // Lower Case
const int _N = 0x04;        // Number
const int _S = 0x08;        // Space character
const int _P = 0x10;        // Punctuation
const int _C = 0x20;        // Control character
const int _X = 0x40;        // Hexadecimal character
const int _B = 0x80;        // Blank space ' '

// Table of character classes
//
const int ctype[]= {
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x20, 0x28, 0x28, 0x28, 0x28, 0x28, 0x20, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
0x88, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44, 0x44,
0x44, 0x44, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
0x10, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
0x01, 0x01, 0x01, 0x10, 0x10, 0x10, 0x10, 0x10,
0x10, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x02,
0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
0x02, 0x02, 0x02, 0x10, 0x10, 0x10, 0x10, 0x20,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

int isgraph(int c)
{
    int i = ((int)c & 0xff);
    return (ctype[i] & (_P|_U|_L|_N)) > 0;
}

double atof(const char *string)
{
    QString value(string);
    return value.toDouble();
}

char *strrchr(const char *string, int c)
{
    int len = strlen(string);
    for (int i = len - 1; i >= 0; i--) {
        if ((int)string[i] == c)
            return (char *)&(string[i]);
    }
    return NULL;
}

// BEGIN COPYRIGHTED FUNCTIONS --------------------------------------
// The functions below are taken from libpng v1.2.5 - October 3, 2002

/*
 * COPYRIGHT NOTICE, DISCLAIMER, and LICENSE:
 *
 * If you modify libpng you may insert additional notices immediately following
 * this sentence.
 *
 * libpng versions 1.0.7, July 1, 2000, through 1.2.5, October 3, 2002, are
 * Copyright (c) 2000-2002 Glenn Randers-Pehrson, and are
 * distributed according to the same disclaimer and license as libpng-1.0.6
 * with the following individuals added to the list of Contributing Authors
 *
 *    Simon-Pierre Cadieux
 *    Eric S. Raymond
 *    Gilles Vollant
 *
 * and with the following additions to the disclaimer:
 *
 *    There is no warranty against interference with your enjoyment of the
 *    library or against infringement.  There is no warranty that our
 *    efforts or the library will fulfill any of your particular purposes
 *    or needs.  This library is provided with all faults, and the entire
 *    risk of satisfactory quality, performance, accuracy, and effort is with
 *    the user.
 *
 * libpng versions 0.97, January 1998, through 1.0.6, March 20, 2000, are
 * Copyright (c) 1998, 1999, 2000 Glenn Randers-Pehrson
 * Distributed according to the same disclaimer and license as libpng-0.96,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    Tom Lane
 *    Glenn Randers-Pehrson
 *    Willem van Schaik
 *
 * libpng versions 0.89, June 1996, through 0.96, May 1997, are
 * Copyright (c) 1996, 1997 Andreas Dilger
 * Distributed according to the same disclaimer and license as libpng-0.88,
 * with the following individuals added to the list of Contributing Authors:
 *
 *    John Bowler
 *    Kevin Bracey
 *    Sam Bushell
 *    Magnus Holmgren
 *    Greg Roelofs
 *    Tom Tanner
 *
 * libpng versions 0.5, May 1995, through 0.88, January 1996, are
 * Copyright (c) 1995, 1996 Guy Eric Schalnat, Group 42, Inc.
 *
 * For the purposes of this copyright and license, "Contributing Authors"
 * is defined as the following set of individuals:
 *
 *    Andreas Dilger
 *    Dave Martindale
 *    Guy Eric Schalnat
 *    Paul Schmidt
 *    Tim Wegner
 *
 * The PNG Reference Library is supplied "AS IS".  The Contributing Authors
 * and Group 42, Inc. disclaim all warranties, expressed or implied,
 * including, without limitation, the warranties of merchantability and of
 * fitness for any purpose.  The Contributing Authors and Group 42, Inc.
 * assume no liability for direct, indirect, incidental, special, exemplary,
 * or consequential damages, which may result from the use of the PNG
 * Reference Library, even if advised of the possibility of such damage.
 *
 * Permission is hereby granted to use, copy, modify, and distribute this
 * source code, or portions hereof, for any purpose, without fee, subject
 * to the following restrictions:
 *
 * 1. The origin of this source code must not be misrepresented.
 *
 * 2. Altered versions must be plainly marked as such and
 * must not be misrepresented as being the original source.
 *
 * 3. This Copyright notice may not be removed or altered from
 *    any source or altered source distribution.
 *
 * The Contributing Authors and Group 42, Inc. specifically permit, without
 * fee, and encourage the use of this source code as a component to
 * supporting the PNG file format in commercial products.  If you use this
 * source code in a product, acknowledgment is not required but would be
 * appreciated.
 */

double strtod(const char *nptr, char **endptr)
{
   double result = 0;
   int len;
   wchar_t *str, *end;

   len = MultiByteToWideChar(CP_ACP, 0, nptr, -1, NULL, 0);
   str = (wchar_t *)malloc(len * sizeof(wchar_t));
   if (NULL != str)
   {
      MultiByteToWideChar(CP_ACP, 0, nptr, -1, str, len);
      result = wcstod(str, &end);
      len = WideCharToMultiByte(CP_ACP, 0, end, -1, NULL, 0, NULL, NULL);
      *endptr = (char *)nptr + (strlen(nptr) - len + 1);
      free(str);
   }
   return result;
}

long strtol(const char *nptr, char **endptr, int base)
{
   long result = 0;
   int len;
   wchar_t *str, *end;

   len = MultiByteToWideChar(CP_ACP, 0, nptr, -1, NULL, 0);
   str = (wchar_t *)malloc(len * sizeof(wchar_t));
   if (NULL != str)
   {
      MultiByteToWideChar(CP_ACP, 0, nptr, -1, str, len);
      result = wcstol(str, &end, base);
      len = WideCharToMultiByte(CP_ACP, 0, end, -1, NULL, 0, NULL, NULL);
      *endptr = (char *)nptr + (strlen(nptr) - len + 1);
      free(str);
   }
   return result;
}
// END COPYRIGHTED FUNCTIONS ----------------------------------------
#endif // POCKET_PC

// BEGIN COPYRIGHTED FUNCTION ---------------------------------------
// The bsearch routine below is taken from BSD 4.4-lite

/*
 * Copyright (c) 1990, 1993
 *        The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgment:
 *        This product includes software developed by the University of
 *        California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

 /*
 * Perform a binary search.
 *
 * The code below is a bit sneaky.  After a comparison fails, we
 * divide the work in half by moving either left or right. If lim
 * is odd, moving left simply involves halving lim: e.g., when lim
 * is 5 we look at item 2, so we change lim to 2 so that we will
 * look at items 0 & 1.  If lim is even, the same applies.  If lim
 * is odd, moving right again involes halving lim, this time moving
 * the base up one item past p: e.g., when lim is 5 we change base
 * to item 3 and make lim 2 so that we will look at items 3 and 4.
 * If lim is even, however, we have to shrink it by one before
 * halving: e.g., when lim is 4, we still looked at item 2, so we
 * have to make lim 3, then halve, obtaining 1, so that we will only
 * look at item 3.
 */
void *bsearch(register const void *key,
               const void *base0,
               size_t nmemb,
               register size_t size,
               register int (__cdecl *compar) (const void *, const void *))
{
    register const char *base = (const char *)base0;
    register size_t lim;
    register int cmp;
    register const void *p;

    for (lim = nmemb; lim != 0; lim >>= 1) {
        p = base + (lim >> 1) * size;
        cmp = (*compar)(key, p);
        if (cmp == 0)
            return ((void *)p);
        if (cmp > 0) {        /* key > p: move right */
            base = (char *)p + size;
            lim--;
        }                /* else move left */
    }
    return (NULL);
}
// END COPYRIGHTED FUNCTION -----------------------------------------

/*
BOOL SystemParametersInfo(UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni) {
    return false;
}

HMENU GetMenu(HWND hWnd) {
    return NULL;
}

BOOL SetMenu(HWND hWnd, HMENU hMenu) {
    return false;
}

BOOL CALLBACK FirstDlgProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hWnd,nMsg,wParam,lParam);
};

LRESULT CALLBACK FirstDefWindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hWnd,nMsg,wParam,lParam);
}

BOOL PreCreateWindow(CREATESTRUCT& cs) {
    return true;
}

void PostCreateWindow(CREATESTRUCT& cs, HWND hWnd, HMENU nIDorHMenu) {
    // Set the menu (SetMenu just caches it away in CWnd)
    if((hWnd != NULL) && (HIWORD(nIDorHMenu) != NULL))
        SetMenu(hWnd, nIDorHMenu);
}

HRGN CreateRectRgn(int x1, int y1, int x2, int y2) {
    RECT rect = { x1, y1, x2, y2 };
    return ::CreateRectRgnIndirect(&rect);
}
*/

//#define        TRACE0(x)
//#ifndef _countof
//#define _countof(array) (sizeof(array)/sizeof(array[0]))
//#endif

//HANDLE CreateSemaphore(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
//                        LONG lInitialCount, LONG lMaximumCount, LPCTSTR lpName) { return NULL; }
//BOOL ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount) { return true; }
// extern "C" void  __cdecl exit(int);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // Q_OS_TEMP
