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
#include "launchersettingsbase.h"
#include <qmap.h>

class QCheckListItem;

class LauncherSettings : public LauncherSettingsBase
{
    Q_OBJECT
public:
    LauncherSettings( QWidget *parent=0, const char *name=0,WFlags fl=0 );

    void accept();

protected slots:
    void categorySelected( int );
    void setBackgroundType( int );
    void setViewMode( int );
    void fontFamilySelected( int );
    void fontSizeSelected(const QString&);
    void selectWallpaper();
    void selectColor( const QColor & );
    void textColor( const QColor & );

protected:
    void done( int );

private:
    void initialize();
    void showTabSettings( const QString &id );
    void readTabSettings();
    void applyTabSettings();
    void populateFontSizes( const QString &name );
    void setBackgroundControls(void);

private:
    enum ViewMode { Icon, List };
    enum BackgroundType { Ruled, SolidColor, Image };
    struct TabSettings {
	ViewMode view;
	BackgroundType bgType;
	QString bgImage;
	QString bgColor;
	QString textColor;
	QString fontFamily;
	int fontSize;
	bool customFont;
	bool changed;
    };

    int maxFontSize;
    QString currentTab;
    QStringList ids;
    QMap<QString,TabSettings> tabSettings;
};

