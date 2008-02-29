/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
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
#include <qtopia/global.h>
#ifdef ENABLE_FRACTION
#include <qtopia/calc/fractioninstruction.h>
#endif
#ifdef ENABLE_INTEGER
#include <qtopia/calc/integerinstruction.h>
#endif

// Braces
class iBraceOpen:public Instruction {
public:
    iBraceOpen():Instruction() {
	name = "Open brace"; // No tr
	precedence = 0;
	displayString = "("; // No tr
	argCount = 0;
    };
    ~iBraceOpen(){};
    void eval(){ systemEngine->incBraceCount(); };
};
class iBraceOpenImpl:public Instruction {
public:
    iBraceOpenImpl():Instruction() {
	name = "Open brace impl"; // No tr
	precedence = 1;
	displayString = "("; // No tr
	argCount = 0;
    };
    ~iBraceOpenImpl(){};
    void eval(){};
};
void Engine::openBrace () {
    if (state == sError)
	return;
#ifdef QTOPIA_PHONE
    if (!Global::mousePreferred())
        // prevents crash when x-y*(...) is entered => calc is not entirely correct
        if (state == sAppend)
	    return;
#endif
    pushInstruction("Open brace"); // No tr
    if (state != sError)
	changeState(sStart);
}
void Engine::closeBrace () {
    if (braceCount) {

	if (state == sStart) // this might be wrong here...
	    executeInstructionOnStack("Factory"); // No tr
	doEvalStack(0,TRUE);
	//braceCount--;
    }
}

// Engine class
Engine::Engine():QObject() {
    // Variable initialisation
    state = sAppend;
    changeResetState(drNone);
    mem = recoveredDStack = 0;
    kDesc = 0;
    lcd = 0;
    memMark = kMark = 0;
    braceCount = previousInstructionsPrecedence = 0;
    currentType = wantedType = "NONE"; // No tr

    // Register the common instructions

    // System instructions - null, open/close braces
    Instruction *da = new Instruction();
    registerInstruction(da);
    da = new iBraceOpen();
    registerInstruction(da);
    da = new iBraceOpenImpl();
    registerInstruction(da);

    if (Global::mousePreferred()) {
        // Factory
        da = new iDoubleFactory();
        registerInstruction(da);
        // Normal instructions with full precedence
        da = new iAddDoubleDouble();
        registerInstruction(da);
        da = new iMultiplyDoubleDouble();
        registerInstruction(da);
        da = new iSubtractDoubleDouble();
        registerInstruction(da);
        da = new iDivideDoubleDouble();
        registerInstruction(da);
        da = new iDoubleCopy();
        registerInstruction(da);
        da = new iDoubleNegate();
        registerInstruction(da);
#ifdef ENABLE_FRACTION
        da = new iNegateFractionFraction();
        registerInstruction(da);
#endif
#ifndef QTOPIA_PHONE
        da = new iDoublePow();
        registerInstruction(da);
        da = new iDoubleExp();
        registerInstruction(da);
        da = new iDoubleSinDeg();//Sin
        registerInstruction(da);
        da = new iDoubleSinRad();
        registerInstruction(da);
        da = new iDoubleSinGra();
        registerInstruction(da);
        da = new iDoubleCosDeg();//Cos
        registerInstruction(da);
        da = new iDoubleCosRad();
        registerInstruction(da);
        da = new iDoubleCosGra(); 
        registerInstruction(da);
        da = new iDoubleTanDeg();//Tan
        registerInstruction(da);
        da = new iDoubleTanRad();
        registerInstruction(da);
        da = new iDoubleTanGra();
        registerInstruction(da);
        da = new iDoubleASinDeg();//ASin
        registerInstruction(da);
        da = new iDoubleASinRad();
        registerInstruction(da);
        da = new iDoubleASinGra();
        registerInstruction(da);
        da = new iDoubleACosRad();//ACos
        registerInstruction(da);
        da = new iDoubleACosDeg();
        registerInstruction(da);
        da = new iDoubleACosGra();
        registerInstruction(da);
        da = new iDoubleATanDeg();//ATan
        registerInstruction(da);
        da = new iDoubleATanRad();
        registerInstruction(da);
        da = new iDoubleATanGra();
        registerInstruction(da);
        da = new iDoubleLog();
        registerInstruction(da);
        da = new iDoubleLn();
        registerInstruction(da);
        da = new iDoubleOneOverX();
        registerInstruction(da);
        da = new iDoubleFactorial();
        registerInstruction(da);
        da = new iDoubleSquareRoot();
        registerInstruction(da);
        da = new iDoubleCubeRoot();
        registerInstruction(da);
        da = new iDoubleXRootY();
        registerInstruction(da);
        da = new iDoubleSquare();
        registerInstruction(da);
#ifdef ENABLE_FRACTION
        da = new iFractionCopy();
        registerInstruction(da);
        da = new iFractionFactory();
        registerInstruction(da);
        da = new iConvertDoubleFraction();
        registerInstruction(da);
        da = new iAddFractionFraction();
        registerInstruction(da);
        da = new iSubtractFractionFraction();
        registerInstruction(da);
        da = new iMultiplyFractionFraction();
        registerInstruction(da);
        da = new iDivideFractionFraction();
        registerInstruction(da);
        da = new iConvertFractionDouble();
        registerInstruction(da);
#endif
#ifdef ENABLE_INTEGER
        da = new iConvertIntDouble();
        registerInstruction(da);
#endif
#endif
    }
    list.setAutoDelete(TRUE);
}
Engine::~Engine() {
    iStack.setAutoDelete(TRUE);
    dStack.setAutoDelete(TRUE);
    iStack.clear();
    dStack.clear();
    delete mem;
};

