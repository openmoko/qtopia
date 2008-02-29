/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef APPEARANCESETTINGS_H
#define APPEARANCESETTINGS_H


#include "fontmap.h"
#ifdef QTOPIA_PHONE
#include "appearancesettingsbasephone.h"
#else
#include "appearancesettingsbase.h"
#endif
#include <qstrlist.h> 
#include <qasciidict.h>

struct WindowDecorationInterface;
struct StyleInterface;
class PluginLoader;
class SampleWindow;
class QHBoxLayout;

class AppearanceSettings : public AppearanceSettingsBase
{ 
    Q_OBJECT

public:
    AppearanceSettings( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~AppearanceSettings();

protected:
    void accept();
    void done(int r);
    void resizeEvent( QResizeEvent * );
    bool eventFilter(QObject *o, QEvent *e);
    QPalette readColorScheme(int id);

protected slots:
    void colorSelected( int );
    void styleSelected( int );
    void themeSelected( int );
    void decorationSelected( int );
    void fontSelected( const QString &name );
    void fontSizeSelected( const QString &sz );
    void fixSampleGeometry();
    void accelerator(int);
    void selectImage();
    void selectWallpaper();
    void updateBackground();
    void clearBackground();

private:
    void populateStyleList();
#ifdef QTOPIA_PHONE
    void selectBackground(bool src_wallpaper);
    void populateThemeList();
#endif
    void populateColorList();
    void populateDecorationList();
    void populateFontList(const QString&, int);
    void setStyle( QWidget *w, QStyle *s );
    bool isWide();

signals:
    void applyTheme(QString themeName);

private:
    WindowDecorationInterface *wdiface;
    PluginLoader *wdLoader;
    bool wdIsPlugin;
    StyleInterface *styleiface;
    PluginLoader *styleLoader;
    SampleWindow *sample;
    int prefFontSize;
    int maxFontSize;
    QStringList colorListIDs;
    QHBoxLayout *AppearanceSettingsBaseLayoutH;
#ifdef QTOPIA_PHONE
    FontMap fontMap;
    QString bgImgName;
    QTimer *bgTimer;
#endif
};


#endif // APPEARANCESETTINGS_H

