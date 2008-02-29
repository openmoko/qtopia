/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qtopia Environment.
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

#ifndef CALCULATOR_DATA_H
#define CALCULATOR_DATA_H

//#define NEW_STYLE_DISPLAY

#include <qtopia/qpeglobal.h>
#include <qstring.h>
#include <qpainter.h>

class QTOPIA_EXPORT Data {
public:
    Data();
    virtual ~Data();

    virtual bool push(char,bool commit=TRUE);
    virtual void del();
    virtual void clear();

    virtual QString getType();
    virtual QString getFormattedOutput();
#ifdef NEW_STYLE_DISPLAY
    virtual void draw(QPainter *);
#endif
protected:
    QString formattedOutput;
};

#endif
