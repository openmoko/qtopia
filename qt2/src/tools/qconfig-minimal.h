/****************************************************************************
** $Id: qt/src/tools/qconfig-minimal.h   2.3.12   edited 2005-10-27 $
**
** Qt/Embedded "minimal" configuration definition.
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define NO_CHECK
#ifndef QT_H
#endif // QT_H
#if !defined(QT_NO_QWS_DEPTH_16) ||  !defined(QT_NO_QWS_DEPTH_8) || !defined(QT_NO_QWS_DEPTH_32) || !defined(QT_NO_QWS_VGA_16)  
//We only need 1-bit support if we have a 1-bit screen
#define QT_NO_QWS_DEPTH_1
#endif
#define QT_NO_PALETTE
#define QT_NO_ACTION
#ifndef QT_NO_CODECS // moc?
#define QT_NO_TEXTCODEC
#endif
#define QT_NO_UNICODETABLES
#define QT_NO_IMAGEIO_BMP
#define QT_NO_IMAGEIO_PPM
#define QT_NO_IMAGEIO_XBM
#define QT_NO_IMAGEIO_XPM
#define QT_NO_IMAGEIO_PNG
#define QT_NO_ASYNC_IO
#define QT_NO_ASYNC_IMAGE_IO
#define QT_NO_FREETYPE
#define QT_NO_BDF
#define QT_NO_FONTDATABASE
#define QT_NO_TRANSLATION
#define QT_NO_MIME
#define QT_NO_SOUND
#define QT_NO_PROPERTIES
#define QT_NO_QWS_CURSOR
#define QT_NO_QWS_GFX_SPEED
#define QT_NO_NETWORK
#define QT_NO_COLORNAMES
#define QT_NO_TRANSFORMATIONS
#define QT_NO_PRINTER
#define QT_NO_PICTURE
#define QT_NO_LAYOUT
#define QT_NO_DRAWUTIL
#define QT_NO_IMAGE_TRUECOLOR
#define QT_NO_IMAGE_SMOOTHSCALE
#define QT_NO_IMAGE_TEXT
#define QT_NO_DIR
#define QT_NO_QWS_MANAGER
#define QT_NO_TEXTSTREAM
#define QT_NO_DATASTREAM
#define QT_NO_QWS_SAVEFONTS
#define QT_NO_STRINGLIST
#define QT_NO_SESSIONMANAGER
#define QT_NO_QWS_KEYBOARD

#define QT_NO_DIALOG
#define QT_NO_FRAME
#define QT_NO_SEMIMODAL

#define QT_NO_STYLE
#define QT_NO_EFFECTS
#define QT_NO_COP
#define QT_NO_QWS_TRANSFORMED
