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
#ifndef DESKTOPSETTINGS_H
#define DESKTOPSETTINGS_H

#include <qdglobal.h>

#include <qsettings.h>
#include <qstring.h>

class QD_EXPORT DesktopSettings : public QSettings
{
public:
    DesktopSettings( const QString &section = QString() );
    virtual ~DesktopSettings();

    static void setDebugMode( bool debugMode );
    static bool debugMode();

    static void setPromptOnQuit( bool promptOnQuit );
    static bool promptOnQuit();

    static void setCurrentPlugin( const QString &plugin );
    static QString currentPlugin();

    static void setDefaultPlugin( const QString &plugin );
    static QString defaultPlugin();

    static void setInstalledDir( const QString &dir );
    static QString installedDir();

    static QString homePath();
    static QString systemLanguage();
    static QStringList languages();

    static void loadTranslations( const QString &file, QObject *parent );

    static QString deviceId();

private:
    static QStringList mLanguages;
    static QString mInstalledDir;
    static QString mCurrentPlugin;
};

#endif
