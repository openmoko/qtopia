/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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

// Platforms where mouse cursor is never required.
#if defined(QT_QWS_IPAQ) || defined(QT_QWS_CASSIOPEIA) || defined(QT_QWS_SL5XXX)
# define QT_NO_QWS_CURSOR
# define QT_NO_QWS_MOUSE_AUTO
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
#ifndef QT_NO_IMAGEIO_MNG
# define QT_NO_IMAGEIO_MNG
#endif
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
#ifndef _WS_WIN32_
#define QT_NO_WIZARD_IMPL
#endif
#define QT_NO_QMEMORYFILE
