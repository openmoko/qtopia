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

#ifndef SIMPLEIMPL_H
#define SIMPLEIMPL_H

#include "simple.h"
#include <qtopia/calc/plugininterface.h>

class QTOPIA_PLUGIN_EXPORT SimpleInterface : public CalculatorInterface
{
public:
    SimpleInterface():CalculatorInterface(){};
    virtual ~SimpleInterface(){};

#ifndef QT_NO_COMPONENT
    QRESULT queryInterface( const QUuid&, QUnknownInterface** );
    Q_REFCOUNT
#endif

    QWidget * create(QWidget *);
    const char * pluginName() { return "Simple"; };

private:
    ulong ref;
};

#endif
