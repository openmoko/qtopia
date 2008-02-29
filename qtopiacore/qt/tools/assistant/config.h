/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef CONFIG_H
#define CONFIG_H

#include "profile.h"

#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QMap>

class Profile;

class Config
{
public:

    Config();

    void load();
    void save();
    Profile *profile() const { return profil; }
    QString profileName() const { return profil->props[QLatin1String("name")]; }
    bool validProfileName() const;
    void hideSideBar( bool b );
    bool sideBarHidden() const;
    QStringList mimePaths();

    // From profile, read only
    QStringList docFiles() const;
    QStringList docTitles() const;
    QString indexPage( const QString &title ) const;
    QString docImageDir( const QString &title ) const;
    QPixmap docIcon( const QString &title ) const;

    QStringList profiles() const;
    QString title() const;
    QString aboutApplicationMenuText() const;
    QString aboutURL() const;
    QPixmap applicationIcon() const;

    // From QSettings, read / write
    QString homePage() const;
    void setHomePage( const QString &hom ) { home = hom; }

    QStringList source() const;
    void setSource( const QStringList &s ) { src = s; }

    int sideBarPage() const { return sideBar; }
    void setSideBarPage( int sbp ) { sideBar = sbp; }

    QByteArray windowGeometry() const { return winGeometry; }
    void setWindowGeometry( const QByteArray &geometry ) { winGeometry = geometry; }
    
    QByteArray mainWindowState() const { return mainWinState; }
    void setMainWindowState( const QByteArray &state ) { mainWinState = state; }

    qreal fontPointSize() const { return pointFntSize; }
    void setFontPointSize(qreal size) { pointFntSize = size; }

    QString assistantDocPath() const;

    bool docRebuild() const { return rebuildDocs; }
    void setDocRebuild( bool rb ) { rebuildDocs = rb; }

    void saveProfile( Profile *profile );
    void loadDefaultProfile();
    bool defaultProfileExists();

    static Config *configuration();
    static Config *loadConfig(const QString &profileFileName);

private:
    Config( const Config &c );
    Config& operator=( const Config &c );

    void saveSettings();
    
private:
    Profile *profil;

    QStringList profileNames;
    QString home;
    QStringList src;
    QByteArray mainWinState;
    QByteArray winGeometry;
    qreal pointFntSize;
    int sideBar;
    bool hideSidebar;
    bool rebuildDocs;
};

#endif // CONFIG_H
