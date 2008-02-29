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

#include "phoneheader.h"
#include "inputmethods.h"
#include <QTimer>
#include "windowmanagement.h"
#include <qscreen_qws.h>

/*!
  \class PhoneHeader
  \ingroup QtopiaServer::PhoneUI
  \brief The PhoneHeader class provides a dockable, themeable phone header.

  This class is part of the Qtopia server.
 */

/*!
  Create a new phone header with the specified \a parent.
 */
PhoneHeader::PhoneHeader(QWidget *parent)
    : PhoneThemedView(parent, Qt::FramelessWindowHint | Qt::Tool |
                              Qt::WindowStaysOnTopHint)
{
    inputMethods = new InputMethods(this, InputMethods::Any);
}

/*! \internal */
void PhoneHeader::themeLoaded(const QString &)
{
    QList<ThemeItem*> items = findItems("inputmethod", ThemedView::Widget);
    QList<ThemeItem*>::ConstIterator it;
    for (it = items.begin(); it != items.end(); ++it) {
        ThemeWidgetItem *item = (ThemeWidgetItem *)(*it);
        item->setAutoDelete(false);
        item->setWidget( inputMethods );
    }
}

/*! \internal */
QSize PhoneHeader::reservedSize() const
{
    int rh = -1;
    ThemeItem *reserved = ((ThemedView *)this)->findItem("reserved", Item);
    if (reserved)
        rh = reserved->rect().height();

    return QSize(sizeHint().width(), rh);
}

