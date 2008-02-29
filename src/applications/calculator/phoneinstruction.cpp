/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "phoneinstruction.h"
#include "engine.h"

#include <QBitmap>

// Factory
iPhoneDoubleFactory::iPhoneDoubleFactory():Instruction() {
    retType = type = "Double"; // No tr
    name = "Factory"; // No tr
}
void iPhoneDoubleFactory::eval() {
    DoubleData *newData = new DoubleData();
    newData->clear();
    systemEngine->dStack.push(newData);
}

// Copy
iPhoneDoubleCopy::iPhoneDoubleCopy():Instruction() {
    name = "Copy"; // No tr
    retType = type = "Double"; // No tr
    argCount = 1;
}
void iPhoneDoubleCopy::eval() {
    DoubleData *src = (DoubleData *)systemEngine->dStack.pop();
    DoubleData *tgt = new DoubleData();
    tgt->clear();
    tgt->set(((DoubleData *)src)->get());
    systemEngine->dStack.push(src);
    systemEngine->dStack.push(tgt);
};

iEvaluateLine::iEvaluateLine():Instruction() {
    name = "EvaluateLine"; // No tr
    retType = type = "Double"; // No tr
    argCount = 2;
    precedence = 1;
}
QPixmap *iEvaluateLine::draw() {
    if (!cache) {
        cache = new QBitmap(50,3);
        cache->clear();
        QPainter p(cache);
        p.setPen(Qt::color1);
        p.drawLine(0,1,50,1);
    }
    return cache;
}

// Basic functions without precedence
iPhoneAddDoubleDouble::iPhoneAddDoubleDouble():Instruction() {
    name = "Add"; // No tr
    precedence = 10;
    displayString = "+";
    argCount = 2;
}
void iPhoneAddDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    DoubleData *result = new DoubleData();
    result->set(num->get() + acc->get());
    systemEngine->putData(result);
    delete num;
    delete acc;
}

iPhoneSubtractDoubleDouble::iPhoneSubtractDoubleDouble():Instruction() {
    name = "Subtract"; // No tr
    precedence = 10;
    displayString = "-";
    argCount = 2;
}
void iPhoneSubtractDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    DoubleData *result = new DoubleData();
    result->set(num->get() - acc->get());
    systemEngine->putData(result);
    delete num;
    delete acc;
}

iPhoneMultiplyDoubleDouble::iPhoneMultiplyDoubleDouble():Instruction() {
    name = "Multiply"; // No tr
    precedence = 10;
    displayString = "x";
    argCount = 2;
}
void iPhoneMultiplyDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    DoubleData *result = new DoubleData();
    result->set(num->get() * acc->get());
    systemEngine->putData(result);
    delete num;
    delete acc;
}

iPhoneDivideDoubleDouble::iPhoneDivideDoubleDouble():Instruction() {
    name = "Divide"; // No tr
    precedence = 10;
    displayString = "/";
    argCount = 2;
}
void iPhoneDivideDoubleDouble::eval() {
    DoubleData *acc = (DoubleData *)systemEngine->getData();
    DoubleData *num = (DoubleData *)systemEngine->getData();
    if (acc->get() == 0) {
        systemEngine->setError(eDivZero);
    } else {
        DoubleData *result = new DoubleData();
        result->set(num->get() / acc->get());
        systemEngine->putData(result);
    }
    delete num;
    delete acc;
}
