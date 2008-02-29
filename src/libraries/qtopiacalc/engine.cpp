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

#include <qtopia/qpeapplication.h>
#include <qtopia/calc/doubleinstruction.h>
#ifdef ENABLE_FRACTION
#include <qtopia/calc/fractioninstruction.h>
#endif
#ifdef ENABLE_INTEGER
#include <qtopia/calc/integerinstruction.h>
#endif

//#define QTEST

// Braces
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

BraceOpen::BraceOpen():InstructionDescription() {
    instructionName = "Open brace"; // No tr
    typeOne = typeTwo = type = "NONE";
    precedence = 50;
};
BraceOpen::~BraceOpen(){};
void Engine::closeBrace () {
    braceCount++;
    acc = evalStack(acc,TRUE);
    updateDisplay();
}
Data *iBraceOpen::eval(Data *d) {
    systemEngine->decBraceCount();
    return d;
}
Instruction *BraceOpen::getInstruction() {
    return new iBraceOpen();
}

Engine::Engine() {
    // Variable initialisation
    state = sStart;
    emptyDataCache = mem = acc = k = 0;
    kDesc = 0;
    lcd = 0;
    memMark = kMark = 0;
    braceCount = decimalPlaces = previousInstructionsPrecedence = 0;
    secondaryReset = FALSE;
    currentType = "NONE";

    list.setAutoDelete(TRUE);

    // Register the common instructions
    // Double
    InstructionDescription *da = new AddDoubleDouble();
    registerInstruction(da);
    da = new MultiplyDoubleDouble();
    registerInstruction(da);
    da = new SubtractDoubleDouble();
    registerInstruction(da);
    da = new DivideDoubleDouble();
    registerInstruction(da);
    da = new DoubleFactory();
    registerInstruction(da);
    da = new DoubleCopy();
    registerInstruction(da);
    da = new DoublePow();
    registerInstruction(da);
    da = new DoubleExp();
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
#ifdef ENABLE_FRACTION
    da = new FractionCopy();
    registerInstruction(da);
    da = new FractionFactory();
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
    da = new ConvertFractionDouble();
    registerInstruction(da);
#endif
#ifdef ENABLE_INTEGER
    da = new ConvertIntDouble();
    registerInstruction(da);
#endif
    // System
    da = new BraceOpen();
    registerInstruction(da);
}
Engine::~Engine() {};

