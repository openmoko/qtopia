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

#include <qtopia/qpeglobal.h>
#include <qstring.h>

class QTOPIA_EXPORT Data {
public:
    Data(){};
    virtual ~Data(){};

    virtual void push(char){};
    virtual void del(){};
    virtual void clear(){formattedOutput.truncate(0);formattedOutput.append("0");};

    virtual QString getType(){return QString("NONE");};
    virtual QString getFormattedOutput(){return formattedOutput;};
protected:
    QString formattedOutput;
};

#endif
