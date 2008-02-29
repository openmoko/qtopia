/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef S3C24A0MOUSEDRIVERPLUGIN_H
#define S3C24A0MOUSEDRIVERPLUGIN_H

#include <QtGui/QWSMouseHandlerFactoryInterface>

class S3c24a0MouseDriverPlugin : public QMouseDriverPlugin {
    Q_OBJECT
public:
    S3c24a0MouseDriverPlugin( QObject *parent  = 0 );
    ~S3c24a0MouseDriverPlugin();

    QWSMouseHandler* create(const QString& driver);
    QWSMouseHandler* create(const QString& driver, const QString& device);
    QStringList keys()const;
};

#endif // S3C24A0MOUSEDRIVERPLUGIN_H
