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

#ifndef ACCESSORYP_H
#define ACCESSORYP_H

class QStartMenuAccessory;

class IStartMenuAccessory {
public:
    virtual ~IStartMenuAccessory();
    static IStartMenuAccessory * create();

    virtual QAction * add(QStartMenuAccessory *, const QIcon&, const QString&) = 0;
    virtual QAction * add(const QIcon&, const QString&, QMenu *) = 0;
    virtual void rem(QAction *) = 0;
};

#define IStartMenuAccessory_iid "com.trolltech.Qtopia.Accessory.IStartMenuAccessory"
Q_DECLARE_INTERFACE(IStartMenuAccessory, IStartMenuAccessory_iid)

#endif // ACCESSORYP_H

