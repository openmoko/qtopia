/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3hbox.h"
#include "qlayout.h"
#include "qapplication.h"


/*!
    \class Q3HBox qhbox.h
    \brief The Q3HBox widget provides horizontal geometry management
    for its child widgets.

    \compat

    All the horizontal box's child widgets will be placed alongside
    each other and sized according to their sizeHint()s.

    Use setMargin() to add space around the edges, and use
    setSpacing() to add space between the widgets. Use
    setStretchFactor() if you want the widgets to be different sizes
    in proportion to one another. (See \link layout.html
    Layouts\endlink for more information on stretch factors.)

    \img qhbox-m.png Q3HBox

    \sa QHBoxLayout Q3VBox Q3Grid
*/


/*!
    Constructs an hbox widget with parent \a parent, called \a name.
    The parent, name and widget flags, \a f, are passed to the Q3Frame
    constructor.
*/
Q3HBox::Q3HBox(QWidget *parent, const char *name, Qt::WindowFlags f)
    :Q3Frame(parent, name, f)
{
    (new QHBoxLayout(this, frameWidth(), frameWidth(), name))->setAutoAdd(true);
}


/*!
    Constructs a horizontal hbox if \a horizontal is TRUE, otherwise
    constructs a vertical hbox (also known as a vbox).

    This constructor is provided for the QVBox class. You should never
    need to use it directly.

    The \a parent, \a name and widget flags, \a f, are passed to the
    Q3Frame constructor.
*/

Q3HBox::Q3HBox(bool horizontal, QWidget *parent , const char *name, Qt::WindowFlags f)
    :Q3Frame(parent, name, f)
{
    (new QBoxLayout(this, horizontal ? QBoxLayout::LeftToRight : QBoxLayout::Down,
                     frameWidth(), frameWidth(), name))->setAutoAdd(true);
}

/*!\reimp
 */
void Q3HBox::frameChanged()
{
    if (layout())
        layout()->setMargin(frameWidth());
}


/*!
    Sets the spacing between the child widgets to \a space.
*/

void Q3HBox::setSpacing(int space)
{
    if (layout())
        layout()->setSpacing(space);
}


/*!
  \reimp
*/

QSize Q3HBox::sizeHint() const
{
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents(mThis, QEvent::ChildInserted);
    return Q3Frame::sizeHint();
}

/*!
    Sets the stretch factor of widget \a w to \a stretch. Returns true if
    \a w is found. Otherwise returns false.

    \sa QBoxLayout::setStretchFactor() \link layout.html Layouts\endlink
*/
bool Q3HBox::setStretchFactor(QWidget* w, int stretch)
{
    QApplication::sendPostedEvents(this, QEvent::ChildInserted);
    if (QBoxLayout *lay = qobject_cast<QBoxLayout *>(layout()))
        return lay->setStretchFactor(w, stretch);
    return false;
}