void Engine::registerInstruction(Instruction *i) { 
    Instruction *it;
    for (uint c = 0; c < list.count(); c++) {
	it = list.at(c);
	if (it->name == i->name 
		&& it->type == i->type
		&& it->retType == i->retType)
	    return;
    }
    list.append(i);
}
Instruction * Engine::resolve(QString name) {
    // Create a shortlist of instructions with the same name
    QList<Instruction> shortList;
    Instruction *it = 0;
    uint c = 0;
    for (; c < list.count(); c++) {
	it = list.at(c);
	if (it->name == name) 
	    shortList.append(list.at(c));
    }

    // No instructions by that name have been found
    if (!shortList.count()) {
	return resolve("NULL"); // No tr
    }

	// Should reuse "Factory" with a parameter?
    if (name == "Convert") { // No tr
	// Exact match
	for (c = 0; c < shortList.count(); c++) {
	    it = shortList.at(c);
	    if (it->type == currentType && it->retType == wantedType)
		return it;
	}
    } else {
        if (wantedType != currentType)
            currentType = wantedType;
        
	// Exact match
	for (c = 0; c < shortList.count(); c++) {
	    it = shortList.at(c);
	    if (it->type == currentType && it->retType == currentType)
		return it;
	}
	// Dont match return type to currentType
	for (c = 0; c < shortList.count(); c++) {
	    it = shortList.at(c);
	    if (it->type == currentType )
		return it;
	}
	// Dont match type to currentType
	for (c = 0; c < shortList.count(); c++) {
	    it = shortList.at(c);
	    return it;
	}
    }

    // Fail
    return new Instruction();
}

void Engine::evaluate() {
    if (!Global::mousePreferred())
        // this could go in doEvalStack but its more efficient here
        if (!iStack.isEmpty() 
		&& ((*iStack.top() == "EvaluateLine") || braceCount > 0)) // No tr
	    return;

    if (state == sStart)
	executeInstructionOnStack("Factory"); // No tr
    doEvalStack();
    if (Global::mousePreferred())
        braceCount = 0;
}

