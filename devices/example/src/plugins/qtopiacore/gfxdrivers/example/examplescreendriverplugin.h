/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef EXAMPLESCREENDRIVERPLUGIN_H
#define EXAMPLESCREENDRIVERPLUGIN_H

#include <QScreenDriverPlugin>

class ExampleScreenDriverPlugin : public QScreenDriverPlugin {
    Q_OBJECT
public:
    ExampleScreenDriverPlugin( QObject *parent  = 0 );
    ~ExampleScreenDriverPlugin();

    QScreen* create(const QString& key, int displayId);
    QStringList keys() const;
};

#endif // EXAMPLESCREENDRIVERPLUGIN_H

