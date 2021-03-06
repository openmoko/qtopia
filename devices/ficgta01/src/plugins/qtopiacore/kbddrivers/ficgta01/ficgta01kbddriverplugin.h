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

#ifndef FICGTA01KBDDRIVERPLUGIN_H
#define FICGTA01KBDDRIVERPLUGIN_H

#include <qglobal.h>

#ifdef Q_WS_QWS

#include <QtGui/QWSKeyboardHandlerFactoryInterface>

class Ficgta01KbdDriverPlugin : public QKbdDriverPlugin {
    Q_OBJECT
public:
    Ficgta01KbdDriverPlugin( QObject *parent  = 0 );
    ~Ficgta01KbdDriverPlugin();

    QWSKeyboardHandler* create(const QString& driver, const QString& device);
    QWSKeyboardHandler* create(const QString& driver);
    QStringList keys()const;
};

#endif

#endif // FICGTA01KBDDRIVERPLUGIN_H
