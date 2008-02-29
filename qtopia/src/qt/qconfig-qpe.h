/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef QT_H
#endif // QT_H

// Note that disabling more features will produce a libqte that is not
// compatible with other libqte builds.

#ifndef QT_DLL
#define QT_DLL // Internal
#endif

// Mouse not normally supported or needed.
// (enabling these, eg. so that USB mouse works can be done,
// but cursor should be turned on/off when device connected)
#ifndef QT_NO_QWS_CURSOR
#define QT_NO_QWS_CURSOR
#endif
#ifndef QT_NO_QWS_MOUSE_AUTO
#define QT_NO_QWS_MOUSE_AUTO
#endif
#ifndef QT_NO_QWS_MOUSE_PC
#define QT_NO_QWS_MOUSE_PC
#endif

// Qtopia Phone
#if defined(QTOPIA_PHONE)
# if !defined(QT_KEYPAD_MODE)
#  define QT_KEYPAD_MODE
# endif
#endif

// No builtin codecs (Qtopia uses plugins to add codecs individually)
#ifndef QT_NO_CODECS
#define QT_NO_CODECS
#endif

// No builtin font factories (Qtopia uses plugins)
#define QT_NO_FREETYPE
#define QT_NO_BDF

// No builtin styles (Qtopia uses plugins)
#define QT_NO_STYLE_POCKETPC
#ifndef QT_NO_STYLE_AQUA
# define QT_NO_STYLE_AQUA
#endif
#define QT_NO_STYLE_MOTIF
#define QT_NO_STYLE_PLATINUM

// No builtin WM styles (Qtopia uses plugins)
#define QT_NO_QWS_BEOS_WM_STYLE
#define QT_NO_QWS_KDE2_WM_STYLE
#define QT_NO_QWS_KDE_WM_STYLE
#define QT_NO_QWS_WINDOWS_WM_STYLE

// Space savings: disable expensive features
#define QT_NO_UNICODETABLES
// Qtopia now uses MNG for animations
//#ifndef QT_NO_IMAGEIO_MNG
//# define QT_NO_IMAGEIO_MNG
//#endif
#define QT_NO_PROPERTIES
#define QT_NO_COLORNAMES

// Space savings: features insufficiently useful on handheld device
#define QT_NO_IMAGEIO_PPM
#define QT_NO_NETWORKPROTOCOL
#define QT_NO_PICTURE
#define QT_NO_PRINTER
#define QT_NO_QWS_SAVEFONTS

// Space savings: features impractical on small display
#define QT_NO_COLORDIALOG
#define QT_NO_FILEDIALOG
#define QT_NO_FONTDIALOG

// Space savings: features impractical on stylus device
#define QT_NO_DIAL
#define QT_NO_DRAGANDDROP
#define QT_NO_IMAGE_TEXT
#define QT_NO_INPUTDIALOG
#define QT_NO_PRINTDIALOG
#define QT_NO_PROGRESSDIALOG
#define QT_NO_SEMIMODAL
#define QT_NO_SIZEGRIP
#define QT_NO_SPLITTER
#define QT_NO_WORKSPACE

// Speed savings: features that incur unacceptable performance penalty
#define QT_NO_DOM
#define QT_NO_EFFECTS
#define QT_NO_TRANSFORMATIONS

// Qt 3 features (not relevant at this time)
#define QT_NO_TRANSLATION_BUILDER
#define QT_NO_COMPLEXTEXT

// Features included directly in Qtopia
#if !(defined (_OS_WIN32_) || defined (_WS_WIN32_))
#define QT_NO_WIZARD_IMPL
#endif
#define QT_NO_QMEMORYFILE

// Features for Qtopia under Win32 SDK
#if defined (_OS_WIN32_) || defined (_WS_WIN32_)
#define QT_NO_QWS_LINUXFB
#define QT_NO_QWS_MACH64
#define QT_NO_QWS_VOODOO3
#define QT_NO_QWS_MATROX
#define QT_NO_QWS_VNC
#define QT_NO_QWS_TRANSFORMED
#define QT_NO_QWS_VGA_16
#define QT_NO_QWS_SVGALIB
#endif

// Disable these drivers under PowerPC because some of them cause compile/link failures
#if defined(__powerpc__)
#define QT_NO_QWS_MACH64
#define QT_NO_QWS_VNC
#define QT_NO_QWS_VGA_16
#define QT_NO_QWS_SVGALIB
#endif
