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
#include "engine.h"
#include "doubleinstruction.h"
#include "fractioninstruction.h"

#include <qtopia/qpeapplication.h>

//#define QTEST

// Static member variables
State Engine::state = sStart;
Data *Engine::mem = 0;
Data *Engine::acc = 0;
Data *Engine::k = 0;
InstructionDescription *Engine::kDesc = 0;
int Engine::braceCount = 0;
int Engine::decimalPlaces = 0;
int Engine::myCounter = 0;
bool Engine::secondaryReset = FALSE;
int Engine::previousInstructionsPrecedence = 0;
QStack<InstructionDescription> Engine::stack;
QList<InstructionDescription> Engine::list;
QLineEdit *Engine::lcd = 0;
QLabel *Engine::memMark = 0;
QLabel *Engine::kMark = 0;

class iBraceOpen:public Instruction {
public:
    iBraceOpen():Instruction(){};
    ~iBraceOpen(){};
    Data *eval(Data *);
};
class BraceOpen:public InstructionDescription {
public:
    BraceOpen();
    ~BraceOpen();
    Instruction *getInstruction();
};

Engine::Engine() {
    // Double
    if (!myCounter) {
	InstructionDescription *da = new AddDoubleDouble();
	registerInstruction(da);
	da = new MultiplyDoubleDouble();
	registerInstruction(da);
	da = new SubtractDoubleDouble();
	registerInstruction(da);
	da = new DivideDoubleDouble();
	registerInstruction(da);
	da = new ConvertDoubleDouble();
	registerInstruction(da);
	da = new ConvertFractionDouble();
	registerInstruction(da);
	da = new ConvertIntDouble();
	registerInstruction(da);
	da = new DoublePow();
	registerInstruction(da);
	da = new DoubleSin();
	registerInstruction(da);
	da = new DoubleCos();
	registerInstruction(da);
	da = new DoubleTan();
	registerInstruction(da);
	da = new DoubleASin();
	registerInstruction(da);
	da = new DoubleACos();
	registerInstruction(da);
	da = new DoubleATan();
	registerInstruction(da);
	da = new DoubleLog();
	registerInstruction(da);
	da = new DoubleLn();
	registerInstruction(da);
	da = new DoubleOneOverX();
	registerInstruction(da);
	da = new DoubleFactorial();
	registerInstruction(da);
	da = new DoubleSquareRoot();
	registerInstruction(da);
	da = new DoubleCubeRoot();
	registerInstruction(da);
	da = new DoubleXRootY();
	registerInstruction(da);
	da = new DoubleSquare();
	registerInstruction(da);
	da = new DoubleNegate();
	registerInstruction(da);
	// Fraction
	da = new ConvertFractionFraction();
	registerInstruction(da);
	da = new ConvertDoubleFraction();
	registerInstruction(da);
	da = new AddFractionFraction();
	registerInstruction(da);
	da = new SubtractFractionFraction();
	registerInstruction(da);
	da = new MultiplyFractionFraction();
	registerInstruction(da);
	da = new DivideFractionFraction();
	registerInstruction(da);
	// System
	da = new BraceOpen();
	registerInstruction(da);
	list.setAutoDelete(TRUE);
	myCounter++;
    }
}
Engine::~Engine() { }

