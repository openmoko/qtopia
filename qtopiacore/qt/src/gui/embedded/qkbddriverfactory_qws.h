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

#ifndef QKBDDRIVERFACTORY_QWS_H
#define QKBDDRIVERFACTORY_QWS_H

#include <QtCore/qstringlist.h>

#ifndef QT_NO_QWS_KEYBOARD

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QString;
class QWSKeyboardHandler;

class Q_GUI_EXPORT QKbdDriverFactory
{
public:
    static QStringList keys();
    static QWSKeyboardHandler *create(const QString&, const QString&);
};

QT_END_HEADER

#endif // QT_NO_QWS_KEYBOARD
#endif // QKBDDRIVERFACTORY_QWS_H
