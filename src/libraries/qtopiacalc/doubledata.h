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
#ifndef CALC_DOUBLEDATA_H
#define CALC_DOUBLEDATA_H

#include "data.h"

// Data type
class DoubleData:public Data {
public:
    DoubleData():Data(){set(0);edited = FALSE;};
    ~DoubleData(){};
    QString getType(){return QString("DOUBLE");};

    void set(double);
    double get(){return dbl;};
    void push(char);
    void del();
    void clear();
private:
    double dbl;
    bool edited;
};

#endif
