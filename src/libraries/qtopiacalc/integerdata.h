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
#ifndef CALC_INTEGERDATA_H
#define CALC_INTEGERDATA_H

#include "data.h"

/*
This class supports bases 2,8,10 and 16
and may be subclassed to support other
bases without having to create new
conversion functions
*/
class QTOPIA_EXPORT IntegerData : public Data {
public:
    IntegerData():Data(){};
    ~IntegerData(){};

    virtual bool appendChar(char);
    virtual void delChar();
    virtual void clear(){set(0);};

    QString getType(){return QString("INTEGER");};

    void set(int = 0,int = 10);
    int get(){return i;};
private:
    int i,base;
};

#endif
