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
#ifndef CALC_INTEGER_H
#define CALC_INTEGER_H

#include "instruction.h"
#include "integerdata.h"

// Integer instruction base
class QTOPIA_EXPORT BaseIntegerInstruction : public Instruction {
public:
    BaseIntegerInstruction():Instruction(){};
    ~BaseIntegerInstruction(){};

    Data *eval(Data*);
    Data *doEval(IntegerData *i){return i;};
protected:
    IntegerData *integerNum;
};

// Mathematical functions
class QTOPIA_EXPORT IntegerAdd : public BaseIntegerInstruction {
public:
    IntegerAdd():BaseIntegerInstruction(){};
    ~IntegerAdd(){};
    Data *doEval(IntegerData *);
};
class QTOPIA_EXPORT IntegerSub : public BaseIntegerInstruction {
public:
    IntegerSub():BaseIntegerInstruction(){};
    ~IntegerSub(){};
    Data *doEval(IntegerData *);
};
class QTOPIA_EXPORT IntegerMul : public BaseIntegerInstruction {
public:
    IntegerMul():BaseIntegerInstruction(){};
    ~IntegerMul(){};
    Data *doEval(IntegerData *);
};
class QTOPIA_EXPORT IntegerDiv : public BaseIntegerInstruction {
public:
    IntegerDiv():BaseIntegerInstruction(){};
    ~IntegerDiv(){};
    Data *doEval(IntegerData *);
};

#endif
