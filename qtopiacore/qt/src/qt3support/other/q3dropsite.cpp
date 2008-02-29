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

#include "q3dropsite.h"

#ifndef QT_NO_DRAGANDDROP

#include "qwidget.h"


/*!
  \class Q3DropSite qdropsite.h
  \brief The Q3DropSite class provides nothing and does nothing.

  \compat

  It was used in Qt 1.x to do some drag and drop; that has since been
  folded into QWidget.

  \sa Q3DragObject, Q3TextDrag, Q3ImageDrag
*/

/*!
  Constructs a Q3DropSite to handle events for the widget \a self.

  Pass \c this as the \a self parameter.
  This enables dropping by calling QWidget::setAcceptDrops(true).
*/
Q3DropSite::Q3DropSite(QWidget* self)
{
    self->setAcceptDrops(true);
}

/*!
  Destroys the drop site.
*/
Q3DropSite::~Q3DropSite()
{
}

#endif // QT_NO_DRAGANDDROP
