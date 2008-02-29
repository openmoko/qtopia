/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** Licensees holding valid Qtopia Developer license may use this
** file in accordance with the Qtopia Developer License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING
** THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
** PURPOSE.
**
** email sales@trolltech.com for information about Qtopia License
** Agreements.
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

#include <qpe/qcom.h>

#ifndef QT_NO_COMPONENT
// {3CE88B66-B3FD-4580-9D04-77338A31A667}
#ifndef IID_Calc
#define IID_Calc QUuid( 0x3ce88b66, 0xb3fd, 0x4580, 0x9d, 0x04, 0x77, 0x33, 0x8a, 0x31, 0xa6, 0x67)
#endif
#endif


// BASE CLASS FOR PLUGIN INTERFACE
struct Q_EXPORT CalculatorInterface:public QUnknownInterface {
    virtual QWidget * create (QWidget *) = 0;
    virtual const char * pluginName() = 0;
};

#endif
