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

#include "qtoolbarextension_p.h"
#include <qpixmap.h>
#include <qstyle.h>

#ifndef QT_NO_TOOLBUTTON

QToolBarExtension::QToolBarExtension(QWidget *parent)
    : QToolButton(parent)
{
    setObjectName(QLatin1String("qt_toolbar_ext_button"));
    setAutoRaise(true);
#ifndef QT_NO_MENU
    setPopupMode(QToolButton::InstantPopup);
#endif
    setOrientation(Qt::Horizontal);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

void QToolBarExtension::setOrientation(Qt::Orientation o)
{
    if (o == Qt::Horizontal) {
        setIcon(style()->standardPixmap(QStyle::SP_ToolBarHorizontalExtensionButton));
    } else {
        setIcon(style()->standardPixmap(QStyle::SP_ToolBarVerticalExtensionButton));
   }
}

QSize QToolBarExtension::sizeHint() const
{
    int ext = style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent);
    return QSize(ext, ext);
}

#endif // QT_NO_TOOLBUTTON
