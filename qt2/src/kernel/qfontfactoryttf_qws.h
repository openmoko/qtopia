/****************************************************************************
** $Id: qt/src/kernel/qfontfactoryttf_qws.h   2.3.12   edited 2005-10-27 $
**
** Definition of QFontFactory for Truetype class for Embedded Qt
**
** Created : 940721
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
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

#ifndef QFONTFACTORY_FT_H
#define QFONTFACTORY_FT_H

#ifndef QT_H
#include "qfontmanager_qws.h"
#endif // QT_H

#ifndef QT_NO_FREETYPE
#if 1 //new freetype version
#include <ft2build.h>
#include FT_FREETYPE_H
#else
extern "C" {
#include <freetype/freetype.h>
}
#endif

// ascent, descent, width(ch), width(string), maxwidth?
// leftbearing, rightbearing, minleftbearing,minrightbearing
// leading

class QFontFactoryFT : public QFontFactory {

public:

    QFontFactoryFT();
    virtual ~QFontFactoryFT();

    QRenderedFont * get(const QFontDef &,QDiskFont *);
    virtual void load(QDiskFont *) const;
    virtual void unload(QDiskFont *);
    virtual QString name();

private:

    friend class QRenderedFontFT;
    FT_Library library;
};

#endif // QT_NO_FREETYPE

#endif // QFONTFACTORY_FT_H
