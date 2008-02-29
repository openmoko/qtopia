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
#ifndef CALC_DOUBLEDATA_H
#define CALC_DOUBLEDATA_H

#include <qtopia/qpeglobal.h>
#include "data.h"
#include "engine.h"

// Data type
class QTOPIA_EXPORT DoubleData:public Data {
public:
    DoubleData();
    ~DoubleData(){};
    QString getType(){return QString("Double");}; // No tr

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
