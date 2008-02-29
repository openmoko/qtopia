/****************************************************************************
**
** Copyright (C) 2006-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef ABSTRACTPROITEMVISITOR
#define ABSTRACTPROITEMVISITOR

#include "proitems.h"

struct AbstractProItemVisitor {
    virtual ~AbstractProItemVisitor() {}
    virtual bool visitBeginProBlock(ProBlock *block) = 0;
    virtual bool visitEndProBlock(ProBlock *block) = 0;

    virtual bool visitBeginProVariable(ProVariable *variable) = 0;
    virtual bool visitEndProVariable(ProVariable *variable) = 0;

    virtual bool visitBeginProFile(ProFile *value) = 0;
    virtual bool visitEndProFile(ProFile *value) = 0;

    virtual bool visitProValue(ProValue *value) = 0;
    virtual bool visitProFunction(ProFunction *function) = 0;
    virtual bool visitProOperator(ProOperator *function) = 0;
    virtual bool visitProCondition(ProCondition *function) = 0;

};

#endif // ABSTRACTPROITEMVISITOR