void Engine::doEvalStack(int p,bool inbrace) {
    if (state == sError || iStack.isEmpty())
	return;
    evalStack(p,inbrace);
    if (!Global::mousePreferred()) {
        if (braceCount == 0)
    	    iStack.push(new QString("EvaluateLine")); // No tr
        if (braceCount > 0 && inbrace)
	    --braceCount;
    }
    if (state != sError)
	changeState(sAppend);
    emit(stackChanged());
}

void Engine::evalStack(int p,bool inbrace) {
    if (state != sError) {
#ifdef QTOPIA_PHONE
	QStack<QString> tmpIStack;
#endif
	// could be more efficient and only resolve i once
	Instruction *i;
	while (!iStack.isEmpty ()
		&& state != sError
#ifdef QTOPIA_PHONE
		&& *iStack.top() != "EvaluateLine" // No tr
#endif
		&& (p <= resolve(*iStack.top())->precedence)) {
	    // Pop the next instruction
	    QString *iString = iStack.pop();
	    i = resolve(*iString);
            if (Global::mousePreferred()) 
                delete iString;
#ifdef QTOPIA_PHONE
            else
	        tmpIStack.push(iString);
#endif

	    // Stop at the open brace
	    if (i->name == "Open brace impl" && inbrace) { // No tr
		i->eval();
                if (Global::mousePreferred()) 
                    return;
                else {
#ifdef QTOPIA_PHONE
		    //--braceCount;
		    delete tmpIStack.pop(); // delete open brace instraction
#endif
		    return;
                }
	    }

	    // Compare precedence with the next instruction on the stack
	    if (!iStack.isEmpty ()) {
		// Recurse into next instruction if necessary
		int topPrec = resolve(*iStack.top())->precedence;
		if ((p && p <= topPrec) || (!p && i->precedence <= topPrec)) {
		    QStack<Data> holdingStack;
		    for (int c = 1;c < i->argCount;c++)
			holdingStack.push(dStack.pop());
		    evalStack(p,inbrace);
		    for (int c = 1;c < i->argCount;c++)
			dStack.push(holdingStack.pop());
		}
	    }

	    // Evaluate 
	    i->eval();

#ifdef QTOPIA_PHONE
            if (!Global::mousePreferred()) {
                if (!braceCount){
                    Data *top = dStack.pop();
                    for (int c = 0;c < i->argCount;c++)
                        dStack.push(tmpDStack.pop());
                    dStack.push(top);
                } else {
                    for (int c = 0;c < i->argCount;c++)
                        delete tmpDStack.pop();
                    delete tmpIStack.pop();
                }
            }
#endif
	}
#ifdef QTOPIA_PHONE
        if (!Global::mousePreferred())
    	    while (!tmpIStack.isEmpty())
	        iStack.push(tmpIStack.pop());
#endif
    }
}

// Reset
void Engine::dualReset() {
    if (drs == drHard)
	hardReset();
    else
	softReset();
}
void Engine::softReset() {
    if (state == sStart && previousInstructionsPrecedence) {
        if (!iStack.isEmpty())
            delete iStack.pop();
    }

    if (!dStack.isEmpty())
	dStack.top()->clear();
    state = sAppend;

    if (dStack.count() == 1 && !iStack.count())
	changeResetState(drNone);
    else
	changeResetState(drHard);
    emit(stackChanged());
}
void Engine::hardReset() {
    dStack.clear();
    iStack.clear();
    braceCount = 0;
    if (recoveredDStack) {
        delete recoveredDStack;
        recoveredDStack = 0;
    }
    softReset();
    changeResetState(drNone);
}