void Engine::registerInstruction(InstructionDescription *d) { 
#ifdef QTEST
qDebug("registerInstruction - %s for %s",d->instructionName.latin1(),d->type.latin1());
#endif
    InstructionDescription *tmp;
    for (uint it = 0; it < list.count(); it++) {
	tmp = list.at(it);
	if (tmp->instructionName == d->instructionName &&
		tmp->typeOne == d->typeOne &&
		tmp->typeTwo == d->typeTwo)
	    return;
    }
    list.append(d);
}
Instruction * Engine::resolveInstruction(QString name) {
    QString type = currentType;
#ifdef QTEST
qDebug("resolveInstruction(%s)",name.latin1());
#endif
    InstructionDescription *id = resolveDescription(name);
    if (!id)
	return 0;
    return resolveInstruction(id);
}
Instruction * Engine::resolveInstruction(InstructionDescription *d) {
    if (!d)
	return 0;
#ifdef QTEST
qDebug("Searching for %s %s %s in %d",
	d->instructionName.latin1(),
	d->typeOne.latin1(),
	d->typeTwo.latin1(),
	list.count());
#endif
    // Create a shortlist of instructions with the same name
    QList<InstructionDescription> shortList;
    InstructionDescription *tmp;
    uint it = 0;
    for (; it < list.count(); it++) {
	tmp = list.at(it);
	if (tmp->instructionName == d->instructionName) 
	    shortList.append(list.at(it));
    }

    // No instructions by that name have been found
    if (!shortList.count()) {
#ifdef QTEST
qDebug("None found by that name");
#endif
	return new Instruction();
    }

    Instruction *ret;

    // Try to match exactly at first
    for (it = 0; it < shortList.count(); it++) {
	tmp = shortList.at(it);
	if (tmp->typeOne == d->typeOne &&
		tmp->typeTwo == d->typeTwo) {
#ifdef QTEST
qDebug("Matched %s %s %s",tmp->instructionName.latin1(),
	tmp->typeOne.latin1(),
	tmp->typeTwo.latin1());
#endif
	    ret = tmp->getInstruction();
	    ret->num = d->num;
	    return ret;
	}
    }
#ifdef QTEST
qDebug("No match found");
#endif
    // Search for conversions that will let what we have work
    // Eventually weighting should be here as well...
    for (it = 0; it < shortList.count(); it++) {
	tmp = shortList.at(it);
	InstructionDescription *tmp2;
	// Search for ops that only require one side to be converted
	for (uint it2 = 0; it < list.count(); it2++) {
	    tmp2 = shortList.at(it2);
	    if ( tmp->typeTwo == currentType &&
		    tmp2->typeOne == currentType &&
		    tmp2->typeTwo == tmp->typeOne ) {
		// Convert
		ret = tmp2->getInstruction();
		ret->num = d->num;
		// eval
		delete ret;
		// crash here...

		// Now return the actual instruction
		ret = tmp->getInstruction();
		ret->num = d->num;
		return ret;
	    } else if ( tmp->typeOne == currentType &&
		    tmp->typeOne == currentType &&
		    tmp2->type == tmp->typeTwo) {
		// Convert
		ret = tmp2->getInstruction();
		ret->num = d->num;
		acc = ret->eval(acc);
		delete ret;

		// Now return the actual instruction
		ret = tmp->getInstruction();
		ret->num = d->num;
		return ret;
	    }
	}
    }
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
	    i = resolveInstruction(id);
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
#ifdef NEW_STYLE_STACK
    if (dStack.isEmpty())
	executeInstructionOnStack("Factory",dStack); // No tr
    if (dStack.isEmpty())
	qDebug("factory didnt work");
    dStack.top()->clear();
#else
    if (acc)
	acc->clear();
    else
	executeInstructionOnStack("Factory",stack); // No tr
#endif
    state = sStart;
    updateDisplay();
}
void Engine::hardReset() {
    stack.clear();
    dStack.clear();
    iStack.clear();
    braceCount = 0;
    secondaryReset = FALSE;
    softReset();
}

