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

#ifndef QMOUSEDRIVERFACTORY_QWS_H
#define QMOUSEDRIVERFACTORY_QWS_H

#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QString;
class QWSMouseHandler;

class Q_GUI_EXPORT QMouseDriverFactory
{
public:
    static QStringList keys();
    static QWSMouseHandler *create(const QString&, const QString &);
};

QT_END_HEADER

#endif // QMOUSEDRIVERFACTORY_QWS_H
