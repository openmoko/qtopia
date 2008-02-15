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
#include "integerinstruction.h"

// Automatic type casting
Data *BaseIntegerInstruction::eval(Data *i) {
        integerNum = (IntegerData *)num;
        Data *ret = doEval((IntegerData *)i);
        return ret;
}

// Mathematical functions
Data * IntegerAdd::doEval(IntegerData *i) {
        i->set( integerNum->get() + i->get() );
        return i;
}
Data * IntegerSub::doEval(IntegerData *i) {
        i->set( i->get() - integerNum->get() );
        return i;
}
Data * IntegerMul::doEval(IntegerData *i) {
        i->set( i->get() * integerNum->get() );
        return i;
}
Data * IntegerDiv::doEval(IntegerData *i) {
        if (!integerNum->get()) {
                systemEngine->setError(eDivZero);
        } else
                integerNum->set( i->get() / integerNum->get() );
        return i;
}

