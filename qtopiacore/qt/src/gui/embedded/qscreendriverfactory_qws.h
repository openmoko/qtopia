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

#ifndef QSCREENDRIVERFACTORY_QWS_H
#define QSCREENDRIVERFACTORY_QWS_H

#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QString;
class QScreen;

class Q_GUI_EXPORT QScreenDriverFactory
{
public:
    static QStringList keys();
    static QScreen *create(const QString&, int);
};

QT_END_HEADER

#endif // QSCREENDRIVERFACTORY_QWS_H
