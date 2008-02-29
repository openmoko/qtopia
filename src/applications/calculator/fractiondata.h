/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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
#ifdef ENABLE_FRACTION

#ifndef CALC_FRACTIONDATA_H
#define CALC_FRACTIONDATA_H

#include "data.h"

class FractionData:public Data {
public:
    FractionData(){clear();};
    ~FractionData(){};
    QString getType(){return "Fraction";};
    void setEdited(bool edit) {
        edited = edit;
    };

    bool push(char,bool);
    bool del();
    void clear();

    void set(qlonglong,qlonglong);
    qlonglong getNumerator(){return numerator;};
    qlonglong getDenominator(){return denominator;};

protected:
    virtual void buildFormattedString();

private:
    qlonglong findGCM(qlonglong,qlonglong);
    qlonglong doFindGCM(qlonglong,qlonglong);
    qlonglong numerator,denominator;
    bool dEdited;
    bool edited;
    QString nString, dString;
};

#endif
#endif //ENABLE_FRACTION
