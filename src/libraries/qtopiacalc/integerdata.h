/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
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
class IntegerData : public Data {
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
