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

#ifndef GREENPHONEKBDDRIVERPLUGIN_H
#define GREENPHONEKBDDRIVERPLUGIN_H

#include <QtGui/QWSKeyboardHandlerFactoryInterface>

class GreenphoneKbdDriverPlugin : public QKbdDriverPlugin {
    Q_OBJECT
public:
    GreenphoneKbdDriverPlugin( QObject *parent  = 0 );
    ~GreenphoneKbdDriverPlugin();

    QWSKeyboardHandler* create(const QString& driver, const QString& device);
    QWSKeyboardHandler* create(const QString& driver);
    QStringList keys()const;
};

#endif // GREENPHONEKBDDRIVERPLUGIN_H
