/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
    LauncherSettings( QWidget *parent=0, const char *name=0 );

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

