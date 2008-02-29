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
		qDebug("div by 0");
	} else
		integerNum->set( i->get() / integerNum->get() );
	return i;
}

