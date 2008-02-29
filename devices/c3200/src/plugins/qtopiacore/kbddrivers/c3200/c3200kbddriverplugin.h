/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef C3200KBDDRIVERPLUGIN_H
#define C3200KBDDRIVERPLUGIN_H

#include <QtGui/QWSKeyboardHandlerFactoryInterface>

class C3200KbdDriverPlugin : public QKbdDriverPlugin {
    Q_OBJECT
public:
    C3200KbdDriverPlugin( QObject *parent  = 0 );
    ~C3200KbdDriverPlugin();

    QWSKeyboardHandler* create(const QString& driver, const QString& device);
    QWSKeyboardHandler* create(const QString& driver);
    QStringList keys()const;
};

#endif // C3200KBDDRIVERPLUGIN_H