// Input and output
void Engine::pushInstruction(QString name) {
    if (!checkState())
	return;

    Instruction *i = resolve(name);
    previousInstructionsPrecedence = i->precedence;

    // Immediate instructions
    if (!i->precedence) {
	executeInstructionOnStack(i);
	emit(stackChanged());
	return;
    }

    // Overwrite last instruction
    if (!iStack.isEmpty()
	    && state == sStart
	    && i->precedence
	    && previousInstructionsPrecedence) {
	if ("Open brace impl" == *(iStack.top())) { // No tr
	    executeInstructionOnStack("Factory"); // No tr
	} else {
	    delete iStack.pop();
	}
    }

    // Evaluate previous high-precedence instructions
    if (!iStack.isEmpty()) {
	Instruction *top = resolve(*(iStack.top()));
	if (i->precedence <= top->precedence)
	    doEvalStack(top->precedence);
    }

    // Push instruction onto stack
    if (state != sError) {
	iStack.push(new QString(name));
	changeState(sStart);
    }
    emit(stackChanged());
}

void Engine::push(char c) {
    if (!checkState())
	return;
    if (state == sStart) {
	executeInstructionOnStack("Copy"); // No tr
	if (!checkState()) {
	    setError("Failed to copy some data"); // No tr
	    return;
	}
	dStack.top()->clear();
	if (!dStack.top()->push(c, FALSE)) {
	    delete dStack.pop();
	    return;
	}
	dStack.top()->clear();
	changeState(sAppend);
    } else {
        
#ifdef QTOPIA_PHONE
	if (!Global::mousePreferred() && !iStack.isEmpty() && *iStack.top() == "EvaluateLine") // No tr
	    hardReset();
	else
#endif
	changeResetState(drSoft);
	if (!dStack.top()->push(c, FALSE)) {
            return;
        }
    }

    dStack.top()->push(c);
    emit(stackChanged());
}
void Engine::push(QString s) {
    if (state == sAppend && dStack.count() >= 1)
            dStack.top()->clear();
    for (uint i=0;i < s.length();i++)
	push(s[(int)i].latin1());
}
void Engine::delChar() {
    if (!checkState())
	return;
    if (state == sStart) 
    {
	if (iStack.isEmpty())
	    return;
	if (*iStack.top() != "Open brace impl") // No tr
	    changeState(sAppend);
	delete iStack.pop();
    } 
    else if (state == sAppend) 
    {
	if(dStack.top()->del()) 
        {
	    if (dStack.count() == 1) {
		hardReset();
	    } else if (dStack.count() > 1) {
		delete dStack.pop();
                if (iStack.count() >= 1) {
		    if (*iStack.top() == "EvaluateLine" || 
                            *iStack.top() != "Open brace impl") {
		        changeState(sAppend);
                    } else {
                        changeState(sStart);
                    }
		    delete iStack.pop();
		} else {
		    changeState(sStart);
		}
	    }
	}
    }
    emit(stackChanged());
}
bool Engine::checkState() {
    if (state == sError)
	return FALSE;
    if (dStack.isEmpty())
	executeInstructionOnStack("Factory"); // No tr
    if (dStack.isEmpty()) {
	setError(eNoDataFactory);
	return FALSE;
    }
    if (currentType == "NULL")
	return FALSE;
    if (state == sError)
	return FALSE;
    return TRUE;
}
Data *Engine::getData() {
#ifdef QTOPIA_PHONE
    if (!Global::mousePreferred()) {
        executeInstructionOnStack("Copy"); // No tr
        tmpDStack.push(dStack.pop());
    }
#endif
    return dStack.pop();
}
void Engine::putData(Data *d) {
    dStack.push(d);
}

// Memory
void Engine::memorySave() {
    if (!checkState())
	return;

    //save previous data on stack
    executeInstructionOnStack("Copy"); // No tr
    if (mem) {
	dStack.push(mem);
	executeInstructionOnStack("Add"); // No tr
    }
    if (state != sError)
	mem = dStack.pop();

    executeInstructionOnStack("Factory"); // No tr
    if (mem->getFormattedOutput() != dStack.top()->getFormattedOutput()) {
	memMark->show();
    } else {
	delete mem;
	mem = 0;
	memMark->hide();
    }
    delete dStack.pop();

    qApp->processEvents();
}
void Engine::memoryRecall() {
    if (!checkState())
	return;

    if (state == sAppend)
	delete dStack.pop();

    if ( mem ) {
        dStack.push(mem);
        //memory may have different data type
        if (currentType != mem->getType()) {
            wantedType = currentType;
            currentType = mem->getType();
            executeInstructionOnStack("Convert");
            currentType = wantedType;
        }

        executeInstructionOnStack("Copy"); // No tr
        if (!checkState())
	    return;
        mem = dStack.pop();
    }
    else 
        executeInstructionOnStack("Factory"); // No tr

    changeState(sAppend);
    emit(stackChanged());
}
void Engine::memoryReset() {
    if (mem) {
	delete mem;
	mem = 0;
    }
    memMark->hide();
}

