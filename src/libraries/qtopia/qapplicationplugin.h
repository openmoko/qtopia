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
#ifndef APPLICATIONPLUGIN_H
#define APPLICATIONPLUGIN_H

#include <qfactoryinterface.h>
#include <qtopiaglobal.h>

struct QTOPIA_EXPORT QApplicationFactoryInterface : public QFactoryInterface
{
    virtual void setProcessKey( const QString &appName ) = 0;
    virtual QWidget *createMainWindow(const QString &key, QWidget *parent=0, Qt::WFlags f=0) = 0;
};

#define QApplicationFactoryInterface_iid "com.trolltech.Qtopia.QApplicationFactoryInterface"
Q_DECLARE_INTERFACE(QApplicationFactoryInterface, QApplicationFactoryInterface_iid)

class QTOPIA_EXPORT QApplicationPlugin : public QObject, public QApplicationFactoryInterface
{
    Q_OBJECT
    Q_INTERFACES(QApplicationFactoryInterface:QFactoryInterface)
public:
    explicit QApplicationPlugin(QObject *parent = 0);
    ~QApplicationPlugin();

    virtual QStringList keys() const = 0;
    virtual void setProcessKey( const QString &appName ) = 0;
    virtual QWidget *createMainWindow(const QString &key, QWidget *parent=0, Qt::WFlags f=0) = 0;
};

#endif
