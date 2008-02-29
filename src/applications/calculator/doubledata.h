/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef CALC_DOUBLEDATA_H
#define CALC_DOUBLEDATA_H

#include <qtopiaglobal.h>

#include "data.h"
#include "engine.h"

// Data type
class DoubleData:public Data {
public:
    DoubleData();
    ~DoubleData(){};
    QString getType(){return QString("Double");}; // No tr
    void setEdited(bool edit) {
        edited = edit;
    };

    void set(double);
    double get();
    bool push(char,bool);
    bool del();
    void clear();
private:
    double dbl;
    bool edited;
};

#endif
