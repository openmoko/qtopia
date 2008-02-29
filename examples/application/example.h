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

#ifndef EXAMPLE_H
#define EXAMPLE_H
#include "ui_examplebase.h"

class ExampleBase : public QWidget, public Ui_ExampleBase
{
public:
    ExampleBase( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~ExampleBase();
};

class Example : public ExampleBase
{ 
    Q_OBJECT
public:
    Example( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~Example();

private slots:
    void goodBye();
};

#endif // EXAMPLE_H
