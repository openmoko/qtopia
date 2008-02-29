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

#ifndef C3200MOUSEDRIVERPLUGIN_H
#define C3200MOUSEDRIVERPLUGIN_H

#include <QtGui/QWSMouseHandlerFactoryInterface>

class C3200MouseDriverPlugin : public QMouseDriverPlugin {
    Q_OBJECT
public:
    C3200MouseDriverPlugin( QObject *parent  = 0 );
    ~C3200MouseDriverPlugin();

    QWSMouseHandler* create(const QString& driver);
    QWSMouseHandler* create(const QString& driver, const QString& device);
    QStringList keys()const;
};

#endif // C3200MOUSEDRIVERPLUGIN_H
