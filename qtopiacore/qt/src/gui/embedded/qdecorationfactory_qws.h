/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QDECORATIONFACTORY_QWS_H
#define QDECORATIONFACTORY_QWS_H

#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QDecoration;

class Q_GUI_EXPORT QDecorationFactory
{
public:
    static QStringList keys();
    static QDecoration *create(const QString&);
};

QT_END_HEADER

#endif // QDECORATIONFACTORY_QWS_H
