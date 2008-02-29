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

#ifndef CALCULATOR_INSTRUCTION_H
#define CALCULATOR_INSTRUCTION_H

#include <qstring.h>
//#include <qstack.h>

#include "data.h"

class QTOPIA_EXPORT Instruction {
public:
    Instruction();
    virtual ~Instruction(){};
    virtual Data *eval(Data *);
    Data *num;
//    QStack<Data> stack;
};

class QTOPIA_EXPORT InstructionDescription {
public:
    InstructionDescription();
    virtual ~InstructionDescription(){};

    virtual Instruction *getInstruction();
    void eval();

    Data *num;
    QString instructionName;
    QString typeOne;
    QString typeTwo;
    QString type;
    int precedence;
    int argCount;
};

#endif