// Input and output
InstructionDescription *Engine::resolveDescription(QString name) {
    QString type = currentType;
#ifdef QTEST
qDebug("resolveDescription(%s, %s)",name.latin1(),type.latin1());
#endif
    InstructionDescription *id;
    for (uint i = 0;i < list.count();i++) {
	id = list.at(i);
#ifdef QTEST
qDebug("  - comparing %s for %s",id->instructionName.latin1(),id->type.latin1());
#endif
	if (id->instructionName == name &&
		id->type == type)
	    return id;
    }
    return 0;
}
#ifdef NEW_STYLE_STACK
void Engine::immediateInstruction(QString name) {
#ifdef QTEST
qDebug("immediateInstruction(%s)",name.latin1());
#endif
    if (state == sError || name.isEmpty())
	return;
    previousInstructionsPrecedence = 0;
    secondaryReset = FALSE;
    executeInstructionOnStack(name,stack);
    updateDisplay();
    state = sStart;
}
#endif
#ifdef NEW_STYLE_STACK
void Engine::pushInstruction(QString name) {
    if (state == sError)
	return;
    InstructionDescription *id = resolveDescription(name);
    if (!id)
	return;
    previousInstructionsPrecedence = id->precedence;
    secondaryReset = FALSE;
    if (!id->precedence) {
	executeInstructionOnStack(name,dStack);
	updateDisplay();
	delete id;
	state = sStart;
	return;
    }
    if (!iStack.isEmpty()
	    && state == sStart
	    && id->precedence
	    && previousInstructionsPrecedence) {
	iStack.pop();
    }
    if (!iStack.isEmpty()) {
	InstructionDescription *top = resolveDescription(*(iStack.top()));
	if (id->precedence <= top->precedence) {
	    acc = evalStack(acc,FALSE,top->precedence);
	    updateDisplay();
	}
    }

    Instruction *copy = resolveInstruction("Copy"); // No tr
    if (!copy)
	return;
    id->num = copy->eval(acc);
    delete copy;
    if ( state == sError )
	return;
    stack.push(id);
    state = sStart;
}
#else
void Engine::pushInstruction(InstructionDescription *i) {
    if (state == sError)
	return;
    previousInstructionsPrecedence = i->precedence;
    secondaryReset = FALSE;
    if (!i->precedence) {
	executeInstructionOnStack(i->instructionName,stack);
	updateDisplay();
	delete i;
	state = sStart;
	return;
    }
    if (!stack.isEmpty()
	    && state == sStart
	    && i->precedence
	    && previousInstructionsPrecedence) {
	stack.pop();
    }
    if (!stack.isEmpty()) {
	if (i->precedence <= stack.top()->precedence) {
	    acc = evalStack(acc,FALSE,stack.top()->precedence);
	    updateDisplay();
	}
    }

    InstructionDescription *id = resolveDescription(i->instructionName);
    if (!id)
	return;
    Instruction *copy = resolveInstruction("Copy"); // No tr
    if (!copy)
	return;
    i->num = copy->eval(acc);
    delete copy;
    if ( state == sError )
	return;
    stack.push(i);
    state = sStart;
}
#endif
#ifdef NEW_STYLE_STACK
void Engine::pushChar(char c) {
    if (state == sError)
	return;
    if (dStack.isEmpty())
	executeInstructionOnStack("Factory",dStack); // No tr
    if (dStack.isEmpty()) {
	setError(eNoDataFactory);
	return;
    }
    if (state == sStart) {
	softReset();
	state = sAppend;
    }
    dStack.top()->push(c);
    secondaryReset = FALSE;
    updateDisplay();
}
#else
void Engine::pushChar(char c) {
    if (!checkState())
	return;
    if (state == sStart) {
	softReset();
	state = sAppend;
    }
    acc->push(c);
    secondaryReset = FALSE;
    updateDisplay();
}
#endif
void Engine::delChar() {
    if (!checkState())
	return;
    acc->del();
    updateDisplay();
}
#ifdef NEW_STYLE_STACK
void Engine::updateDisplay() {
    if (state == sError)
	return;
    if (dStack.isEmpty())
	executeInstructionOnStack("Factory",dStack); // No tr
    if (dStack.isEmpty())
	qDebug("still empty, type = %s, state = %d,stack size = %d",currentType.latin1(),state,dStack.count());
    if (lcd) 
	lcd->setText(dStack.top()->getFormattedOutput());
}
#else
void Engine::updateDisplay() {
    if (state == sError)
	return;
    if (!acc)
	executeInstructionOnStack("Factory",stack); // No tr
    if (lcd)
	lcd->setText(acc->getFormattedOutput());
}
#endif
bool Engine::checkState() {
    if (state == sError)
	return FALSE;
    if (!acc) //stack.isEmpty())
	executeInstructionOnStack("Factory",stack); // No tr
    if (!acc) //stack.isEmpty())
	return FALSE;
    if (currentType == "NONE")
	return FALSE;
    return TRUE;
}

// Memory
#ifdef NEW_STYLE_STACK
void Engine::memorySave() {
    if (state == sError)
	return;
    if (dStack.isEmpty())
	return;

    QStack<Data> tmp;
    if (!mem) {
	executeInstructionOnStack("Factory",tmp); // No tr
	mem = tmp.pop();
    }
    tmp.push(dStack.pop());
    tmp.push(mem);
    executeInstructionOnStack("Add",tmp); // No tr

    state = sStart;
}
#else
void Engine::memorySave() {
    if (!checkState())
	return;

    if (!mem) {
	Instruction *factory = resolveInstruction("Factory"); // No tr
	if (!factory) {
	    setError(eNoDataFactory);
	    return;
	}
	mem = factory->eval(new Data());
	delete factory;
	memMark->show();
#ifdef QTEST
qDebug("creating the new memory data at %p, value = %s" ,mem,mem->getFormattedOutput().latin1());
#endif
    }
    Instruction *add = resolveInstruction("Add"); // No tr
    if (!add)
	return;
    add->num = acc; 
    mem = add->eval(mem);
    delete add;
#ifdef QTEST
qDebug("adding the new memory data to the old at %p, value = %s" ,mem,mem->getFormattedOutput().latin1());
#endif
    Instruction *factory = resolveInstruction("Factory"); // No tr
    if (factory) {
	Data *memCmp = factory->eval(acc);
	if (memCmp) {
	    if (mem->getFormattedOutput() == memCmp->getFormattedOutput())
		memoryReset();
	    delete memCmp;
	}
	delete factory;
    }
    state = sStart;
}
#endif
void Engine::memoryRecall() {
    softReset();
    if (!mem)
	return;
#ifdef QTEST
qDebug("contents of memory at memoryRecall() = %s", mem->getFormattedOutput().latin1());
#endif
    Instruction *copy = resolveInstruction("Copy"); // No tr
    if (!copy)
	return;
    if (acc)
	delete acc;
    acc = copy->eval(mem);
    delete copy;
    updateDisplay();
    state = sStart;
}
void Engine::memoryReset() {
    if (mem) {
	delete mem;
	mem = 0;
    }
    memMark->hide();
}

