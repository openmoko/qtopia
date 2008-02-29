/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include <qtopiaglobal.h>
#include <QObject>

class QPluginManagerPrivate;
class QPluginLoader;

class QTOPIA_EXPORT QPluginManager : public QObject
{
    Q_OBJECT
public:
    explicit QPluginManager(const QString &type, QObject *parent=0);
    ~QPluginManager();

    void clear();

    const QStringList &list() const;
    const QStringList &disabledList() const;
    QObject *instance(const QString &key);

    void setEnabled(const QString &name, bool enabled=true);
    bool isEnabled(const QString &name) const;

    static bool inSafeMode();
    static void init();

private slots:
    void instanceDestroyed();

private:
    void initType();
    QStringList languageList() const;
    QString stripSystem(const QString &libFile) const;
    void loaded( QObject *iface, QPluginLoader *lib, QString name );

private:
    QPluginManagerPrivate *d;
};

#endif // PLUGINLOADER_H
