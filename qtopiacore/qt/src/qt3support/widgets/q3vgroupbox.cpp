/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3vgroupbox.h"

/*!
    \class Q3VGroupBox

    \brief The Q3VGroupBox widget organizes widgets in a group with one
    vertical column.

    \compat

    Q3VGroupBox is a convenience class that offers a thin layer on top
    of Q3GroupBox. Think of it as a Q3VBox that offers a frame with a
    title.

    \sa Q3HGroupBox
*/

/*!
    Constructs a horizontal group box with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/
Q3VGroupBox::Q3VGroupBox( QWidget *parent, const char *name )
    : Q3GroupBox( 1, Qt::Horizontal /* sic! */, parent, name )
{
}

/*!
    Constructs a horizontal group box with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

Q3VGroupBox::Q3VGroupBox( const QString &title, QWidget *parent,
			    const char *name )
    : Q3GroupBox( 1, Qt::Horizontal /* sic! */, title, parent, name )
{
}

/*!
    Destroys the horizontal group box, deleting its child widgets.
*/
Q3VGroupBox::~Q3VGroupBox()
{
}
