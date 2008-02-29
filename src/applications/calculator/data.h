/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CALCULATOR_DATA_H
#define CALCULATOR_DATA_H

#include <qtopiaglobal.h>
#include <QString>

class QPixmap;

class Data {
public:
    Data();
    virtual ~Data();

    virtual bool push(char,bool commit=true);
    virtual bool del(); // Returns true if the item is to be deleted completely
    virtual void clear();

    virtual QString getType();
    virtual QString getFormattedOutput();
    virtual QPixmap *draw();
    QPixmap *cache;
protected:
    QString formattedOutput, cachedOutput;
};

#endif