void Engine::setError(Error e, bool resetStack){
    QString s;
    switch (e) {
	case eNotANumber:
	    s = qApp->translate("Engine",
		    "Not a number");
	    break;
	case eNegInf:
	    s = qApp->translate("Engine",
		    "Negative infinity");
	    break;
	case eInf:
	    s = qApp->translate("Engine",
		    "Infinity");
	    break;
	case eNonPositive:
	    s = qApp->translate("Engine",
		    "Input is not a positive number");
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
	case eSurpassLimits:
	    s = qApp->translate("Engine",
		    "Calc surpasses limit");
	case eError:
	default:
	    // let setError(QString) pick it up
	    break;
    }
    setError(s, resetStack);
}
void Engine::setError(QString s, bool resetStack) {
    if (s.isEmpty())
	s = qApp->translate("Engine","Error");
    if (!resetStack) {
        recoveredDStack = getData();
        putData(recoveredDStack);
    }
    
    changeState(sError);
    errorString = s;
    emit(stackChanged());
}
void Engine::setDisplay(MyLcdDisplay *l) {
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

void Engine::executeInstructionOnStack(QString name) {
    executeInstructionOnStack(resolve(name));
}
void Engine::executeInstructionOnStack(Instruction *i) {
    if (i->name == "Factory") { // No tr
	i->eval();
	return;
    }

    while (dStack.count() < uint(i->argCount))
	executeInstructionOnStack("Factory"); // No tr

    if (state != sError)
	i->eval();
}
void Engine::setAccType(QString type) {
    if (currentType != type || recoveredDStack) {
	if (dStack.isEmpty()) {
	    currentType = type;
	    executeInstructionOnStack("Factory"); // No tr
	    state = sAppend;
	} else {
            if (recoveredDStack) {
                //don't call dualReset() or recoveredDStack will be deleted
                dStack.clear();
                iStack.clear();
                braceCount = 0;
                softReset();
                changeResetState(drNone);
                putData(recoveredDStack);
            }
	    wantedType = type;

            if (currentType == "NONE") // workaround for bug #3356
                currentType = dStack.top()->getType();
           
            if (currentType != wantedType) {
                if (!recoveredDStack) 
                    executeInstructionOnStack("Convert"); // No tr
                else  // don't convert recoveredStack
                    recoveredDStack = 0;
            }
	}
	// Doublecheck the type in case a factory or convert doesnt go as planned.
	currentType = dStack.top()->getType();
	emit(stackChanged());
    }
    changeResetState(drs);
}
QString Engine::getDisplay() {
    return dStack.top()->getFormattedOutput();
}

void Engine::changeState(EngineState s) {
    switch (s) {
	case sStart:
	case sAppend:
	    changeResetState(drSoft);
	    break;
	case sError:
	    changeResetState(drHard);
	    break;
    }
    state = s;
}
void Engine::changeResetState(ResetState s) {
    emit(dualResetStateChanged(s));
    drs = s;
}
void Engine::incBraceCount() {
    if (systemEngine->state == sAppend)
        systemEngine->iStack.push(new QString("Multiply")); // No tr
    systemEngine->iStack.push(new QString("Open brace impl")); // No tr
    braceCount++;
}

#ifdef QTOPIA_PHONE
bool Engine::error(){
    if ( state == sError )
        return TRUE;
    return FALSE;
}

int Engine::numOps(){
    return dStack.count();
}

#endif
