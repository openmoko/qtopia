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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "q3button.h"
#include "qpainter.h"

/*!
    \class Q3Button qbutton.h
    \brief The Q3Button class is a compatibility base class of button
    widgets

    \compat

    \bold{In new code, use QAbstractButton.}

    To subclass Q3Button, you must reimplement at least drawButton()
    (to draw the button's outline) and drawButtonLabel() (to draw its
    text or pixmap). It is generally advisable to reimplement
    sizeHint() as well, and sometimes hitButton() (to determine
    whether a button press is within the button).
*/

/*!
    Constructs a standard button called \a name with parent \a parent,
    using the widget flags \a f.
*/

Q3Button::Q3Button( QWidget *parent, const char *name, Qt::WindowFlags f )
    : QAbstractButton( parent, name, f )
{
}

/*!
    Destroys the button.
 */
Q3Button::~Q3Button()
{
}

/*!
    \fn void Q3Button::paintEvent( QPaintEvent *event)

    Handles paint events, received in \a event, for buttons. Small and
    typically complex buttons are painted double-buffered to reduce
    flicker. The actually drawing is done in the virtual functions
    drawButton() and drawButtonLabel().

    \sa drawButton(), drawButtonLabel()
*/
void Q3Button::paintEvent( QPaintEvent *)
{
    QPainter p(this);
    drawButton( &p );
}

/*!
    \fn void Q3Button::drawButton( QPainter *painter)

    Draws the button on the given \a painter. The default
    implementation does nothing.

    This virtual function is reimplemented by subclasses to draw real
    buttons. At some point, these reimplementations should call
    drawButtonLabel().

    \sa drawButtonLabel(), paintEvent()
*/
void Q3Button::drawButton( QPainter * )
{
}

/*!
    \fn void Q3Button::drawButtonLabel( QPainter *painter )

    Draws the button text or pixmap on the given \a painter.

    This virtual function is reimplemented by subclasses to draw real
    buttons. It is invoked by drawButton().

    \sa drawButton(), paintEvent()
*/

void Q3Button::drawButtonLabel( QPainter * )
{
}
