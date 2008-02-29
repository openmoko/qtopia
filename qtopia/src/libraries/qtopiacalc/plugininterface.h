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

#ifndef CALCINTERFACE_H
#define CALCINTERFACE_H

#ifndef QT_H
#include <qwidget.h>
#endif

#include <qtopia/qcom.h>
#include <qlist.h>
#include <qtopia/calc/instruction.h>

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
