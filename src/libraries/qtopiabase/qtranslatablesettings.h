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

#ifndef CONFIG_H
#define CONFIG_H

#include <qtopiaglobal.h>
#include <qsettings.h>

class QTranslatableSettingsPrivate;

class QTOPIA_EXPORT QTranslatableSettings : public QSettings
{
    Q_OBJECT
    QTranslatableSettingsPrivate *d;

public:
    explicit QTranslatableSettings(const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QTranslatableSettings(Scope scope, const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QTranslatableSettings(Format format, Scope scope, const QString &organization,
              const QString &application = QString(), QObject *parent = 0);
    QTranslatableSettings(const QString &fileName, Format format, QObject *parent = 0);
    explicit QTranslatableSettings(QObject *parent = 0);

    ~QTranslatableSettings();

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const;

    QStringList allKeys() const;
    QStringList childKeys() const;

    bool contains(const QString &key) const;

private:
    void initTranslation();
    static void stripTranslations(QStringList&);
};

#endif