void Engine::registerInstruction(InstructionDescription *d) { 
    list.append(d);

    /* Check if its already there?
    InstructionDescription *tmp;
    uint it = 0;
    bool found = FALSE;
    for (; it < list.count(); it++) {
	tmp = list.at(it);
	if (tmp->instructionName == d->instructionName &&
		tmp->typeOne == d->typeOne &&
		tmp->typeTwo == d->typeTwo)
	    found = TRUE;
    }
    if (!found) {
qDebug("appending %p",d);
	list.append(d);
    }
    */
}
Instruction * Engine::resolveInstruction(InstructionDescription &d) {
#ifdef QTEST
qDebug("Searching for %s %s %s in %d",
	d.instructionName.latin1(),
	d.typeOne.latin1(),
	d.typeTwo.latin1(),
	list.count());
#endif
    // Create a shortlist of instructions with the same name
    QList<InstructionDescription> shortList;
    InstructionDescription *tmp;
    uint it = 0;
    for (; it < list.count(); it++) {
	tmp = list.at(it);
	if (tmp->instructionName == d.instructionName) 
	    shortList.append(list.at(it));
    }

    // No instructions by that name have been found
    if (!shortList.count()) {
#ifdef QTEST
qDebug("None found by that name");
#endif
	return new Instruction();
    }

    // Try to match exactly at first
    for (it = 0; it < shortList.count(); it++) {
	tmp = shortList.at(it);
	if (tmp->typeOne == d.typeOne &&
		tmp->typeTwo == d.typeTwo) {
#ifdef QTEST
qDebug("Matched %s %s %s",tmp->instructionName.latin1(),
	tmp->typeOne.latin1(),
	tmp->typeTwo.latin1());
#endif
	    Instruction *ret = tmp->getInstruction();
	    ret->num = d.num;
	    return ret;
	}
    }
#ifdef QTEST
qDebug("No match found");
#endif
    // Search for conversions that will let what we have work
    // Eventually weighting should be here as well...
    return new Instruction();
}
// Stack
void Engine::evaluate() {
    acc = evalStack(acc,FALSE);
    updateDisplay();
    if ( state != sError )
	state = sStart;
    braceCount = 0;
}
Data *Engine::evalStack(Data *intermediate,bool inbrace,int p) {
    if (state != sError) {
	InstructionDescription *id;
	Instruction *i;
	while (!stack.isEmpty ()
		&& (braceCount || !inbrace)
		&& state != sError
		&& (p <= stack.top()->precedence || p == 0) ) {
	    // Pop the next instruction
	    id = stack.pop();

	    // Compare precedence with the next instruction on the stack
	    if (!stack.isEmpty ()) {
		// Recurse into next instruction if necessary
		if (p) {
		    if (p <= stack.top()->precedence)
			id->num = evalStack(id->num,inbrace,p);
		} else if (id->precedence <= stack.top()->precedence) {
		    id->num = evalStack(id->num,inbrace,id->precedence);
		}
	    }
	    // Evaluate 
	    i = resolveInstruction(*id);
	    i->num = id->num;
	    Data *tmp = i->eval(intermediate);

	    // Clean up
	    if (intermediate != tmp)
		delete intermediate;
	    intermediate = tmp;
	    delete i;
	    delete id;
	}
    }
    return intermediate;
}

// Miscellaneous functions
void Engine::dualReset() {
    if (secondaryReset) {
	hardReset();
    } else {
	softReset();
	secondaryReset = TRUE;
    }

}
void Engine::softReset() {
    decimalPlaces = -1;
    acc->clear();
    state = sStart;
    updateDisplay();
}
void Engine::hardReset() {
    stack.clear();
    braceCount = 0;
    secondaryReset = FALSE;
    softReset();
}

// Input and output
void Engine::pushInstruction(InstructionDescription *i) {
    if (state == sError)
	return;
    if (stack.count()
	    && state == sStart
	    && i->precedence
	    && previousInstructionsPrecedence) {
	stack.pop();
    }
    previousInstructionsPrecedence = i->precedence;

    secondaryReset = FALSE;
    InstructionDescription desc;
    desc.typeOne = desc.typeTwo = acc->getType();
    if (!i->precedence) {
	desc.instructionName = i->instructionName;
	desc.precedence = 0;
	desc.num = acc;
	delete i;
    } else {
	// TODO: this is the rest of the evalStack hack
	if (!stack.isEmpty())
	    if (i->precedence <= stack.top()->precedence) {
		acc = evalStack(acc,FALSE,stack.top()->precedence);
	    }
	desc.instructionName = QString("CONVERT");
	i->num = acc;
	stack.push(i);
    }
    Instruction *in = resolveInstruction(desc);
    acc = in->eval(acc);
    delete in;
    if ( state != sError )
	state = sStart;
    // this could be causing some flicker, convert
    // the top of the stack instead of acc
    updateDisplay();
}
void Engine::pushChar(char c) {
    if (state == sError)
	return;
    if (state == sAppend) {
	acc->push(c);
    } else if (state == sStart) {
	softReset();
	state = sAppend;
	acc->push(c);
    }
    secondaryReset = FALSE;
    updateDisplay();
}
void Engine::delChar() {
    if (state == sError)
	return;
    acc->del();
    updateDisplay();
}
void Engine::updateDisplay() {
    if (state == sError)
	return;
    if (lcd) 
	lcd->setText(acc->getFormattedOutput());
    else
	setError(qApp->translate("Engine",
		    "Error in main programs: LCD has not been set"));
}

