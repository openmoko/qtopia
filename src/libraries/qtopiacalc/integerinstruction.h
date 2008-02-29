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