// Uninteresting functions
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
		    "Input not a positive number");
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
	case eNoDataFactory:
	    s = qApp->translate("Engine",
		    "No data factory found");
	case eError:
	default:
	    // let setError(QString) pick it up
	    break;
    }
    setError(s);
}
void Engine::setError(QString s) {
    if (s.isEmpty())
	s = qApp->translate("Engine","Error");
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
#ifdef NEW_STYLE_STACK
void Engine::executeInstructionOnStack(QString name,QStack<Data> ds) {
    InstructionDescription *id = resolveDescription(name);
    if (!id) {
#ifdef QTEST
qDebug("desc not found for %s",name.latin1());
#endif
	return;
    }
    Instruction *i = resolveInstruction(id);
    if (name == "Factory") { // No tr
	if (!i) {
#ifdef QTEST
qDebug("Instruction not found for %s",name.latin1());
#endif
	    setError(eNoDataFactory);
	} else {
#ifdef QTEST
qDebug("before eval stack size is %d",ds.count());
#endif
	    ds.push(i->eval(0));
#ifdef QTEST
qDebug("after eval stack size is %d",ds.count());
#endif
	    delete id;
	    delete i;
	}
	return;
    }
    if (!i) {
#ifdef QTEST
qDebug("Instruction not found for %s",name.latin1());
#endif
	return;
    }
    while (iStack.count() < uint(id->argCount)) // might be better to just bail out
	executeInstructionOnStack("Factory",ds); // No tr
    if (state == sError) {
#ifdef QTEST
qDebug("trouble in recursive call");
#endif
	return;
    }
    Data *ret = i->eval(ds.pop());
    if (!ret) {
#ifdef QTEST
qDebug("error in evaluating instruction");
#endif
	return;
    }
    if (state == sError) {
#ifdef QTEST
qDebug("Error in evaluating %s",name.latin1());
#endif
	delete ret;
	return;
    }
    ds.push(ret);
    delete i;
    delete id;
#ifdef QTEST
qDebug("end");
#endif
}
#else
// this function is convoluted because acc is not (yet) the top of the stack
void Engine::executeInstructionOnStack(QString name,QStack<InstructionDescription> ds) {
    Instruction *i = resolveInstruction(name);
    if (name == "Factory") { // No tr
	if (!i) {
	    setError(eNoDataFactory);
	} else {
	    if (acc) {
		InstructionDescription *id = new InstructionDescription();
		id->num = acc;
		stack.push(id);
	    }
	    acc = i->eval(new Data());
	    delete i;
	}
	return;
    }

    if (!acc) // i->argCount)
	executeInstructionOnStack("Factory",ds); // No tr
    if (state == sError)
	return;

    acc = i->eval(acc);
    delete i;
}
#endif
#ifdef NEW_STYLE_STACK
void Engine::setAccType(QString type) {
    if (currentType == type)
	return;
    currentType = type;
    if (dStack.isEmpty()) {
	executeInstructionOnStack("Factory",dStack); // No tr
	state = sStart;
    } else {
	executeInstructionOnStack("Convert",dStack); // No tr
    }
    updateDisplay();
}
#else
void Engine::setAccType(QString type) {
    if (currentType == type)
	return;
    currentType = type;
    if (!acc) {
	executeInstructionOnStack("Factory",stack); // No tr
	state = sStart;
    } else {
	executeInstructionOnStack("Convert",stack); // No tr
    }
    updateDisplay();
}
#endif
QString Engine::getDisplay() {
    return acc->getFormattedOutput();
}
void Engine::openBrace () {
    stack.push(new BraceOpen());
    state = sStart;
}