// Memory
void Engine::memorySave() {
    if (state == sError)
	return;
    InstructionDescription f;
    f.typeOne = acc->getType();
    if (!mem) {
	f.instructionName = "CONVERT";
	f.typeTwo = f.typeOne;
	memMark->show();
    } else {
	f.instructionName = "ADD";
	f.num = mem;
	f.typeTwo = mem->getType();
    }
    Data *tmp = convert(acc,acc);
    Instruction *i = resolveInstruction(f);
    tmp = i->eval(tmp);
    delete i;
    if (mem)
	delete mem;
    if (acc == tmp)
	acc = convert(acc,acc);
    mem = tmp;
    state = sStart;
}
void Engine::memoryRecall() {
    if (mem) {
	Data *tmp = convert(mem,acc);
	setAcc(tmp);
    } else
	softReset();
}
void Engine::memoryReset() {
    if (mem) {
	delete mem;
	mem = 0;
	memMark->hide();
    }
}
void Engine::setError(Error e){
    QString s;
    switch (e) {
	case eNotANumber:
	    s = qApp->translate("Engine",
		    "Not a number");
	    break;
	case eInf:
	    s = qApp->translate("Engine",
		    "Infinity");
	    break;
	case eNonPositive:
	    s = qApp->translate("Engine",
		    "Not a positive number");
	    break;
	case eNonInteger:
	    s = qApp->translate("Engine",
		    "Not an integer");
	    break;
	case eOutOfRange:
	    s = qApp->translate("Engine",
		    "Out of range");
	    break;
	case eDivZero:
	    s = qApp->translate("Engine",
		    "Divide by zero error");
	    break;
	case eNotSolvable:
	    s = qApp->translate("Engine",
		    "Not solvable");
	    break;
	case eError:
	default:
	    s = qApp->translate("Engine",
		    "Error");
	    break;
    }
    setError(s);
}
void Engine::setError(QString s) {
    state = sError;
    lcd->setText(s);
}
void Engine::setDisplay(QLineEdit *l) {
    lcd = l;
    memMark = new QLabel( "m", lcd );
    memMark->setBackgroundMode( QWidget::PaletteBase );
    memMark->setFont( QFont( "helvetica", 12, QFont::Bold, TRUE ) );
    memMark->resize( 12, 12 );
    memMark->move( 4, 2 );
    memMark->hide();

    kMark = new QLabel( "k", lcd );
    kMark->setBackgroundMode( QWidget::PaletteBase );
    kMark->setFont( QFont( "helvetica", 12, QFont::Bold, TRUE ) );
    kMark->resize( 12, 12 );
    kMark->move( 4, 14 );
    kMark->hide();
}
void Engine::setAcc(Data *d) {
    if (acc && d != acc)
	delete acc;
    acc = d;
    state = sStart;
    updateDisplay();
}
void Engine::setAccType(Data *d) {
    if (!acc) {
	acc = d;
	acc->clear();
	state = sStart;
    } else {
	Data *tmp = convert(acc,d);
	if (acc != tmp)
	    delete acc;
	acc = tmp;
	if (acc != d)
	    delete d;
	if (stack.count())
	    stack.top()->typeTwo = acc->getType();
    }
    updateDisplay();
}
Data * Engine::getAcc() {
    return acc;
}
Data *Engine::convert(Data *from, Data *to) {
    InstructionDescription f;
    f.instructionName = "CONVERT";
    f.typeOne = from->getType();
    f.typeTwo = to->getType();
    Data *tmp = resolveInstruction(f)->eval(from);
    return tmp;
}

// Braces
BraceOpen::BraceOpen():InstructionDescription() {
    instructionName = "BRACE_OPEN";
    typeOne = typeTwo = "ALL";
    precedence = 50;
};
BraceOpen::~BraceOpen(){};
void Engine::closeBrace () {
    braceCount++;
    acc = evalStack(acc,TRUE);
    updateDisplay();
}
void Engine::openBrace () {
    stack.push(new BraceOpen());
    state = sStart;
}
Data *iBraceOpen::eval(Data *d) {
    Engine::braceCount--;
    return d;
}
Instruction *BraceOpen::getInstruction() {
    return new iBraceOpen();
}
