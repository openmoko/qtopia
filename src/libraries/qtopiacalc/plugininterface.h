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

#ifndef CALCINTERFACE_H
#define CALCINTERFACE_H

#ifndef QT_H
#include <qwidget.h>
#endif

#include <qtopia/qcom.h>
#include <qlist.h>
#include "instruction.h"

#ifndef QT_NO_COMPONENT
// {3CE88B66-B3FD-4580-9D04-77338A31A667}
#ifndef IID_Calc
#define IID_Calc QUuid( 0x3ce88b66, 0xb3fd, 0x4580, 0x9d, 0x04, 0x77, 0x33, 0x8a, 0x31, 0xa6, 0x67)
#endif
#endif

/*
struct Plugin {
    QWidget *plugin;
    QString pluginName;
    QString libraryInputWidgetToUse;
};
*/

// BASE CLASS FOR PLUGIN INTERFACE
struct QTOPIA_EXPORT CalculatorInterface:public QUnknownInterface {
    virtual QWidget * create (QWidget *) = 0;
    virtual QString pluginName() const = 0;

    // not used yet
//    virtual Instruction * createInstruction(QString /* name */, QString /* type */) { return 0; };
};

#endif
