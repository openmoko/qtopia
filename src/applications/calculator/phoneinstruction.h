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
#ifndef CALC_PHONE_H
#define CALC_PHONE_H

#include "instruction.h"
#include "doubledata.h"

// Factory
class iPhoneDoubleFactory:public Instruction {
public:
    iPhoneDoubleFactory();
    ~iPhoneDoubleFactory(){};
    void eval();
};

// Copy
class iPhoneDoubleCopy:public Instruction {
public:
    iPhoneDoubleCopy();
    ~iPhoneDoubleCopy(){};
    void eval();
};

class iEvaluateLine:public Instruction {
public:
    iEvaluateLine();
    ~iEvaluateLine(){};
    QPixmap *draw();
    void eval(){qWarning("Error - iEvaluateLine should never be evaluated!");};
};

// Mathematical functions
class iPhoneAddDoubleDouble:public Instruction {
public:
    iPhoneAddDoubleDouble();
    ~iPhoneAddDoubleDouble(){};
    void eval();
};

class iPhoneSubtractDoubleDouble:public Instruction {
public:
    iPhoneSubtractDoubleDouble();
    ~iPhoneSubtractDoubleDouble(){};
    void eval();
};

class iPhoneMultiplyDoubleDouble:public Instruction {
public:
    iPhoneMultiplyDoubleDouble();
    ~iPhoneMultiplyDoubleDouble(){};
    void eval();
};

class iPhoneDivideDoubleDouble:public Instruction {
public:
    iPhoneDivideDoubleDouble();
    ~iPhoneDivideDoubleDouble(){};
    void eval();
};
#endif //CALC_PHONE_H
