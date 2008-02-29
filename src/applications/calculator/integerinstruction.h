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
#ifndef CALC_INTEGER_H
#define CALC_INTEGER_H

#include "instruction.h"
#include "integerdata.h"

// Integer instruction base
class BaseIntegerInstruction : public Instruction {
public:
    BaseIntegerInstruction():Instruction(){};
    ~BaseIntegerInstruction(){};

    Data *eval(Data*);
    Data *doEval(IntegerData *i){return i;};
protected:
    IntegerData *integerNum;
};

// Mathematical functions
class IntegerAdd : public BaseIntegerInstruction {
public:
    IntegerAdd():BaseIntegerInstruction(){};
    ~IntegerAdd(){};
    Data *doEval(IntegerData *);
};
class IntegerSub : public BaseIntegerInstruction {
public:
    IntegerSub():BaseIntegerInstruction(){};
    ~IntegerSub(){};
    Data *doEval(IntegerData *);
};
class IntegerMul : public BaseIntegerInstruction {
public:
    IntegerMul():BaseIntegerInstruction(){};
    ~IntegerMul(){};
    Data *doEval(IntegerData *);
};
class IntegerDiv : public BaseIntegerInstruction {
public:
    IntegerDiv():BaseIntegerInstruction(){};
    ~IntegerDiv(){};
    Data *doEval(IntegerData *);
};

#endif
