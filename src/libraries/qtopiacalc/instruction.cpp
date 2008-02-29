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
#include "instruction.h"

Instruction::Instruction(){num=0;}
Instruction::~Instruction(){};
Data *Instruction::eval(Data *d){return d;}

InstructionDescription::InstructionDescription() {
    num = 0;
    precedence = 0;
    instructionName = typeOne = typeTwo = "NULL";
    argCount = 2;
}
InstructionDescription::~InstructionDescription(){};
Instruction *InstructionDescription::getInstruction(){return new Instruction();}
